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



#include <fcntl.h>      
#include <sys/stat.h> 


#include "json.hpp"
#include "common.h"

struct ShmConfig_t {
    std::string shm_name;          //默认是/dev/shm/shm_name
    size_t      size;              //单位是MB
};


struct SharedMemoryHeader_t {
    size_t total_size;    //空间大小
    size_t used_size;     //实际使用的大小
    size_t item_count;    //数据记录条数
    uint32_t  shm_stat;   //标记数据是否写完。  1 表示写完 0表示非完整数据
    uint32_t  r_lock;     //暂时用不到，多路写竞态锁
    char   res[64];       //保留
};


class SharedMemoryManager {
public:
    enum Mode {
        CREATE_NEW = 0,     // 创建新的共享内存
        OVERWRITE = 1,      // 覆写现有共享内存
        ATTACH_EXISTING = 2 // 附加到现有共享内存
    };

    SharedMemoryManager(const std::string shm_name, size_t sz, int mode){
        m_shm_name = shm_name;
        m_shm_size = sz;

        if (mode == CREATE_NEW){
            m_shm_fd = shm_open(m_shm_name.c_str(), O_CREAT | O_EXCL | O_RDWR, 0666);
            if (m_shm_fd == -1) {
                throw std::runtime_error("Failed to create shared memory object");
            }

            if (ftruncate(m_shm_fd, m_shm_size) == -1) {
                close(m_shm_fd);
                shm_unlink(m_shm_name.c_str());
                throw std::runtime_error("Failed to set shared memory size");
            }

            m_shm_ptr = (char *)mmap(nullptr, m_shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_shm_fd, 0);
            if (m_shm_ptr == MAP_FAILED) {
                close(m_shm_fd);
                shm_unlink(m_shm_name.c_str());
                throw std::runtime_error("Failed to map shared memory");
            }

            m_header = (SharedMemoryHeader_t *)m_shm_ptr;
            m_header->total_size = m_shm_size;
            m_header->used_size = sizeof(SharedMemoryHeader_t);
            m_header->item_count = 0;
            m_header->shm_stat = 0;
            m_header->r_lock = 0;

        }
        else if(mode == OVERWRITE){
            shm_unlink(m_shm_name.c_str());
            m_shm_fd = shm_open(m_shm_name.c_str(), O_CREAT | O_RDWR, 0666);
            if (m_shm_fd == -1) {
                throw std::runtime_error("Failed to create shared memory object");
            }

            if (ftruncate(m_shm_fd, m_shm_size) == -1) {
                close(m_shm_fd);
                shm_unlink(m_shm_name.c_str());
                throw std::runtime_error("Failed to set shared memory size");
            }

            m_shm_ptr = (char *)mmap(nullptr, m_shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_shm_fd, 0);
            if (m_shm_ptr == MAP_FAILED) {
                close(m_shm_fd);
                shm_unlink(m_shm_name.c_str());
                throw std::runtime_error("Failed to map shared memory");
            }

            m_header = (SharedMemoryHeader_t *)m_shm_ptr;
            m_header->total_size = m_shm_size;
            m_header->used_size = sizeof(SharedMemoryHeader_t);
            m_header->item_count = 0;
            m_header->shm_stat = 0;
            m_header->r_lock = 0;
        }
        else if(mode == ATTACH_EXISTING){
            m_shm_fd = shm_open(m_shm_name.c_str(), O_RDWR, 0666);
            if (m_shm_fd == -1) {
                throw std::runtime_error("Failed to create shared memory object");
            }

            struct stat st;
            if (fstat(m_shm_fd, &st) == -1) {
                close(m_shm_fd);
                throw std::runtime_error("Failed to get shared memory size");
            }

            m_shm_ptr = (char *)mmap(nullptr, m_shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_shm_fd, 0);
            if (m_shm_ptr == MAP_FAILED) {
                close(m_shm_fd);
                shm_unlink(m_shm_name.c_str());
                throw std::runtime_error("Failed to map shared memory");
            }
        }
    }
    ~SharedMemoryManager(){
        if (m_shm_ptr != MAP_FAILED) {
            munmap(m_shm_ptr, m_shm_size);
        }
        if (m_shm_fd != -1) {
            close(m_shm_fd);
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
    std::string           m_shm_name;
    size_t                m_shm_size;
    int                   m_shm_fd;
};





// 将共享内存dump到文件
void dumpSharedMemoryToFile(const std::string shmname, const std::string targetfilename) {
    int shm_fd = shm_open(shmname.c_str(), O_RDONLY, 0666);
    if (shm_fd == -1) {
        throw std::runtime_error("Failed to open shared memory object");
    }
    
    struct stat st;
    if (fstat(shm_fd, &st) == -1) {
        close(shm_fd);
        throw std::runtime_error("Failed to get shared memory size");
    }
    
    void* shm_ptr = mmap(nullptr, st.st_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        close(shm_fd);
        throw std::runtime_error("Failed to map shared memory");
    }
    
    try {
        time_t now = time(nullptr);
        tm* local_time = localtime(&now);
        char date_suffix[32];
        strftime(date_suffix, sizeof(date_suffix), "%Y%m%d", local_time);
        std::string filename = targetfilename + "_" + date_suffix + ".mem";
        
        std::ofstream out_file(filename, std::ios::binary);
        if (!out_file) {
            throw std::runtime_error("Failed to open output file");
        }
        
        out_file.write(reinterpret_cast<const char*>(shm_ptr), st.st_size);
        
        if (!out_file) {
            throw std::runtime_error("Failed to write to output file");
        }
        
        std::cout << "Successfully dumped shared memory to " << filename << std::endl;
    } catch (...) {
        munmap(shm_ptr, st.st_size);
        close(shm_fd);
        throw;
    }
    
    munmap(shm_ptr, st.st_size);
    close(shm_fd);
}

// 从文件加载数据到共享内存
void loadFileToSharedMemory(const char* shmname, const std::string& filename) {

    int shm_fd = shm_open(shmname, O_RDWR, 0666);
    if (shm_fd == -1) {
        throw std::runtime_error("Failed to open shared memory object");
    }
    
    struct stat st;
    if (fstat(shm_fd, &st) == -1) {
        close(shm_fd);
        throw std::runtime_error("Failed to get shared memory size");
    }

    SharedMemoryHeader_t* header = static_cast<SharedMemoryHeader_t*>(
        mmap(nullptr, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
    if (header == MAP_FAILED) {
        close(shm_fd);
        throw std::runtime_error("Failed to map shared memory");
    }
    
    try {
        std::ifstream in_file(filename, std::ios::binary | std::ios::ate);
        if (!in_file) {
            throw std::runtime_error("Failed to open input file");
        }
        
        std::streamsize file_size = in_file.tellg();
        in_file.seekg(0, std::ios::beg);
        
        if (file_size > st.st_size) {
            throw std::runtime_error("File size exceeds shared memory capacity");
        }
        
        in_file.read(reinterpret_cast<char*>(header), file_size);
        
        if (!in_file) {
            throw std::runtime_error("Failed to read from input file");
        }
        
        std::cout << "Successfully loaded " << filename << " to shared memory" << std::endl;
    } catch (...) {
        munmap(header, st.st_size);
        close(shm_fd);
        throw;
    }
    
    munmap(header, st.st_size);
    close(shm_fd);
}









