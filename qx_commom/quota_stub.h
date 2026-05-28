
#include <vector>
#include <string>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <unistd.h>


#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "SockProto.h"

#include "common.h"


#pragma pack(push, 1)

typedef struct
{
    unsigned int type; // MD_TYPE
    unsigned int len;  // length of data
    char         data[1024];    // data
} Quotatation;




struct Option_SnapShot
{
    char security_id[8];        // 证券代码
    unsigned long long md_time; // 交易所时间年月日时分秒毫秒

    unsigned long long pre_close;    // 前收盘价，放大10000
    unsigned long long open;         // 开盘价，放大1000000
    unsigned long long high;         // 最高价，放大1000000
    unsigned long long low;          // 最低价，放大1000000
    unsigned long long match;        // 最新价，放大1000000
    unsigned long long ask_price[5]; // 申卖价，放大1000000
    unsigned long long ask_vol[5];   // 申卖量，放大100
    unsigned long long bid_price[5]; // 申买价，放大1000000
    unsigned long long bid_vol[5];   // 申买量，放大100

    unsigned long long total_value;     // 成交总金额，放大10000
    unsigned long long total_volume;    // 成交总量，放大100
    unsigned long long upper_limit;     // 涨停价，放大1000000
    unsigned long long lower_limit;     // 跌停价，放大1000000
    unsigned long long position_volume; // 持仓数量，放大100

    int num_trades;      // 成交笔数
    char phase_code[8];  // 阶段码
    char mdstream_id[3]; // 行情类别
};


//撮合行情
struct ETF_10ms_snapshot
{
    char security_id[8];        // 证券代码
    int nTime;                  //时间(HHMMSSmmmm)
    unsigned int uPreClose;     //前收盘价,扩大10000倍
    unsigned int uOpen;         //开盘价,扩大10000倍
    unsigned int uHigh;         //最高价,扩大10000倍
    unsigned int uLow;          //最低价,扩大10000倍
    unsigned int uMatch;        //最新价,扩大10000倍
    unsigned int uAskPrice[10]; //申卖价,扩大10000倍
    unsigned int uAskVol[10];   //申卖量
    unsigned int uBidPrice[10]; //申买价,扩大10000倍
    unsigned int uBidVol[10];   //申买量
    unsigned int uNumTrades;    //成交笔数
    long long iVolume;          //成交总量
    long long iTurnover;        //成交总金额,扩大10000倍
};
#pragma pack(pop)






























class QuotaStub{

public:
    QuotaStub(){};
    

    void Load(std::string filename){
        
        std::clog <<__func__<<","<< __LINE__<< ","<<filename<< std::endl;
        std::ifstream if_inst(filename, std::ios::in);
        std::string line;
        std::getline(if_inst, line);
        while (std::getline(if_inst, line)) {

            std::vector<std::string> vec;
            SplitString(line, vec, ",");

            //std::copy(vec.begin(), vec.end(), std::ostream_iterator<std::string>(std::cout, ","));
            std::string instrument_id = vec[5].substr(4, vec[5].size()-4);
            int exch_id = std::atoi(vec[5].c_str());
            double bp1 = std::atof(vec[22].c_str());
            double ap1 = std::atof(vec[23].c_str());
            uint32_t bv1 = std::atoi(vec[24].c_str());
            uint32_t av1 = std::atoi(vec[25].c_str());
            uint64_t vol = std::atoll(vec[18].c_str());

            //std::clog <<__func__<<","<< __LINE__<<","<<vec[21]<<","<<vec[22]<<","<<vec[23]<<","<<vec[24]<<","<<vec[17]<< std::endl;
            m_quotation.emplace_back(Quotatation());
            auto &cur = m_quotation.back();
            if (instrument_id.size() == 6){
                cur.type = 0x00020008U;
                ETF_10ms_snapshot *msgdata = (ETF_10ms_snapshot *)(cur.data);
                memcpy(msgdata->security_id, instrument_id.c_str(), instrument_id.size()+1);
                msgdata->uAskPrice[0] = ap1*100000;
                msgdata->uAskVol[0]   = av1;
                msgdata->uBidPrice[0] = bp1*100000;
                msgdata->uBidVol[0]   = bv1;
                msgdata->iVolume      = vol;
            }
            else if(instrument_id.size() == 8){
                cur.type = 0x00020007U;
                Option_SnapShot *msgdata = (Option_SnapShot *)(cur.data);
                memcpy(msgdata->security_id, instrument_id.c_str(), 8);
                msgdata->ask_price[0] = ap1*1000000;
                msgdata->ask_vol[0]   = av1*100;
                msgdata->bid_price[0] = bp1*1000000;
                msgdata->bid_vol[0]   = bv1*100;
                msgdata->total_volume = vol*100;
            }
            std::clog <<__func__<<","<< __LINE__<< ",m_quotation:"<<m_quotation.size()<<",instrument_id:"<<instrument_id<<","<<ap1<<","<<bp1<<","<<av1<<","<<bv1<<","<<vol<< std::endl;
        }
        std::clog <<__func__<<","<< __LINE__<< ",m_quotation:"<<m_quotation.size()<< std::endl;
    }

    ~QuotaStub(){
        
    }

    int32_t GetNextQuota(int32_t idx, Quotatation &quota, int delay_us=0){
        if (idx < m_quotation.size()){
            quota = m_quotation[idx];
        }
        usleep(delay_us);
        return (idx+1) >= m_quotation.size() ? -1 : (idx+1);
    }

private:

    std::vector<Quotatation>  m_quotation;

};


//////////////////////////////////////////////////////////////////////////////////




struct EulerTick{
    std::string inst_code;
	uint32_t    tradingday;  //str
	uint64_t    extime;      //str
	float64_t   turnover;
	uint64_t    volume;
	float64_t   last_price;
    uint64_t    oi;
	float64_t   upper;
	float64_t   lower;
	uint64_t    local_time;  //str
	uint32_t    level;  //有效档位
	float64_t   bp1;
	float64_t   bp2;
	float64_t   bp3;
	float64_t   bp4;
	float64_t   bp5;
	float64_t   ap1;
	float64_t   ap2;
	float64_t   ap3;
	float64_t   ap4;
	float64_t   ap5;
	uint32_t   bv1;
	uint32_t   bv2;
	uint32_t   bv3;
	uint32_t   bv4;
	uint32_t   bv5;
	uint32_t   av1;
	uint32_t   av2;
	uint32_t   av3;
	uint32_t   av4;
	uint32_t   av5;

};






class EulerQuotaStub{

public:
    EulerQuotaStub(){};
    ~EulerQuotaStub(){};

    void Load(std::string filename, uint32_t level){
        std::clog <<__func__<<","<< __LINE__<< ","<<filename<< std::endl;
        std::ifstream if_inst(filename, std::ios::in);
        std::string line;
        while (std::getline(if_inst, line)) {
            std::vector<std::string> vec;
            SplitString(line, vec, ",");
            
            m_quotation.emplace_back(EulerTick());
            auto &tick = m_quotation.back();
            char* pEnd;
            tick.inst_code = vec[0];
            tick.tradingday = std::atoi(vec[1].c_str());
            tick.extime     = std::atoll(vec[2].c_str());
            tick.turnover   = std::strtod(vec[3].c_str(),&pEnd);
            tick.volume     = std::atoll(vec[4].c_str());
            tick.last_price = std::strtod(vec[5].c_str(),&pEnd);
            tick.oi         = std::strtod(vec[6].c_str(),&pEnd);
            tick.upper      = std::strtod(vec[7].c_str(),&pEnd);
            tick.lower      = std::strtod(vec[8].c_str(),&pEnd);
            tick.local_time = std::atoll(vec[9].c_str());

            tick.bp1        = std::strtod(vec[10].c_str(),&pEnd);
            tick.ap1        = std::strtod(vec[15].c_str(),&pEnd);
            tick.bv1        = std::atoi(vec[20].c_str());
            tick.av1        = std::atoi(vec[25].c_str());

            if (level == 1){
                tick.bp2        = std::strtod(vec[11].c_str(),&pEnd);
                tick.bp3        = std::strtod(vec[12].c_str(),&pEnd);
                tick.bp4        = std::strtod(vec[13].c_str(),&pEnd);
                tick.bp5        = std::strtod(vec[14].c_str(),&pEnd);

                tick.ap2        = std::strtod(vec[16].c_str(),&pEnd);
                tick.ap3        = std::strtod(vec[17].c_str(),&pEnd);
                tick.ap4        = std::strtod(vec[18].c_str(),&pEnd);
                tick.ap5        = std::strtod(vec[19].c_str(),&pEnd);

                tick.bv2        = std::atoi(vec[21].c_str());
                tick.bv3        = std::atoi(vec[22].c_str());
                tick.bv4        = std::atoi(vec[23].c_str());
                tick.bv5        = std::atoi(vec[24].c_str());

                tick.av2        = std::atoi(vec[26].c_str());
                tick.av3        = std::atoi(vec[27].c_str());
                tick.av4        = std::atoi(vec[28].c_str());
                tick.av5        = std::atoi(vec[29].c_str());
            }

        }
        std::clog <<__func__<<","<< __LINE__<< ",m_quotation:"<<m_quotation.size()<< std::endl;
    }

    void Sort(){
        std::sort(m_quotation.begin(), m_quotation.end(),[](const EulerTick& a, const EulerTick& b) {return a.extime < b.extime;});
    }

    void Clean(){
        // 处理头部数据(time < 90000000)
        auto head_end = std::find_if_not(m_quotation.begin(), m_quotation.end(), [](const EulerTick& t) { return t.extime < 90000000; });
        if (head_end != m_quotation.begin()) {
            // 保留最后一条
            auto last_head = std::prev(head_end);
            m_quotation.erase(m_quotation.begin(), last_head);
        }

        // 处理尾部数据(time > 150000000)
        auto tail_start = std::find_if(m_quotation.rbegin(), m_quotation.rend(), [](const EulerTick& t) { return t.extime > 150000000 && t.last_price > 0.0; });
        if (tail_start != m_quotation.rend()) {
            // 保留最后一条符合条件的
            auto last_valid = tail_start.base();
            m_quotation.erase(last_valid, m_quotation.end());
        }
    }

    int32_t GetNextQuota(int32_t idx, EulerTick &quota, int delay_us=0){
        if (idx < m_quotation.size()){
            quota = m_quotation[idx];
        }
        usleep(delay_us);
        return (idx+1) >= m_quotation.size() ? -1 : (idx+1);
    }

    size_t GetSize(){
        return m_quotation.size();
    }

private:
    std::vector<EulerTick>  m_quotation;
};








class EthernetQuotaStub{

public:
    EthernetQuotaStub(){

    };
    ~EthernetQuotaStub(){

        munmap(m_mapped, m_file_size);
        close(m_fd);
    };

    void Load(std::string &capfile){
        m_fd = open(capfile.c_str(), O_RDONLY);
        if (m_fd == -1) {
            perror("open failed");
            return;
        }

        struct stat sb;
        if (fstat(m_fd, &sb) == -1) {
            perror("fstat failed");
            close(m_fd);
            return;
        }
        m_file_size = sb.st_size;
        m_mapped = mmap(nullptr, m_file_size, PROT_READ, MAP_PRIVATE, m_fd, 0);
        if (m_mapped == MAP_FAILED) {
            perror("mmap failed");
            close(m_fd);
            return;
        }

        char * data = (char *)m_mapped;

        hexDumpToClog(data, 1024);

        data += 28;

        int64_t re_len = m_file_size;
        do {
            //ethhdr    18
            //ip_header 20
            //udp       8

            data += 12;

            tcpip::ethhdr *eth_ptr = (tcpip::ethhdr *)data;
            

            tcpip::ip_header *ip_ptr = (tcpip::ip_header *)(data + sizeof(tcpip::ethhdr));
            uint16_t ip_header_data_len = TransU16(ip_ptr->ip_len);

            // std::clog <<__func__<<","<< __LINE__<<",ip_header_data_len:" << ip_header_data_len << std::endl;

            tcpip::udp_header *udp_ptr = (tcpip::udp_header *)(data + sizeof(tcpip::ethhdr) + sizeof(tcpip::ip_header));
            uint16_t udp_header_data_len = TransU16(udp_ptr->dataLen);


            // std::clog <<__func__<<","<< __LINE__<<",udp_header_data_len[" <<udp_header_data_len<<"]" << std::endl;

            std::string ip_str = uint32_to_ip_safe(ip_ptr->destination_ip_address);
            uint16_t dstPort_local = ntohs(udp_ptr->dstPort);

            // std::clog <<__func__<<","<< __LINE__<<",ip_str[" <<ip_str<<"],port:" << dstPort_local << std::endl;



            char *data_ptr = (char *)(udp_ptr +1);


            size_t pk_len = sizeof(tcpip::ethhdr) + sizeof(tcpip::ip_header) + sizeof(tcpip::udp_header) + udp_header_data_len;
            
            m_quotation.emplace_back(std::make_pair((char *)eth_ptr,pk_len));
          
            
            // std::clog <<__func__<<","<< __LINE__<<",pk_len[" <<pk_len<<",eth:" <<sizeof(tcpip::ethhdr)<<",ip:" << sizeof(tcpip::ip_header)<<",udp:" << sizeof(tcpip::udp_header) << std::endl;


            //std::clog <<__func__<<","<< __LINE__<<",ip_str[" <<ip_str<<"],port:" << dstPort_local <<",data_len[" <<udp_header_data_len<<"]" << std::endl;

            data += (pk_len);

            re_len -= (pk_len + 12);

        }while(re_len>0);

        std::clog <<__func__<<","<< __LINE__<<",m_quotation.size[" <<m_quotation.size()<<"]"<< std::endl;
    }

    size_t GetSize(){
        return m_quotation.size();
    }


    char * GetNextQuota(int32_t idx, size_t &sz, int delay_us=0){

        // return nullptr;

        if (idx >= m_quotation.size()){
            return nullptr;
        }
        usleep(delay_us);
        sz = m_quotation.at(idx).second;
        return m_quotation.at(idx).first;
    }





private:
    int    m_fd;
    void*  m_mapped;
    size_t m_file_size;
    
    std::vector<std::pair<char *, size_t>>  m_quotation;


};


