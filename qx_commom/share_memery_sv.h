#pragma once

#include <sys/mman.h>
#include <string>
#include <functional>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdexcept>
#include <cstring>
#include <fstream>



#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>



#include <time.h>
#include <iostream>

#include "common.h"

#include "data_type_def.h"




// #pragma pack(1)
// struct NanoShfeMd{
// 	char		    inst_id[32];		// 合约代码
//     uint32_t        inst_no;
//     uint32_t        update_time;                //更新时间秒
//     uint16_t        update_milli_sec;           //更新时间毫秒

//     uint16_t        type;                       //合约数据类型(0:上期 256:能源)
//     uint32_t        change_no;                  //合约行情编号

//     double          open;
//     double          high;
//     double          low;
//     double          last_price;  
//     double          turn_over;                  //总成交额
//     uint32_t        volume;      
//     uint32_t        open_interest;              //持仓量
//     double          ap[5];
//     double          bp[5];
//     uint32_t        av[5];
//     uint32_t        bv[5];

//     uint32_t        bid_volume;                 //买报单总量（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
//     int64_t         bid_amount;                 //买报单总额，放大100倍（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
//     uint32_t        ask_volume;                 //卖报单总量（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
//     int64_t         ask_amount;                 //卖报单总额，放大100倍（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
//     uint32_t        max_inst_no;                //最大合约号(同对应主题配置最大合约号值一致)
//     bool            last_tick_flag;             //last tick标识位
//     uint64_t        local_time;
//     std::string ToString();
//     void Clear();

// };
// #pragma pack()








struct ShmConfig_t {
    std::string shm_name;          //默认是/dev/shm/shm_name
    size_t      size;              //单位是MB
};

// struct SharedMemoryHeader_t {
//     size_t total_size;    //空间大小
//     size_t used_size;     //实际使用的大小
//     size_t item_count;    //数据记录条数
//     uint32_t  shm_stat;   //标记数据是否写完。  1 表示写完 0表示非完整数据
//     uint32_t  r_lock;     //暂时用不到，多路写竞态锁
//     char   res[64];       //保留
// };

class SharedMemoryManager {
public:
    enum Mode {
        CREATE_NEW = 0,     // 创建新的共享内存段
        OVERWRITE = 1,      // 覆写现有共享内存段并改变大小
        ATTACH_EXISTING = 2 // 挂接现有共享内存段
    };

    // mode 0:创建一个新的  1,附加到现有内存上,覆盖所有; 2,附加到现有内存上,保持现有数据
    SharedMemoryManager(uint32_t key, size_t sz, int shmid, int mode){
        m_shm_key = key;
        m_shm_size = sz;
        m_shm_id = shmid;

        if(mode == CREATE_NEW){
            m_shm_id = shmget(key, sz, IPC_CREAT | IPC_EXCL | 0666);
            if(m_shm_id == -1){
                throw std::runtime_error("Failed to create shared memory");
            }

            m_header = static_cast<SharedMemoryHeader_t*>(shmat(m_shm_id, nullptr, 0));
            if (m_header == nullptr) {
                throw std::runtime_error("Failed to attach shared memory segment");
            }

            m_header->total_size = m_shm_size;
            m_header->used_size = sizeof(SharedMemoryHeader_t);
            m_header->item_count = 0;
            m_header->shm_stat = 0;
            m_header->r_lock = 0;
            memset(m_header->res, 0, sizeof(m_header->res));
        }
        else if (mode == OVERWRITE){
            if (shmid == -1) {
                throw std::runtime_error("Invalid shmid for OVERWRITE mode");
            }

            if (shmctl(shmid, IPC_RMID, nullptr) == -1) {
                throw std::runtime_error("Failed to remove existing shared memory segment");
            }

            m_shm_id = shmget(key, sz, IPC_CREAT | 0666);
            if (m_shm_id == -1) {
                throw std::runtime_error("Failed to create new shared memory segment after removal");
            }

            m_header = static_cast<SharedMemoryHeader_t*>(shmat(m_shm_id, nullptr, 0));
            if (m_header == nullptr) {
                throw std::runtime_error("Failed to attach shared memory segment");
            }

            m_header->total_size = m_shm_size;
            m_header->used_size = sizeof(SharedMemoryHeader_t);
            m_header->item_count = 0;
            m_header->shm_stat = 0;
            m_header->r_lock = 0;
            memset(m_header->res, 0, sizeof(m_header->res));
        }
        else if(mode == ATTACH_EXISTING){
            if (m_shm_id == -1) {
                throw std::runtime_error("Invalid shmid for OVERWRITE mode");
            }

            m_header = static_cast<SharedMemoryHeader_t*>(shmat(m_shm_id, nullptr, 0));
            if (m_header == nullptr) {
                throw std::runtime_error("Failed to attach shared memory segment");
            }
        }
        m_shm_ptr = (char *)(m_header);

    }
    ~SharedMemoryManager(){
        if (m_header != MAP_FAILED) {
            shmdt(m_header);
        }
    }

    bool appendData(char type, const void* data, size_t data_size){
        size_t required_size = m_header->used_size + sizeof(char) + data_size;
        if (required_size > m_header->total_size) {
            return false;
        }

        char* data_ptr = m_shm_ptr + m_header->used_size;
        *data_ptr = type;
        memcpy(data_ptr + sizeof(char), data, data_size);

        m_header->used_size = required_size;
        m_header->item_count++;
        return true;
    }

    bool dumpToFile(const std::string& filename) const {
        std::ofstream out(filename, std::ios::binary);
        if (!out.is_open()) {
            return false;
        }
        out.write(m_shm_ptr, m_header->used_size);
        out.close();
        return out.good();
    }

    bool loadFromFile(const std::string& filename) {
        std::ifstream in(filename, std::ios::binary | std::ios::ate);
        if (!in.is_open()) {
            return false;
        }
        
        size_t file_size = in.tellg();
        in.seekg(0);
        if(file_size > m_header->total_size){
            return false;
        }
        in.read(m_shm_ptr, file_size);
        return in.good();
    }

    void setComplete(bool complete){
        m_header->shm_stat == 1;
    }

    char* getData() {
        return m_shm_ptr + sizeof(SharedMemoryHeader_t);
    }

    size_t getCount() const{
        return m_header->item_count;
    }

    uint32_t getComplete(bool complete){
        return m_header->shm_stat;
    }

private:
    char*                 m_shm_ptr;
    SharedMemoryHeader_t* m_header;

    key_t                 m_shm_key;
    int                   m_shm_id;



    size_t                m_shm_size;
    int                   m_shm_fd;
};

// 将共享内存dump到文件
void dumpSharedMemoryToFile(int shmid, const std::string& base_filename) {
    SharedMemoryHeader_t* header = static_cast<SharedMemoryHeader_t*>(shmat(shmid, nullptr, SHM_RDONLY));
    if (header == reinterpret_cast<SharedMemoryHeader_t*>(-1)) {
        throw std::runtime_error("Failed to attach shared memory segment");
    }
    
    try {
        time_t now = time(nullptr);
        tm* local_time = localtime(&now);
        char date_suffix[32];
        strftime(date_suffix, sizeof(date_suffix), "%Y%m%d", local_time);
        std::string filename = base_filename + ".bin";
        
        std::ofstream out_file(filename, std::ios::binary);
        if (!out_file) {
            throw std::runtime_error("Failed to open output file");
        }
        
        out_file.write(reinterpret_cast<const char*>(header), header->used_size);
        if (!out_file) {
            throw std::runtime_error("Failed to write to output file");
        }
        
        std::cout << "Successfully dumped shared memory to " << filename << std::endl;
    } catch (...) {
        shmdt(header);
        throw;
    }
    shmdt(header);
}

// 从文件加载数据到共享内存
void loadFileToSharedMemory(int shmid, const std::string& filename) {
    SharedMemoryHeader_t* header = static_cast<SharedMemoryHeader_t*>(shmat(shmid, nullptr, 0));
    if (header == reinterpret_cast<SharedMemoryHeader_t*>(-1)) {
        throw std::runtime_error("Failed to attach shared memory segment");
    }
    
    try {
        std::ifstream in_file(filename, std::ios::binary | std::ios::ate);
        if (!in_file) {
            throw std::runtime_error("Failed to open input file");
        }
        
        std::streamsize file_size = in_file.tellg();
        in_file.seekg(0, std::ios::beg);
        if (file_size > static_cast<std::streamsize>(header->total_size)) {
            throw std::runtime_error("File size exceeds shared memory capacity");
        }

        in_file.read(reinterpret_cast<char*>(header), file_size);
        
        if (!in_file) {
            throw std::runtime_error("Failed to read from input file");
        }
        
        std::cout << "Successfully loaded " << filename << " to shared memory" << std::endl;
    } catch (...) {
        shmdt(header);
        throw;
    }
    
    shmdt(header);
}



// #pragma pack(1)
// struct NanoShfeMd{
// 	char		    inst_id[32];		// 合约代码
//     uint32_t        inst_no;
//     uint32_t        update_time;                //更新时间秒
//     uint16_t        update_milli_sec;           //更新时间毫秒

//     uint16_t        type;                       //合约数据类型(0:上期 256:能源)
//     uint32_t        change_no;                  //合约行情编号

//     double          open;
//     double          high;
//     double          low;
//     double          last_price;  
//     double          turn_over;                  //总成交额
//     uint32_t        volume;      
//     uint32_t        open_interest;              //持仓量
//     double          ap[5];
//     double          bp[5];
//     uint32_t        av[5];
//     uint32_t        bv[5];

//     uint32_t        bid_volume;                 //买报单总量（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
//     int64_t         bid_amount;                 //买报单总额，放大100倍（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
//     uint32_t        ask_volume;                 //卖报单总量（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
//     int64_t         ask_amount;                 //卖报单总额，放大100倍（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
//     uint32_t        max_inst_no;                //最大合约号(同对应主题配置最大合约号值一致)
//     bool            last_tick_flag;             //last tick标识位
//     uint64_t        local_time;
//     std::string ToString();
//     void Clear();

// };
// #pragma pack()








void dumpSharedMemoryToFileCsv(int shmid, const std::string& base_filename) {
    SharedMemoryHeader_t* header = static_cast<SharedMemoryHeader_t*>(shmat(shmid, nullptr, SHM_RDONLY));
    if (header == reinterpret_cast<SharedMemoryHeader_t*>(-1)) {
        throw std::runtime_error("Failed to attach shared memory segment");
    }
    
    std::string SHFE_MD_TITLE="d_type,inst_id,inst_no,update_time,update_milli_sec,type,change_no,open,high,low,last_price,turn_over,volume,open_interest,ap1,ap2,ap3,ap4,ap5,bp1,bp2,bp3,bp4,bp5,av1,av2,av3,av4,av5,bv1,bv2,bv3,bv4,bv5,bid_volume,bid_amount,ask_volume,ask_amount,max_inst_no,last_tick_flag,local_time";

    std::string CFFEX_MD_TITLE="d_type,inst_id,update_time,update_milli_sec,open,high,low,close,last_price,turn_over,volume,open_interest,ap1,ap2,ap3,ap4,ap5,bp1,bp2,bp3,bp4,bp5,av1,av2,av3,av4,av5,bv1,bv2,bv3,bv4,bv5,upper,lower,settlement_price,curr_delta,local_time";

    std::string GFEX_MD_TITLE="d_type,inst_id,md_level,extime,open,high,low,last_price,turn_over,volume,last_match_qty,open_interest,open_interest_change,ap1,ap2,ap3,ap4,ap5,bp1,bp2,bp3,bp4,bp5,av1,av2,av3,av4,av5,bv1,bv2,bv3,bv4,bv5,avg_price,contract_no,L1_update_flag,buy_imply_qty1,buy_imply_qty2,buy_imply_qty3,buy_imply_qty4,buy_imply_qty5,sell_imply_qty1,sell_imply_qty2,sell_imply_qty3,sell_imply_qty4,sell_imply_qty5,local_time";


    std::clog <<__func__<<","<< __LINE__ <<","<<header->total_size<<","<<header->used_size<<","<<header->item_count<< std::endl;

    try {
        time_t now = time(nullptr);
        tm* local_time = localtime(&now);
        char date_suffix[32];
        strftime(date_suffix, sizeof(date_suffix), "%Y%m%d", local_time);
        std::string filename = base_filename + ".csv";
        
        std::ofstream out_file(filename, std::ios::binary);
        if (!out_file) {
            throw std::runtime_error("Failed to open output file");
        }

        bool title_write = false;

        char *start = (char *)(header+1);
        char type = *start;
        for (size_t i = 0; i< header->item_count; ++ i){
            type = *start;
            std::string line = std::to_string(type);

            if (type == DATA_TYPE_NANO_SHFE_MD){

                if (!title_write){
                    SHFE_MD_TITLE += "\n";
                    out_file.write(SHFE_MD_TITLE.c_str(), SHFE_MD_TITLE.size());
                    title_write=true;
                }

                NanoShfeMd *md = (NanoShfeMd *)(start+1);
                line = line + "," + std::string(md->inst_id) 
                            + "," + std::to_string(md->inst_no)  
                            + "," + std::to_string(md->update_time) 
                            + "," + std::to_string(md->update_milli_sec)
                            + "," + std::to_string(md->type) 
                            + "," + std::to_string(md->change_no) 
                            + "," + std::to_string(md->open)  
                            + "," + std::to_string(md->high) 
                            + "," + std::to_string(md->low)
                            + "," + std::to_string(md->last_price)
                            + "," + std::to_string(md->turn_over)
                            + "," + std::to_string(md->volume)
                            + "," + std::to_string(md->open_interest)
                            + "," + std::to_string(md->ap[0])
                            + "," + std::to_string(md->ap[1])
                            + "," + std::to_string(md->ap[2])
                            + "," + std::to_string(md->ap[3])
                            + "," + std::to_string(md->ap[4])
                            + "," + std::to_string(md->bp[0])
                            + "," + std::to_string(md->bp[1])
                            + "," + std::to_string(md->bp[2])
                            + "," + std::to_string(md->bp[3])
                            + "," + std::to_string(md->bp[4])
                            + "," + std::to_string(md->av[0])
                            + "," + std::to_string(md->av[1])
                            + "," + std::to_string(md->av[2])
                            + "," + std::to_string(md->av[3])
                            + "," + std::to_string(md->av[4])
                            + "," + std::to_string(md->bv[0])
                            + "," + std::to_string(md->bv[1])
                            + "," + std::to_string(md->bv[2])
                            + "," + std::to_string(md->bv[3])
                            + "," + std::to_string(md->bv[4])
                            + "," + std::to_string(md->bid_volume)
                            + "," + std::to_string(md->bid_amount)
                            + "," + std::to_string(md->ask_volume)
                            + "," + std::to_string(md->ask_amount)
                            + "," + std::to_string(md->max_inst_no)
                            + "," + std::to_string(md->last_tick_flag)
                            + "," + std::to_string(md->local_time) 
                            + "\n";

                out_file.write(line.c_str(), line.size());
                start = start + sizeof(NanoShfeMd) + 1;
            }
            else if(type == DATA_TYPE_NANO_CFFEX_MD){

                if (!title_write){
                    CFFEX_MD_TITLE += "\n";
                    out_file.write(CFFEX_MD_TITLE.c_str(), CFFEX_MD_TITLE.size());
                    title_write=true;
                }

                NanoCffexMd *md = (NanoCffexMd *)(start+1);
                line = line + "," + std::string(md->inst_id) 
                            + "," + std::to_string(md->update_time) 
                            + "," + std::to_string(md->update_milli_sec)
                            + "," + std::to_string(md->open)  
                            + "," + std::to_string(md->high) 
                            + "," + std::to_string(md->low)
                            + "," + std::to_string(md->close)
                            + "," + std::to_string(md->last_price)
                            + "," + std::to_string(md->turn_over)
                            + "," + std::to_string(md->volume)
                            + "," + std::to_string(md->open_interest)
                            + "," + std::to_string(md->ap[0])
                            + "," + std::to_string(md->ap[1])
                            + "," + std::to_string(md->ap[2])
                            + "," + std::to_string(md->ap[3])
                            + "," + std::to_string(md->ap[4])
                            + "," + std::to_string(md->bp[0])
                            + "," + std::to_string(md->bp[1])
                            + "," + std::to_string(md->bp[2])
                            + "," + std::to_string(md->bp[3])
                            + "," + std::to_string(md->bp[4])
                            + "," + std::to_string(md->av[0])
                            + "," + std::to_string(md->av[1])
                            + "," + std::to_string(md->av[2])
                            + "," + std::to_string(md->av[3])
                            + "," + std::to_string(md->av[4])
                            + "," + std::to_string(md->bv[0])
                            + "," + std::to_string(md->bv[1])
                            + "," + std::to_string(md->bv[2])
                            + "," + std::to_string(md->bv[3])
                            + "," + std::to_string(md->bv[4])
                            + "," + std::to_string(md->upper)
                            + "," + std::to_string(md->lower)
                            + "," + std::to_string(md->settlement_price)
                            + "," + std::to_string(md->curr_delta)
                            + "," + std::to_string(md->local_time) 
                            + "\n";
                out_file.write(line.c_str(), line.size());
                start = start + sizeof(NanoCffexMd) + 1;
            }
            else if(type == DATA_TYPE_NANO_GFEX_MD){

                if (!title_write){
                    GFEX_MD_TITLE += "\n";
                    out_file.write(GFEX_MD_TITLE.c_str(), GFEX_MD_TITLE.size());
                    title_write=true;
                }

                NanoGfexMd *md = (NanoGfexMd *)(start+1);
                line = line + "," + std::string(md->inst_id) 
                            + "," + std::to_string(md->md_level) 
                            + "," + std::to_string(md->extime)
                            + "," + std::to_string(md->open)  
                            + "," + std::to_string(md->high) 
                            + "," + std::to_string(md->low)
                            + "," + std::to_string(md->last_price)
                            + "," + std::to_string(md->turn_over)
                            + "," + std::to_string(md->volume)
                            + "," + std::to_string(md->last_match_qty)
                            + "," + std::to_string(md->open_interest)
                            + "," + std::to_string(md->open_interest_change)
                            + "," + std::to_string(md->ap[0])
                            + "," + std::to_string(md->ap[1])
                            + "," + std::to_string(md->ap[2])
                            + "," + std::to_string(md->ap[3])
                            + "," + std::to_string(md->ap[4])
                            + "," + std::to_string(md->bp[0])
                            + "," + std::to_string(md->bp[1])
                            + "," + std::to_string(md->bp[2])
                            + "," + std::to_string(md->bp[3])
                            + "," + std::to_string(md->bp[4])
                            + "," + std::to_string(md->av[0])
                            + "," + std::to_string(md->av[1])
                            + "," + std::to_string(md->av[2])
                            + "," + std::to_string(md->av[3])
                            + "," + std::to_string(md->av[4])
                            + "," + std::to_string(md->bv[0])
                            + "," + std::to_string(md->bv[1])
                            + "," + std::to_string(md->bv[2])
                            + "," + std::to_string(md->bv[3])
                            + "," + std::to_string(md->bv[4])
                            + "," + std::to_string(md->avg_price)
                            + "," + std::to_string(md->contract_no)
                            + "," + std::to_string(md->L1_update_flag)
                            + "," + std::to_string(md->buy_imply_qty[0])
                            + "," + std::to_string(md->buy_imply_qty[1])
                            + "," + std::to_string(md->buy_imply_qty[2])
                            + "," + std::to_string(md->buy_imply_qty[3])
                            + "," + std::to_string(md->buy_imply_qty[4])
                            + "," + std::to_string(md->sell_imply_qty[0])
                            + "," + std::to_string(md->sell_imply_qty[1])
                            + "," + std::to_string(md->sell_imply_qty[2])
                            + "," + std::to_string(md->sell_imply_qty[3])
                            + "," + std::to_string(md->sell_imply_qty[4])
                            + "," + std::to_string(md->local_time) 
                            + "\n";
                out_file.write(line.c_str(), line.size());
                start = start + sizeof(NanoGfexMd) + 1;


            }
        }
        
        std::cout << "Successfully dumped to " << filename << std::endl;
    } catch (...) {
        shmdt(header);
        throw;
    }
    shmdt(header);
}






