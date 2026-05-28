#pragma once

#include <algorithm>
#include <ctime>
#include <chrono>
#include <float.h>
#include <math.h>
#include <fstream>
#include <thread>
#include <unistd.h>
#include <sys/syscall.h>
#include <cstdint>
#include <iostream>
#include <set>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <sys/mman.h>
#include <x86intrin.h>

#include <unordered_map>
#include <cstring>
#include <functional>


#include <dirent.h>



#include "tradingtime.h"

#include "json.hpp"  //nlohmann

#include "macro.h"

#define LOG_LEVEL_ERROR    1
#define LOG_LEVEL_TRACE    2
#define LOG_LEVEL_DEBUG    4
#define LOG_LEVEL (LOG_LEVEL_DEBUG|LOG_LEVEL_TRACE|LOG_LEVEL_ERROR)
static uint32_t g_log_level = LOG_LEVEL;


#define LOG_DEBUG(info) do { if (g_log_level&LOG_LEVEL_DEBUG) {LOG_WRITE(NAME2STR(DEBUG),__func__,__LINE__,info); }  } while(0)
#define LOG_TRACE(info) do { if (g_log_level&LOG_LEVEL_TRACE) {LOG_WRITE(NAME2STR(TRACE),__func__,__LINE__,info); }  } while(0)
#define LOG_ERROR(info) do { if (g_log_level&LOG_LEVEL_ERROR) {LOG_WRITE(NAME2STR(ERROR),__func__,__LINE__,info); }  } while(0)


USED_API static void LOG_WRITE(std::string level,std::string func_name, uint32_t line_no, std::string info){
    std::string logfile = std::string("./")+ "my.log";
    std::ofstream logstream(logfile, std::ios::binary|std::ios::out|std::ios::app);
    std::string line = std::string("[")+ level + "]["+ func_name + "][" + std::to_string(line_no)+ "]["+ info + "]\n";
    //std::string line = std::string("[")+ func_name + "][" + info + "]\n";
    logstream.write(line.c_str(),line.size());
    logstream.close();
    std::cout<<line;
}

// febao行情中，和价格相关的值使用DML_MAX表示空值
static inline double convert_price_if_zero(double price) noexcept
{
    return (price == 0.0f) ? DBL_MAX : price;
}

// febao行情中，板价和板量的关系
static inline double convert_price_if_volume_zero(double price, uint32_t volume) noexcept
{
    if (volume > 0 && price > std::numeric_limits<double>::lowest() && price < std::numeric_limits<double>::max())
        return price;
    return DBL_MAX;
}


// inline void SplitString(const std::string& str,std::vector<std::string>& vec,const std::string& delims) noexcept
// {
//     vec.clear();
//     auto start = str.begin();
//     const auto end = str.end();

//     while(start != end) {
//         start = std::find_if_not(start, end, [&delims](char c){return delims.find(c) != std::string::npos || isspace(c);});
//         if(start == end) break;
//         auto pos = std::find_if(start, end, [&delims](char c){return delims.find(c) != std::string::npos;});

//         while(pos != start && isspace(*(pos-1))) --pos;
//         vec.emplace_back(start, pos);
//         start = pos;
//     }
// }


inline void BindCPU(int cpu_id){
    cpu_set_t mask{0};
    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);

    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) != 0)
    {
        printf("bind CPU failed[%ld][%d]\n", pthread_self(), cpu_id);
    }
    std::clog <<__func__<<","<< __LINE__<<",bind thread["<<syscall(SYS_gettid)<<"] to CPU["<<cpu_id<<"]"<< std::endl;
    return;
}


inline std::string uint32_to_ipstr(uint32_t ip) noexcept
{
    struct in_addr addr;
    addr.s_addr = ip;  
    return inet_ntoa(addr);   
}

// 线程安全
inline std::string uint32_to_ip_safe(uint32_t ip) {
    char buf[INET_ADDRSTRLEN];
    struct in_addr addr;
    addr.s_addr = ip;
    inet_ntop(AF_INET, &addr, buf, sizeof(buf));
    return std::string(buf);
}

inline uint32_t ip_str_to_net_ip(std::string &&ip_str) {
    struct in_addr addr;
    inet_pton(AF_INET, ip_str.c_str(), &addr);
    return addr.s_addr; // 网络字节序
}

inline uint32_t ip_str_to_net_ip2(std::string ip_str) {
    struct in_addr addr;
    inet_pton(AF_INET, ip_str.c_str(), &addr);
    return addr.s_addr; // 网络字节序
}


inline void hexDumpToClog(const char* buffer, size_t length) {
    // 保存原始格式状态
    std::ios oldState(nullptr);
    oldState.copyfmt(std::clog);
    
    // 设置十六进制输出格式
    std::clog << std::hex << std::setfill('0');
    
    for (size_t i = 0; i < length; ++i) {
        // 每128字节换行并显示偏移量
        if (i % 16 == 0) {
            std::clog << std::dec << std::setfill('0');
            std::clog << "\n" << std::setw(4) << i << ": ";
            //std::clog << "\n0x" << std::setw(4) << i << ": ";
        }
        std::clog << std::hex << std::setfill('0');
        // 输出当前字节的十六进制值
        std::clog << std::setw(2) << (static_cast<unsigned>(buffer[i]) & 0xFF) << " ";
        
        // 每16字节增加额外空格提高可读性
        if (i % 8 == 7 && i % 16 != 15) {
            std::clog << " ";
        }
    }
    
    // 恢复原始格式状态
    std::clog.copyfmt(oldState);
    std::clog << std::endl;
}


inline void hexDumpToClog2(const char* buffer, size_t length) {
    // 保存原始格式状态
    std::ios oldState(nullptr);
    oldState.copyfmt(std::clog);
    std::clog << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; ++i) {
        std::clog << std::hex << std::setfill('0')<< std::setw(2) << (static_cast<unsigned>(buffer[i]) & 0xFF) << "";
    }
    
    std::clog.copyfmt(oldState);
    std::clog << std::endl;
}



inline uint16_t TransU16(uint16_t a)
{
    return ((a) >> 8) | ((a) << 8);
}
inline uint32_t TransU32(uint32_t a)
{
    return (a = (((a) >> 16) | ((a) << 16))), (((a) >> 8) & 0x00ff00ff) | (((a) << 8) & 0xff00ff00);
}

inline uint64_t TransU64(uint64_t a)
{
    return (a = (((a) >> 32) | ((a) << 32))), (a = ((((a) >> 16) & 0x0000ffff0000ffff) | (((a) << 16) & 0xffff0000ffff0000))), ((((a) >> 8) & 0x00ff00ff00ff00ff) | (((a) << 8) & 0xff00ff00ff00ff00));
}

class OptionInfoFilter{
public:

    OptionInfoFilter():m_underlying{}{
    }
    void Load(std::string instfile){
        std::clog <<__func__<<","<< __LINE__<< ","<<instfile<< std::endl;
        std::ifstream if_inst(instfile,std::ios::in);
        std::string line;
        while (std::getline(if_inst, line)) {

            std::string_view line_view(line);
            size_t start = 0;
            size_t end = 0;
            int currentCol = 0;
            std::string_view underlying;
            std::string_view inst;
            while ((end = line_view.find(',', start)) != line_view.npos) {

                if (currentCol == 6) {
                    std::string_view inst2 = line_view.substr(start, end - start);
                    size_t pos = inst2.find('_');
                    inst = inst2.substr(pos + 1);
                }

                if (currentCol == 16) {
                    std::string_view underlying2 = line_view.substr(start, end - start);
                    size_t pos = underlying2.find('_');
                    underlying = underlying2.substr(pos + 1);
                }

                start = end + 1;
                currentCol ++;
            }

            if (!m_underlying.count((std::string)underlying)){
                continue;
            }
            m_underlying_inst.emplace((std::string)(inst), (std::string)(underlying));
            m_inst.emplace((std::string)(inst));
            std::clog <<__func__<<","<< __LINE__<< ","<<inst<<","<< underlying<< std::endl;
        }
        return;
    }


    bool LoadWithTitle(std::string instfile){
        std::clog <<__func__<<","<< __LINE__<< ","<<instfile<< std::endl;
        std::ifstream if_inst(instfile,std::ios::in);
        std::string line;
        std::getline(if_inst, line);
        while (std::getline(if_inst, line)) {
            // std::clog <<__func__<<","<< __LINE__<< ","<<line<< std::endl;
            std::string_view line_view(line);
            size_t start = 0;
            size_t end = 0;
            int currentCol = 0;
            std::string_view underlying;
            std::string_view inst;
            while ((end = line_view.find(',', start)) != line_view.npos) {

                if (currentCol == 6) {
                    std::string_view inst2 = line_view.substr(start, end - start);
                    size_t pos = inst2.find('_');
                    inst = inst2.substr(pos + 1);
                }

                if (currentCol == 16) {
                    std::string_view underlying2 = line_view.substr(start, end - start);
                    size_t pos = underlying2.find('_');
                    underlying = underlying2.substr(pos + 1);
                }

                start = end + 1;
                currentCol ++;
            }

            m_underlying_inst.emplace((std::string)(inst), (std::string)(underlying));
            m_inst.emplace((std::string)(inst));
            std::clog <<__func__<<","<< __LINE__<< ","<<inst<<","<< underlying<< std::endl;
        }
        return true;
    }

    bool Filter(std::string inst){
        if (m_underlying.count(inst) > 0) {
            return true;
        }

        return m_underlying_inst.count(inst)>0;
    }

    void AddUnderlying(std::string underlying){
        if (m_underlying.count(underlying) > 0) {
            return;
        }
        m_underlying.emplace(underlying);
        std::clog <<__func__<<","<< __LINE__<<",add underlying:"<< underlying<< std::endl;
    }

    void AppendInst(std::string inst,std::string underlying){
        if (m_underlying_inst.count(inst) > 0) {
            return;
        }
        m_underlying_inst.emplace(inst,underlying);
    }

    std::set<std::string> &GetAllInst(){
        return m_inst;
    }


private:
    std::unordered_map<std::string, std::string>   m_underlying_inst;   //inst->underlying
    std::set<std::string>                          m_underlying;
    std::set<std::string>                          m_inst;
};


inline void LoadFebaoInstrumentInfo(std::string &instfile, std::unordered_map<std::string, std::string> &underlying_inst_map){
    std::clog <<__func__<<","<< __LINE__<< ","<<instfile<< std::endl;
    std::ifstream if_inst(instfile,std::ios::in);
    std::string line;
    std::getline(if_inst, line);
    while (std::getline(if_inst, line)) {

        std::string_view line_view(line);
        size_t start = 0;
        size_t end = 0;
        int currentCol = 0;
        std::string_view underlying;
        std::string_view inst;
        while ((end = line_view.find(',', start)) != line_view.npos) {

            if (currentCol == 6) {
                std::string_view inst2 = line_view.substr(start, end - start);
                size_t pos = inst2.find('_');
                inst = inst2.substr(pos + 1);
            }

            if (currentCol == 16) {
                std::string_view underlying2 = line_view.substr(start, end - start);
                size_t pos = underlying2.find('_');
                underlying = underlying2.substr(pos + 1);
            }

            start = end + 1;
            currentCol ++;
        }

        underlying_inst_map.emplace((std::string)(inst), (std::string)(underlying));
        std::clog <<__func__<<","<< __LINE__<< ","<<inst<<","<< underlying<< std::endl;
    }
    return;
}


inline bool CreateDir(const std::string path) {
    // mode_t权限设置为755(rwxr-xr-x)
    const mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    if (mkdir(path.c_str(), mode) == 0) {
        return true;
    } else {
        printf("[%s] create failed.\n",path.c_str());
        return false;
    }
}

inline bool isDirExist(const std::string& path) {
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode));
}


inline bool isFileExist(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0){
        return false;
    }

    if (!S_ISREG(info.st_mode)) {
        return false;
    }

    return true;
}





class QXStats{


// inline void RANK(const std::vector<int>& v, std::vector<double> &ranks){
//     std::vector<size_t> idx(v.size());
//     std::iota(idx.begin(), idx.end(), 0);
//     std::sort(idx.begin(), idx.end(), [&v](size_t i, size_t j) { return v[i] < v[j]; });

//     ranks.resize(v.size());
//     for (size_t i = 0; i < idx.size(); ++i) {
//         ranks[idx[i]] = i + 1;
//     }
//     return;
// }


// // Spearman ic（Rank IC）
// double spearman_ic(const std::vector<double>& x, const std::vector<int>& y) {
//     if (x.size() != y.size() || x.empty()) return 0.0;

//     // 计算秩
//     std::vector<double> rank_x;
//     RANK(x, rank_x);
//     std::vector<double> rank_y;
//     RANK(y, rank_y);

//     // 计算皮尔逊相关系数（即斯皮尔曼IC）
//     double mean_x = std::accumulate(rank_x.begin(), rank_x.end(), 0.0) / rank_x.size();
//     double mean_y = std::accumulate(rank_y.begin(), rank_y.end(), 0.0) / rank_y.size();

//     double cov = 0.0, var_x = 0.0, var_y = 0.0;
//     for (size_t i = 0; i < rank_x.size(); ++i) {
//         cov += (rank_x[i] - mean_x) * (rank_y[i] - mean_y);
//         var_x += pow(rank_x[i] - mean_x, 2);
//         var_y += pow(rank_y[i] - mean_y, 2);
//     }

//     return cov / sqrt(var_x * var_y);
// }

// 计算信息比率（IR）
double information_ratio(const std::vector<double>& ics) {
    if (ics.empty()) return 0.0;
    double mean = std::accumulate(ics.begin(), ics.end(), 0.0) / ics.size();
    double variance = 0.0;
    for (double ic : ics) {
        variance += pow(ic - mean, 2);
    }
    double std_dev = sqrt(variance / ics.size());
    return mean / std_dev;
}

// 计算皮尔逊相关系数（Normal IC）
double pearson_corr(const std::vector<int>& x, const std::vector<int>& y) {
    if (x.size() != y.size() || x.empty()) return 0.0;

    double mean_x = std::accumulate(x.begin(), x.end(), 0.0) / x.size();
    double mean_y = std::accumulate(y.begin(), y.end(), 0.0) / y.size();

    double cov = 0.0, var_x = 0.0, var_y = 0.0;
    for (size_t i = 0; i < x.size(); ++i) {
        cov += (x[i] - mean_x) * (y[i] - mean_y);
        var_x += pow(x[i] - mean_x, 2);
        var_y += pow(y[i] - mean_y, 2);
    }

    return cov / sqrt(var_x * var_y);
}









};







// struct Char16Key {
//     char data[16];
//     bool operator==(const Char16Key& other) const {
//         return *reinterpret_cast<const uint64_t*>(data)   == *reinterpret_cast<const uint64_t*>(other.data) &&
//                *reinterpret_cast<const uint64_t*>(data+8) == *reinterpret_cast<const uint64_t*>(other.data+8);
//     }
// };

// namespace std {
//     template<> 
//     struct hash<Char16Key> {
//         size_t operator()(const Char16Key& key) const noexcept {
//             const uint64_t* ptr = reinterpret_cast<const uint64_t*>(key.data);
//             // 混合哈希算法：XXHash启发式
//             uint64_t h = ptr[0] ^ (ptr[1] * 0x9e3779b97f4a7c15);
//             h ^= h >> 33;
//             h *= 0xff51afd7ed558ccd;
//             h ^= h >> 33;
//             return static_cast<size_t>(h);
//         }
//     };
// }

// template<typename T>
// using InstrumentMap = std::unordered_map<Char16Key, T>;






USED_API inline std::string PriceDoubleToString(double value) {
    if (value == std::numeric_limits<double>::max()) {
        return "nan";
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6) << value;
    std::string str = oss.str();
    
    size_t dot_pos = str.find('.');
    if (dot_pos != std::string::npos) {
        str = str.substr(0, str.find_last_not_of('0') + 1);
        if (str.back() == '.') {
            str.pop_back();
        }
    }
    

    return str;

    // return std::to_string(value);
}




	template<typename T>
	USED_API static T StringToNumber(std::string &str){
		T number;
		std::istringstream datastr(str);
		datastr >> number;
		return number;
	}

	USED_API static std::string FloatTostring(float64_t &number){
		std::stringstream datastr;
		datastr<<std::setprecision(15)<<number;
		return datastr.str();
	}
	

	USED_API static bool IsNumber(std::string &str){
		std::stringstream datastr(str);
		double d;
		char c;
		if(!(datastr >> d)){
			return false;
		}
		if(datastr >>c){
			return false;
		}
		return true;
	}

	USED_API static void SplitString(const std::string &str, const char* delimiters, std::vector<std::string>& results){
		results.clear();
		uint32_t pos = str.find_first_not_of(delimiters);
		uint32_t pos2 = std::min(str.find_first_of(delimiters, pos),str.length());
		while(pos<pos2){
			std::string sub=str.substr(pos,pos2 - pos);
			results.push_back(sub);
			pos = str.find_first_not_of(delimiters, pos2);
			pos2 = std::min(str.find_first_of(delimiters, pos),str.length());
		}
	}

	USED_API static void MySplitString(const std::string &strs, char* delimiter,std::vector<std::string>& results){
		results.clear();
		if(strs.size()== 0)return;
		size_t Rpos = 0;
		size_t Lpos = strs.find(delimiter);
		while(Lpos !=strs.npos){
			results.emplace_back(strs.substr(Rpos, Lpos-Rpos));
			Rpos= Lpos+ 1;
			Lpos = strs.find(delimiter, Rpos);
		}
		if (Lpos== strs.npos && Rpos!=strs.npos){
			results.emplace_back(strs.substr(Rpos, strs.size() - Rpos));
		}
	}
			
    USED_API static void CSVStringSplit(const std::string& str, const char delimiter, std::vector<std::pair<size_t, size_t>> &results){
		//printf("[%s][%d],[%s]\n",func,LINE str.c str());
		results.clear();
		size_t Rpos = 0;
		size_t Lpos = str.find(delimiter);

		//.若找不到内容则字符串搜索函数返回·npos
		while(Lpos !=str.npos){
			results.emplace_back(std::make_pair(Rpos, Lpos-Rpos));
			//printf("[%s][%d],[%zu][%zu]\n",func,LINE,RpOs, Lpos-Rpos);
			Rpos = Lpos+ 1;
			Lpos = str.find(delimiter, Rpos);
		}
		if(Lpos == str.npos && Rpos!=str.npos){
			results.emplace_back(std::make_pair(Rpos,str.size()-Rpos));
			// printf("[%s][%d],[%zu][%zu]\n", func,LINE,Rpos, str.size()-Rpos);
		}
	}


	USED_API static bool Mycompare(const uint32_t &a, const uint32_t &b){
		return a<b;
	}
	
    USED_API static bool IsFileExist(const std::string &file){
		struct stat st;
		if(stat(file.c_str(),&st)!= 0){
			return false;
		}
		return true;
	}
	
	USED_API static bool IsFileEmpty(const std::string &file){
		struct stat st;
		if(stat(file.c_str(),&st)==0 && st.st_size ==0){
			return true;
		}
		return false;
	}
	
	USED_API static bool IsDirExist(std::string &dir){
		struct stat st;
		if(stat(dir.c_str(),&st)!= 0){
			return false;
		}
        //if(LOG_FALG) std::cout<< func <<","<< LINE <<", dir:"<<dir<<" already exist !!!"<<std::endl;
		return true;
	}
	
	USED_API static bool IsDirExist2(std::string &dir){
		int ret=access(dir.c_str(),R_OK);
		if(ret != 0){
            //if(LOG FALG) std::cout<< func <<","<< LINE <<",dir:"<<dir<<",ret:"<<ret<<std::endl;
			return false;
		}
		return true;
	}

	USED_API static bool IsDirEmpty(std::string &path){
		DIR *dir = opendir(path.c_str());
		if(nullptr == dir){
			return true;
		}

		struct dirent *sub_dir;
		while((sub_dir = readdir(dir))!= nullptr){
			std::string sub_path(sub_dir->d_name);
			if(sub_path != "." && sub_path ==".."){
				continue;
			}
			return false;
		}
		return true;
	}

	USED_API static std::string Trim(std::string &str){
		std::string ret= str;
		//std: :cout<<"Trim:"<<str<<":"<<(int)(str[@])<<"|"<<(int)(str[ret.size()-1])<<std::endl;
		while(ret.size()>0 &&(ret[0] ==' ' || ret[0] =='\"'|| ret[0] =='['|| ret[0] =='{'|| ret[0] ==']'|| ret[0] =='}'|| ret[0] =='\r'|| ret[0] =='\n')){
			ret = ret.substr(1,ret.size()-1);
		}
		
		while(ret.size()>0 &&(ret[ret.size()-1] == ' ' || ret[ret.size()-1] == '\"'|| ret[ret.size()-1] =='['|| ret[ret.size()-1] =='{'
		|| ret[ret.size()-1] ==']'|| ret[ret.size()-1] =='}'|| ret[ret.size()-1] =='\r'|| ret[ret.size()-1] =='\n')){
			ret = ret.substr(0,ret.size()-1);
		}
		return ret;
	}

    USED_API static int32_t ParserCacheToBinary(char *data, size_t len, std::string filename){
        if(IsFileExist(filename)==true){
            //LOG_NOTICE(filename+"overwrite" );
            //return RET_FILE_ALREADY_EXIST;
        }
        // LOG_TRACE(filename +" len:"+std::to_string(len));
        if(data ==nullptr || len == 0){
            LOG_ERROR(filename + " empty file");
            return RET_SUCCESS;
        }
        int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 00777);
        if (fd < 0){
            LOG_ERROR(filename +",open ERROR,fd:" + std::to_string(fd) + ",errno:" + + strerror(errno));
            return RET_FILE_ERROR;
        }
        lseek(fd, len-1, SEEK_END);
        write(fd, "", 1);
        
        char *buffer =(char *)mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if(buffer == MAP_FAILED){
            LOG_ERROR(filename +",mmap failed! MAP_FAILED errno:" + strerror(errno));
            return RET_MMAP_ERROR;
        }
        memcpy(buffer, data, len);
        close(fd);
        return RET_SUCCESS;
    }

    USED_API static int32_t WriteCacheToBinary(char *data, size_t len, std::string filename){
        if(IsFileExist(filename)==true){
            //LOG_NOTICE(filename+"overwrite" );
            //return RET_FILE_ALREADY_EXIST;
        }
        // LOG_TRACE(filename +" len:"+std::to_string(len));
        if(data ==nullptr || len == 0){
            LOG_ERROR(filename + " empty file");
            return RET_SUCCESS;
        }

        std::ofstream outstream(filename, std::ios::binary|std::ios::out|std::ios::trunc);
        outstream.write(data,len);
        outstream.flush();
        outstream.close();
        return RET_SUCCESS;
    }


    USED_API static int32_t ParserMulticacheToBinary(std::vector<std::pair<char *, size_t>> &data, std::string filename){
        if(IsFileExist(filename)==true){
            //LOG_DEBUG(filename +"overwrite" );
            //return RET_FILE_ALREADY_EXIST;
        }
        size_t tot_sz = 0;
        for(auto &it: data){
            tot_sz += it.second;
        }
        if(tot_sz == 0){
            // LOG_NOTICE(filename +"empty file");
            return RET_SUCCESS;
        }
        int fd = open(filename.c_str(),O_RDWR | O_CREAT | O_TRUNC, 00777);
        if(fd < 0){
            // LOG_NOTICE(filename +"open error");
            return RET_UNKOWN_ERROR;
        }
        // LOG_TRACE(filename + " tot sz:"+ std::to string(tot sz)+ ",fd:"+ std::to string(fd));
        lseek(fd, tot_sz-1,SEEK_END);
        write(fd,"",1);
        char *buffer =(char *)mmap(NULL, tot_sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if(buffer == MAP_FAILED){
            // LOG_FAULT(filename +"munmap error! RET MMAP ERROR:"+ std::to string(tot_sz));
            return RET_MMAP_ERROR;
        }
        char * ptr = buffer;
        for(auto &it: data){
            char *ss = it.first;
            size_t sz= it.second;
            memcpy(ptr,ss,sz);
            ptr += sz;
        }
        close(fd);
        return RET_SUCCESS;
    }

    USED_API static int32_t ListDirFile(std::string &path,std::vector<std::string> &resultout, uint32_t mask, int32_t mask_start, uint32_t mask_len){
        DIR *dir=opendir(path.c_str());
        if(nullptr == dir){
            // LOG_NOTICE(path + " RET_DIR_NOT_EXIST");
            return RET_DIR_NOT_EXIST;
        }
        
        std::set<std::string> unique_files;
        struct dirent *sub_dir;
        while((sub_dir=readdir(dir))!= nullptr){
            std::string name(sub_dir->d_name);
            if(name == "." || name == ".."){//202001.dat
                continue;
            }
            if(mask>0){ //则只包含year的文件
                if(mask_start<0){
                    mask_start =name.size()+mask_start;
                }
                std::string year_str = name.substr(mask_start, mask_len);//截取-18开始的4位为year 201711.dat
                uint32_t file_year = StringToNumber<uint32_t>(year_str);
                if(file_year != mask){
                    continue;
                }
            }
        
            std::string filename= path + "/"+ name;
            if(IsFileEmpty(filename)==true){
                // LOG_TRACE(filename +" RET_FILE_EMPTY");
                continue;
            }
            resultout.emplace_back(filename);   
            //LOG_DEBUG(filename + ",add");
        }
        if(resultout.size()==0){
            // LOG_NOTICE(path + " RET_NO_FILE_INCLUDE mask:" + std::to_string(mask));
            return RET_NO_FILE_INCLUDE;
        }
        std::sort(resultout.begin(),resultout.end());
        return RET_SUCCESS;
    }


USED_API static int32_t ListFile(std::string &path,std::vector<std::string> &resultout){
    DIR *dir=opendir(path.c_str());
    if(nullptr == dir){
        // LOG_NOTICE(path + " RET_DIR_NOT_EXIST");
        return RET_DIR_NOT_EXIST;
    }
    
    struct dirent *sub_dir;
    while((sub_dir=readdir(dir))!= nullptr){
        std::string name(sub_dir->d_name);
        if(name == "." || name == ".."){//202001.dat
            continue;
        }

        std::string filename= path + "/"+ name;
        if(IsFileEmpty(filename)==true){
            // LOG_TRACE(filename +" RET_FILE_EMPTY");
            continue;
        }
        resultout.emplace_back(filename);   
        //LOG_DEBUG(filename + ",add");
    }
    if(resultout.size()==0){
        // LOG_NOTICE(path + " RET_NO_FILE_INCLUDE mask:" + std::to_string(mask));
        return RET_NO_FILE_INCLUDE;
    }
    std::sort(resultout.begin(),resultout.end());
    return RET_SUCCESS;
}



USED_API static int32_t ListDir(const std::string &path, std::vector<std::string> &resultout){
    DIR *dir= opendir(path.c_str());
    if(nullptr == dir){
        // LOG_NOTICE(path + " RET_DIR_NOT_EXIST");
        return RET_DIR_NOT_EXIST;
    }
    struct dirent *sub_dir;
    while((sub_dir= readdir(dir))!= nullptr){
        std::string sub_dirname(sub_dir->d_name);
        if(sub_dirname == "." || sub_dirname == ".."){
            continue;
        }
        if(sub_dir->d_type == DT_DIR){
            resultout.emplace_back(sub_dirname);
            //LOG_TRACE(sub_dirname + " add");
        }
    }
    return RET_SUCCESS;
}


USED_API inline uint64_t time_str_to_utc_s(const char *time_str){
    int nums[4] = {0, 0, 0, 0};
    int numIdx = 0;
    
    const char *time = time_str;

    while (*time) {
        if (*time >= '0' && *time <= '9') {
            nums[numIdx] = nums[numIdx] * 10 + (*time - '0');
        } else if (*time == ':' || *time == '.') {
            numIdx++;
        }
        time++;
    }

    const int64_t HOURS_TO_MS = 3600000LL;
    const int64_t MIN_TO_MS   = 60000LL;
    const int64_t SEC_TO_MS   = 1000LL;

    return (nums[0] * HOURS_TO_MS) +(nums[1] * MIN_TO_MS) + (nums[2] * SEC_TO_MS) +nums[3];
}


inline uint64_t time_str_to_utc_s_2(const char *time_str){
    // 直接访问字符数组
    const char* h  = time_str;
    const char* m  = time_str + 3;
    const char* s  = time_str + 6;
    const char* ms = time_str + 9;

    int hours        = (h[0] - '0') * 10 + (h[1] - '0');
    int minutes      = (m[0] - '0') * 10 + (m[1] - '0');
    int seconds      = (s[0] - '0') * 10 + (s[1] - '0');
    int milliseconds = (ms[0] - '0') * 100 +(ms[1] - '0') * 10 +(ms[2] - '0');

    const int64_t HOURS_TO_MS = 3600000LL;
    const int64_t MIN_TO_MS   = 60000LL;
    const int64_t SEC_TO_MS   = 1000LL;
    return (hours * HOURS_TO_MS) +(minutes * MIN_TO_MS) + (seconds * SEC_TO_MS) +milliseconds;
}