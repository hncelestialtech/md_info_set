#pragma once



#include <stdint.h>

#include <vector>
#include <string>

#include <unordered_map>

#include <sys/time.h>


#include "macro.h"
















class TradingCalendar{

public:

    TradingCalendar(){
    }

    ~TradingCalendar(){
    }

    //是否闰年
    bool IsLeep(){
        return false;
    }

    //某一天是这一年的第几天
    uint32_t GetDayOfYear(uint32_t yyyymmdd){
        return 0;
    }

    //某一天是周几
    uint32_t GetWeek(uint32_t yyyymmdd){
        return 0;
    }

    //某一天是这一年的第几周
    uint32_t GetWeekNo(uint32_t yyyymmdd){
        return 0;
    }


    bool IsTradingDay(uint32_t yyyymmdd){
        return true;
    }


    uint32_t PreTradingDay(uint32_t yyyymmdd){
        return 0;
    }

    uint32_t NextTradingDay(uint32_t yyyymmdd){
        return 0;
    }

    //获取一年的交易日
    void GetTradingDayOfYear(uint32_t yyyy, std::vector<uint32_t> &vec){

    }

    //获取一年的交易日
    void GetTradingDayOfMonth(uint32_t yyyymm, std::vector<uint32_t> &vec){

    }


private:


private:


};






class TradingInterval{




public:




    void GetIntervals(std::string product, uint32_t date, std::vector<uint32_t> &vec){

    }

    void GetIntervals(std::string product, std::vector<uint32_t> &vec){

    }

    void GetIntervals(std::vector<uint32_t> &vec){

    }







};













































