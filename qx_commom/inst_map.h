#pragma once

#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>

#include "macro.h"


#pragma pack(1)

const size_t  INST_LEN_MAX = 16;


    std::string uint128_to_hex_string(uint128_t value) {
        constexpr size_t hex_size = 32 + 1; // 32个十六进制字符 + null终止符
        char buffer[hex_size];

        // 将内存直接转为十六进制
        const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&value);

        // 小端序转大端序显示（可选，根据需求）
        static const char* hex_chars = "0123456789ABCDEF";
        for (int i = 0; i < 16; ++i) {
            // std::cout<<bytes[i]<<std::endl;
            unsigned char byte = bytes[ i]; // 反转字节顺序，按大端序显示
            buffer[i * 2] = hex_chars[byte >> 4];
            buffer[i * 2 + 1] = hex_chars[byte & 0x0F];
        }
        buffer[32] = '\0';

        return std::string(buffer);
    }


template<typename T>
class InstMap {


public:

    __attribute__((always_inline))
    void InitOnce(const char* inst) {
        Item kv = {};
        kv.key = chars_to_uint128(inst);
        // std::clog <<__func__<<","<< __LINE__<<",inst:"<<inst<<",key:"<<uint128_to_hex_string(kv.key).c_str()<< std::endl;
        m_data.push_back(kv);
    }

    __attribute__((always_inline))
    void InitOnce(const std::string &inst) {
        Item kv = {};
        kv.key = chars_to_uint128(inst);
        // std::clog <<__func__<<","<< __LINE__<<",inst:"<<inst.c_str()<<",key:"<<uint128_to_hex_string(kv.key).c_str()<< std::endl;
        m_data.push_back(kv);
    }

    __attribute__((always_inline))
    void Sort() {
        std::sort(m_data.begin(), m_data.end());
        m_sorted = true;
        m_data.shrink_to_fit();
    }

    __attribute__((always_inline))
    T* Find(const char* key) {
        uint128_t search_key = chars_to_uint128(key);
        // std::clog <<__func__<<","<< __LINE__<<",key:"<<key<<",key:"<<uint128_to_hex_string(search_key).c_str()<< std::endl;
        auto it = std::lower_bound(m_data.begin(), m_data.end(), search_key,
            [](const Item& kv, uint128_t k) {

                // std::clog <<__func__<<","<< __LINE__<<",kv.key:"<<uint128_to_hex_string(kv.key).c_str()<<",key:"<<uint128_to_hex_string(k).c_str()<< std::endl;
                return kv.key < k;
            });
            
        if (it != m_data.end() && it->key == search_key) {
            // std::clog <<__func__<<","<< __LINE__<<",key:"<<key<<",key:"<<uint128_to_hex_string(search_key).c_str()<< std::endl;
            return &it->value;
        }
        // std::clog <<__func__<<","<< __LINE__<<",key:"<<key<<",key:"<<uint128_to_hex_string(search_key).c_str()<< std::endl;
        return nullptr;
    }

    __attribute__((always_inline))
    size_t size(){
        return m_data.size();
    }

private:

    __attribute__((always_inline))
    static uint128_t chars_to_uint128(const char* str) {
        uint128_t result = 0;
        size_t len = std::min(strlen(str), INST_LEN_MAX);
        memcpy(&result, str, len); // 注意，最多只拷贝前16字节
        memset((char *)(&result) + len, 0 , INST_LEN_MAX - len);
        return result;
    }

    __attribute__((always_inline))
    static uint128_t chars_to_uint128(const std::string &str){
        uint128_t result = 0;
        size_t len = std::min(str.length(), INST_LEN_MAX);
        memcpy(&result, str.data(), len); // 注意，最多只拷贝前16字节
        memset((char *)(&result) + len, 0 , INST_LEN_MAX - len);
        return result;
    };

    // std::string uint128_to_hex_string(uint128_t value) {
    //     constexpr size_t hex_size = 32 + 1; // 32个十六进制字符 + null终止符
    //     char buffer[hex_size];

    //     // 将内存直接转为十六进制
    //     const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&value);

    //     // 小端序转大端序显示（可选，根据需求）
    //     static const char* hex_chars = "0123456789ABCDEF";
    //     for (int i = 0; i < 16; ++i) {
    //         // std::cout<<bytes[i]<<std::endl;
    //         unsigned char byte = bytes[ i]; // 反转字节顺序，按大端序显示
    //         buffer[i * 2] = hex_chars[byte >> 4];
    //         buffer[i * 2 + 1] = hex_chars[byte & 0x0F];
    //     }
    //     buffer[32] = '\0';

    //     return std::string(buffer);
    // }


    // 初始化
    // void Init(const std::vector<std::pair<const char*, QuotaInfo>>& items) {
    //     m_data.reserve(items.size());

    //     for (const auto& item : items) {
    //         Item kv;
    //         kv.key = chars_to_uint128(item.first);
    //         kv.value = item.second;
    //         m_data.push_back(kv);
    //     }
        
    //     std::sort(m_data.begin(), m_data.end());
    //     m_sorted = true;
    //     m_data.shrink_to_fit();
    // }
    



    // __attribute__((always_inline))
    // bool Exist(const char* key) const {
    //     uint128_t search_key = chars_to_uint128(key);
        
    //     // 二分查找
    //     auto it = std::lower_bound(m_data.begin(), m_data.end(), search_key,
    //         [](const Item& kv, uint128_t k) {
    //             return kv.key < k;
    //         });
            
    //     return it != m_data.end() && it->key == search_key;
    // }
    
    void printKey(){
        std::clog <<__func__<<","<< __LINE__<<",m_sorted:"<<m_sorted<< std::endl;
        for(auto &k: m_data){
            std::clog <<__func__<<","<< __LINE__<<",key:"<<uint128_to_hex_string(k).c_str()<< std::endl;
        }

    }



private:
    struct Item {
        uint128_t key;
        T value;
        
        bool operator<(const Item& other) const {
            return key < other.key;
        }
    };
    
    std::vector<Item> m_data;
    bool              m_sorted = false;

};


#pragma pack()








