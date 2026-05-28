#pragma once



#include <string>
#include <vector>



#include "common.h"


struct ProductSpecifications{
    std::string Exchange;
    std::string Product;
    double      PriceTick;
    uint32_t    Multiplier;

    std::vector<std::pair<uint32_t, uint32_t>> TradingSection;




};






class FutureStaticInfo{


public:

    std::string get_product(std::string inst){
        std::string ret = "";
        for (size_t i = 0; i< 3; ++i){   //最前面的3个字符，遇到数字就结束
            if (('0' <= inst[i]) && ('9' >= inst[i])){
                break;
            }
            ret += inst[i];
        }
        return ret;
    }



    uint32_t get_month(std::string inst){
        std::string mon_str = inst.substr(inst.length()-2, 2);
        uint32_t mon = StringToNumber<uint32_t>(mon_str);
        return mon;
    }

    uint32_t get_year(std::string inst){
        std::string ret("20");   //只支持2020-2029
        std::string pro = get_product(inst);
        std::string year_str = inst.substr(pro.length(), inst.length()-pro.length()-2);
        if (year_str.length() == 1){
            ret = ret + "2" + year_str;
        }
        else{
            ret = ret + year_str;
        }
        return  StringToNumber<uint32_t>(ret);
    }



    bool splitinst(const std::string &inst, std::string &product, uint32_t &year, uint32_t &month) {
        product.clear();
        year = 0;
        month = 0;
        if (inst.empty()) {
            return false;
        }

        std::string inst_upper = inst;
        std::transform(inst_upper.begin(), inst_upper.end(), inst_upper.begin(), ::toupper);

        size_t i = 0;
        while (i < inst_upper.length() && std::isalpha(inst_upper[i])) {
            i++;
        }

        if (i == 0) {
            return false;
        }

        product = inst.substr(0, i);
        size_t date_start = i;
        std::string date_digits;

        while (date_start < inst_upper.length() && std::isdigit(inst_upper[date_start])) {
            date_digits += inst_upper[date_start];
            date_start++;
        }

        if (date_digits.length() < 3 || date_digits.length() > 4) {
            return false;
        }

        if (date_digits.length() == 4) {
            year = std::stoi(date_digits.substr(0, 2)) + 2000;
            month = std::stoi(date_digits.substr(2, 2));
        } else if (date_digits.length() == 3) {
            int year_digit = date_digits[0] - '0';
            if (year_digit <= 2) {
                year = 2020 + year_digit;
            } else {
                year = 2020 + year_digit; // For 6, it becomes 2006, but might need adjustment
            }
            month = std::stoi(date_digits.substr(1, 2));
        }
        if (month < 1 || month > 12) {
            return false;
        }
    }




    uint32_t get_future_last_tradingday(std::string inst){
        std::string product = get_product(inst);
        uint32_t year = get_year(inst);
        uint32_t month = get_month(inst);

        std::clog <<__func__<<","<< __LINE__<<",product[" <<product.c_str()<<",year:"<<year<<",month:"<<month<<std::endl;

        std::vector<uint32_t>  tradingday_mon;
        GenTradingDay(year*10000+month*100+1,  year*10000+month*100+31,  tradingday_mon);

        if (tradingday_mon.size() < 10){
            std::clog <<__func__<<","<< __LINE__<<",error,product[" <<product.c_str()<<"],year:"<<year<<"],month:"<<month<<"],size:"<<tradingday_mon.size()<< std::endl;
            return 0;
        }

        if (product == "A" || product == "C" || product == "BB" || product == "CS" || product == "M" || product == "Y" || product == "B" || product == "P" 
         || product == "RR" || product == "FB" || product == "JM" || product == "J" || product == "I" || product == "L" || product == "V" || product == "PP"
         || product == "WH" || product == "SR" || product == "LR" || product == "OI" || product == "CF" || product == "RI" || product == "JR" || product == "PM"
         || product == "RS" || product == "CJ" || product == "RM" || product == "CY" || product == "PK" || product == "AP" || product == "TA"
         || product == "UR" || product == "PX" || product == "PL" || product == "MA" || product == "SF" || product == "SA" || product == "SH" || product == "FG"
         || product == "SM" || product == "PF" || product == "PR" || product == "ps" || product == "lc" || product == "si" || product == "pt" || product == "pd")
        {
            return tradingday_mon[9];  // 合约交割月份的第10个交易日
        }
        else if(product == "ZC"){
            return tradingday_mon[4]; // 合约月份第5个交易日
        }
        else if(product == "LG" || product == "LH" || product == "EG" || product == "EB" || product == "PG" || product == "BZ" || product == "JD"){
            return tradingday_mon[tradingday_mon.size()-4]; // 合约月份倒数第4个交易日
        }
        else if(product == "cu" || product == "bc" || product == "al" || product == "zn" || product == "pb" || product == "ni" || product == "sn" || product == "ao"
             || product == "ad" || product == "au" || product == "ag" || product == "rb" || product == "wr" || product == "hc" || product == "ss" || product == "bu"
             || product == "br" || product == "ru" || product == "nr" || product == "sp" || product == "op" || product == "hc" || product == "ss" || product == "bu"){

            for (auto &day : tradingday_mon){
                if (day%100 >=15){    //合约月份的15日,假期递延
                    return day;
                }
            }
        }
        else if(product == "sc" || product == "lu" || product == "fu"){
            std::vector<uint32_t>  tradingday_premon;                       
            if (month == 1){
                GenTradingDay((year-1)*10000+12*100+1,  (year-1)*10000+12*100+31,  tradingday_premon);
            }
            else{
                GenTradingDay(year*10000+(month-1)*100+1, year*10000+(month-1)*100+31,  tradingday_premon);
            }
            return tradingday_premon[tradingday_premon.size()-1];  //合约月份前一月份的最后一个交易日           
        }
        else if(product == "ec"){
            //合约交割月份最后一个交易的周一
            std::vector<uint32_t> day_mon;
            GenNatureDay(year*10000 + month*100 + 1, year*10000 + month*100 + 31, day_mon);
            uint32_t ret_day = 0;
            int32_t i = 0;
            for (i = day_mon.size()-1; i >= 0; i--){
                int32_t w = getWeekDay(day_mon[i]);
                if (w==1){
                   break;
                }
            }

            for (int32_t j = i; j <day_mon.size(); j++){
                if(isTradingDay(day_mon[j])){
                    ret_day = day_mon[j];
                }
            }

            if(ret_day==0){
                for (int32_t j = i; i >=0; j--){
                    if(isTradingDay(day_mon[j])){
                        ret_day = day_mon[j];
                    }
                }
            }
            return ret_day;
        }
        else if(product == "IF" || product == "IC" || product == "IM" || product == "IH"){

            //合约到期月份的第三个周五

            std::vector<uint32_t> day_mon;
            GenNatureDay(year*10000 + month*100 + 1, year*10000 + month*100 + 31, day_mon);
            uint32_t ret_day = 0;
            int32_t i = 0;
            int count = 0;
            for (i = 0; i <= day_mon.size(); i ++){
                int32_t w = getWeekDay(day_mon[i]);
                if (w == 5){
                   count++;
                }

                if (count ==3){
                    break;
                }
            }

            for (int32_t j = i; i <day_mon.size(); j++){
                if(isTradingDay(day_mon[j])){
                    ret_day = day_mon[j];
                }
            }
            return ret_day;
        }
        else if(product == "TS" || product == "TF" || product == "T" || product == "TL"){

            //合约到期月份的第二个周五,如果不是交易日则顺延
            std::vector<uint32_t> day_mon;
            GenNatureDay(year*10000 + month*100 + 1, year*10000 + month*100 + 31, day_mon);
            uint32_t ret_day = 0;
            int32_t i = 0;
            int count = 0;
            for (i = 0; i <= day_mon.size(); i ++){
                int32_t w = getWeekDay(day_mon[i]);
                if (w == 5){
                   count++;
                }

                if (count ==2){
                    break;
                }
            }

            for (int32_t j = i; i <day_mon.size(); j++){
                if(isTradingDay(day_mon[j])){
                    ret_day = day_mon[j];
                }
            }
            return ret_day;
        }
        return 0;
    }





    uint32_t get_option_last_tradingday(std::string inst){
        std::string product = "";
        uint32_t year = 0;
        uint32_t month = 0;
        if (false == splitinst(inst, product, year, month)){
            std::clog <<__func__<<","<< __LINE__<<",inst decode error[" <<inst.c_str()<<std::endl;
            return 0;
        }

        std::clog <<__func__<<","<< __LINE__<<",product[" <<product.c_str()<<",year:"<<year<<",month:"<<month<<std::endl;

        // uint32_t ret = 0;
        // std::vector<uint32_t> tradingday_mon;
        // GenTradingDay(year*10000+month*100+1,  year*10000+month*100+31,  tradingday_mon);

        std::vector<uint32_t>  tradingday_premon;     
        std::vector<uint32_t>  day_premon;
        if (month == 1){
            GenTradingDay((year-1)*10000+12*100+1,  (year-1)*10000+12*100+31,  tradingday_premon);
            GenNatureDay((year-1)*10000+12*100+1,  (year-1)*10000+12*100+31,  day_premon);
        }
        else{
            GenTradingDay(year*10000+(month-1)*100+1, year*10000+(month-1)*100+31,  tradingday_premon);
            GenNatureDay(year*10000+(month-1)*100+1, year*10000+(month-1)*100+31,  day_premon);
        }

        std::vector<uint32_t>  tradingday_prepremon;
        std::vector<uint32_t>  day_prepremon;
        if (month == 1){
            GenTradingDay((year-1)*10000+11*100+1,  (year-1)*10000+11*100+31,  tradingday_prepremon);
            GenNatureDay((year-1)*10000+11*100+1,  (year-1)*10000+11*100+31,  day_premon);
        }
        else if (month == 2){
            GenTradingDay((year-1)*10000+12*100+1,  (year-1)*10000+12*100+31,  tradingday_prepremon);
            GenNatureDay((year-1)*10000+12*100+1,  (year-1)*10000+12*100+31,  day_premon);
        }
        else{
            GenTradingDay(year*10000+(month-1)*100+1, year*10000+(month-1)*100+31,  tradingday_prepremon);
            GenNatureDay(year*10000+(month-1)*100+1, year*10000+(month-1)*100+31,  day_premon);
        }

        if (tradingday_premon.size() < 12) {
            std::clog <<__func__<<","<< __LINE__<<",product[" <<product.c_str()<<"],year:"<<year<<"],month:"<<month<<"],size:"<<tradingday_premon.size()<< std::endl;
            return;
        }

        if (product == "A" || product == "C" || product == "CS" || product == "B" || product == "M" || product == "Y" || product == "P" || product == "JD"
         || product == "LG" || product == "LH" || product == "JM" || product == "I" || product == "L" || product == "V" || product == "PP"
         || product == "EG" || product == "EB" || product == "PG" || product == "BZ" )
        {
            //标的期货合约交割月份前一个月的第12个交易日
            return tradingday_premon[11];
        }
        else if (product == "SR"){
            //常规期权：标的期货合约交割月份前一个月第15个日历日之前（含该日）的倒数第3个交易日     SR605C5800
            //系列期权：标的期货合约交割月份前两个月第15个日历日之前（含该日）的倒数第3个交易日     SR605MSP5700
            size_t ms_pos = inst.find("MS");
            if(ms_pos == std::string::npos){     //常规期权
                for (int i = 14; i >=0 ; i--){
                    int j = 0;
                    if(isTradingDay(day_premon[i])){
                        j++;
                    }
                    if (j==3){
                        return day_premon[i];
                    }
                }
            }
            else{                                //系列期权
                for (int i = 14; i >=0 ; i--){
                    int j = 0;
                    if(isTradingDay(day_prepremon[i])){
                        j++;
                    }
                    if (j==3){
                        return day_prepremon[i];
                    }
                }
            }
        }
        else if (product == "CJ" || product == "AP" || product == "PX"){
            //标的期货合约交割月份前两个月最后一个日历日之前（含该日）的倒数第3个交易日
            for (int i = day_prepremon.size()-1; i >=0 ; i--){
                int j = 0;
                if(isTradingDay(day_prepremon[i])){
                    j++;
                }
                if (j==3){
                    return day_prepremon[i];
                }
            }
        }
        else if(product == "OI" || product == "RM" || product == "PK" || product == "CF" || product == "TA" || product == "ZC" || product == "UR" || product == "PL" 
             || product == "MA" || product == "SF" || product == "SA" || product == "SH" || product == "FG" || product == "SM" || product == "PF" || product == "PR"){
            // 标的期货合约交割月份前一个月第15个日历日之前（含该日）的倒数第3个交易日
            for (int i = 14; i >=0 ; i--){
                int j = 0;
                if(isTradingDay(day_premon[i])){
                    j++;
                }
                if (j==3){
                    return day_premon[i];
                }
            }
        }
        else if (product == "cu" || product == "al" || product == "zn" || product == "pb" || product == "ni" || product == "sn" || product == "ao" || product == "ad"
              || product == "au" || product == "ag" || product == "rb"){
            //标的期货合约交割月前第一月的倒数第五个交易日
            return tradingday_premon[tradingday_premon.size()-5];
        }
        else if (product == "sc"){
            //标的期货合约交割月前第一月的倒数第13个交易日
            return tradingday_premon[tradingday_premon.size()-13];
        }
        else if (product == "lu" || product == "fu"){
            //标的期货合约交割月前第一月的倒数第十个交易日
            return tradingday_premon[tradingday_premon.size()-10];
        }
        else if (product == "bu" || product == "br" || product == "ru" || product == "sp" || product == "op"){
            //标的期货合约交割月前第一月的倒数第五个交易日
            return tradingday_premon[tradingday_premon.size()-5];
        }
        else if (product == "IO" || product == "MO" || product == "HO"){
            //合约到期月份的第三个星期五
            std::vector<uint32_t> day_mon;
            GenNatureDay(year*10000 + month*100 + 1, year*10000 + month*100 + 31, day_mon);
            uint32_t ret_day = 0;
            int32_t i = 0;
            int count = 0;
            for (i = 0; i <= day_mon.size(); i ++){
                int32_t w = getWeekDay(day_mon[i]);
                if (w == 5){
                   count++;
                }

                if (count ==3){
                    break;
                }
            }

            for (int32_t j = i; i <day_mon.size(); j++){
                if(isTradingDay(day_mon[j])){
                    ret_day = day_mon[j];
                }
            }
            return ret_day;
        }
        return 0;
    }




private:




}








