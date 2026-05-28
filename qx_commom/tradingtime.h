#pragma once

#include <ctime>
#include <chrono>
#include <x86intrin.h>

#include <sys/time.h>
#include <cstdint>

#include <cstring>

#include <set>

#include "macro.h"


static std::set<uint32_t> g_holiday={
    20030101,20030130,20030131,20030203,20030204,20030205,20030206,20030207,20030501,20030502,20030505,20030506,20030507,20031001,20031002,20031003,20031006,20031007,
    20040101,20040119,20040120,20040121,20040122,20040123,20040126,20040127,20040128,20040503,20040504,20040505,20040506,20040507,20041001,20041004,20041005,20041006,20041007,
    20050103,20050207,20050208,20050209,20050210,20050211,20050214,20050215,20050502,20050503,20050504,20050505,20050506,20051003,20051004,20051005,20051006,20051007,
    20060102,20060103,20060130,20060131,20060201,20060202,20060203,20060501,20060502,20060503,20060504,20060505,20061002,20061003,20061004,20061005,20061006,
    20070101,20070102,20070103,20070219,20070220,20070221,20070222,20070223,20070501,20070502,20070503,20070504,20070507,20071001,20071002,20071003,20071004,20071005,20071231,
    20080101,20080206,20080207,20080208,20080211,20080212,20080404,20080501,20080502,20080609,20080915,20080929,20080930,20081001,20081002,20081003,
    20090101,20090102,20090126,20090127,20090128,20090129,20090130,20090406,20090501,20090528,20090529,20091001,20091002,20091005,20091006,20091007,20091008,
    20100101,20100215,20100216,20100217,20100218,20100219,20100405,20100503,20100614,20100615,20100616,20100922,20100923,20100924,20101001,20101004,20101005,20101006,20101007,
    20110103,20110202,20110203,20110204,20110207,20110208,20110404,20110405,20110501,20110502,20110606,20110912,20111003,20111004,20111005,20111006,20111007,
    20120102,20120103,20120123,20120124,20120125,20120126,20120127,20120402,20120403,20120404,20120430,20120501,20120622,20121001,20121002,20121003,20121004,20121005,
    20130101,20130102,20130103,20130211,20130212,20130213,20130214,20130215,20130404,20130405,20130429,20130430,20130501,20130610,20130611,20130612,20130919,20130920,20131001,20131002,20131003,20131004,20131007,
    20140101,20140131,20140203,20140204,20140205,20140206,20140407,20040501,20140502,20140602,20140908,20141001,20141002,20141003,20141006,20141007,
    20150101,20150102,20150218,20150219,20150220,20150223,20150224,20150406,20150501,20150622,20150903,20150904,20151001,20151002,20151005,20151006,20151007,
    20160101,20160208,20160209,20160210,20160211,20160212,20160404,20160502,20160609,20160610,20160915,20160916,20161003,20161004,20161005,20161006,20161007,
    20170102,20170127,20170130,20170131,20170201,20170202,20170403,20170404,20170501,20170529,20170530,20171002,20171003,20171004,20171005,20171006,
    20180101,20180215,20180216,20180219,20180220,20180221,20180405,20180406,20180430,20180501,20180618,20180924,20181001,20181002,20181003,20181004,20181005,20181231,
    20190101,20190204,20190205,20190206,20190207,20190208,20190405,20190501,20190502,20190503,20190607,20190913,20191001,20191002,20191003,20191004,20191007,
    20200101,20200124,20200127,20200128,20200129,20200130,20200131,20200406,20200501,20200504,20200505,20200625,20200626,20201001,20201002,20201005,20201006,20201007,20201008,
    20210101,20210211,20210212,20210215,20210216,20210217,20210405,20210503,20210504,20210505,20210614,20210920,20210921,20211001,20211004,20211005,20211006,20211007,
    20220103,20220131,20220201,20220202,20220203,20220204,20220404,20220405,20220502,20220503,20220504,20220603,20220912,20221003,20221004,20221005,20221006,20221007,
    20230102,20230123,20230124,20230125,20230126,20230127,20230405,20230501,20230502,20230503,20230622,20230623,20230929,20231002,20231003,20231004,20231005,20231006,
    20240101,20240212,20240213,20240214,20240215,20240216,20240404,20240405,20240501,20240502,20240503,20240610,20240916,20240917,20241001,20241002,20241003,20241004,20241007, 
    20250101,20250128,20250120,20250130,20250131,20250203,20250404,20250501,20250502,20250505,20250602,20251001,20251002,20251003,20251006,20251007,20251008,       
    20260101,20260102,20260216,20260217,20260218,20260219,20260220,20260223,20260406,20260501,20260504,20260505,20260619,20260925,20261001,20261002,20261005,20261006,20261007
};



USED_API static void SetTimeZone(const char* timezone = "Etc/UTC"){
    setenv("Tz",timezone, 1);
    tzset();
}

USED_API static void UnsetTimeZone(){
    unsetenv("TZ");	
}

USED_API static std::string getsysdatetime(){
    time_t sys_now = time(0);
    sys_now += 8 *3600;
    tm *t = gmtime(&sys_now);
    char today_str[64];
    sprintf(today_str,"%04u%02u%02u %02u:%02u:%02u",t->tm_year + 1908,t->tm_mon + 1, t->tm_mday,t->tm_hour, t->tm_min, t->tm_sec);
    return(std::string)today_str;
}

USED_API static std::string getsysdatetime2(){
    time_t sys_now = time(0);
    sys_now += 8 *3600;
    tm *t = gmtime(&sys_now);
    char today_str[64];
    sprintf(today_str,"%04u%02u%02u%02u%02u%02u",t->tm_year + 1908,t->tm_mon + 1, t->tm_mday,t->tm_hour, t->tm_min, t->tm_sec);
    return(std::string)today_str;
}



USED_API static long long getsystimems(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long ret =tv.tv_sec*1000LL+ tv.tv_usec/100LL;
    return ret;
}

USED_API static long long getsystimeus(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long ret = tv.tv_sec*1000LL*1000LL + tv.tv_usec;
    return ret;
}

USED_API static std::string getsysdate(){
    time_t sys_now = time(0);
    sys_now += 8*3600;
    tm *t= gmtime(&sys_now);
    char today_str[20];
    sprintf(today_str,"%04u%02u%02u",t->tm_year + 1900,t->tm_mon + 1,t->tm_mday);
    return(std::string)today_str;
}

USED_API static int32_t getsysdate2(){
    time_t sys_now = time(0);
    sys_now += 8*3600;
    tm *t= gmtime(&sys_now);
    return (t->tm_year + 1900) * 10000 + (t->tm_mon + 1)*100+t->tm_mday;
}




USED_API inline void utc_to_local(int32_t utc_seconds,int32_t& date_yyyymmdd,int32_t& time_hhmmss,int32_t timezone_offset = 0) {
    time_t adjusted_time = static_cast<time_t>(utc_seconds) + timezone_offset * 3600;
    
    struct tm timeinfo;
    gmtime_r(&adjusted_time, &timeinfo);

    date_yyyymmdd = (timeinfo.tm_year + 1900) * 10000 + 
                   (timeinfo.tm_mon + 1) * 100 + 
                   timeinfo.tm_mday;

    time_hhmmss = timeinfo.tm_hour * 10000 + timeinfo.tm_min * 100 + timeinfo.tm_sec;
}





inline int getCurrentSeconds() {
    time_t now = time(nullptr);
    struct tm *local = localtime(&now);
    return local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;
}

inline int getCurrentDaySeconds() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    struct tm *local_time = std::localtime(&now_time);
    return local_time->tm_hour * 3600 + local_time->tm_min * 60 + local_time->tm_sec;
}

inline int getCurrentTimes() {

    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    struct tm *local_time = std::localtime(&now_time);
    return local_time->tm_hour * 10000 + local_time->tm_min * 100 + local_time->tm_sec;
}

inline int getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    struct tm *local_time = std::localtime(&now_time);
    return (local_time->tm_year+1900) *10000+ (local_time->tm_mon + 1)*100 + local_time->tm_mday;
}






inline uint64_t get_nanoseconds() {
#ifdef __linux__
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
#else
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
#endif
}

//用于统计运行间隔，有低于1%的误差
class NanoStamp {
public:
    NanoStamp() { 
        reset();
        // printf("[%lld]\n", m_ns_per_cycle);
    }

    __attribute__((always_inline)) 
    uint64_t stamp() {
        const uint64_t new_tsc = read_tscp();
        const uint64_t old_tsc = m_last_tsc;
        m_last_tsc = new_tsc;

        if (__builtin_expect(old_tsc == 0, 0)) return 0;
        return ((new_tsc - old_tsc) * m_ns_per_cycle)/1000000;
    }

private:

    void reset(){
        constexpr uint64_t CAL_NS = 500'000'000; // 500ms
        timespec ts1, ts2;
        
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts1);
        uint64_t tsc1 = read_tscp();
        do {
            clock_gettime(CLOCK_MONOTONIC_RAW, &ts2);
        } while ((ts2.tv_sec - ts1.tv_sec)*1e9 +  (ts2.tv_nsec - ts1.tv_nsec) < CAL_NS);
        uint64_t tsc2 = read_tscp();
        
        uint64_t elapsed_ns = (ts2.tv_sec - ts1.tv_sec)*1e9 + (ts2.tv_nsec - ts1.tv_nsec);
        m_ns_per_cycle = elapsed_ns*1000000/ (tsc2 - tsc1);
    }

    static uint64_t read_tscp() {
        unsigned int aux;
        return __rdtscp(&aux);
    }



    // __attribute__((always_inline)) 
    // static uint64_t rdtscp() {
    //     uint32_t lo, hi, aux;     
    //     __asm__ __volatile__("rdtscp" : "=a"(lo), "=d"(hi), "=c"(aux));     
    //     return ((uint64_t)hi << 32) | lo; 
    // } 


private:
    alignas(64) uint64_t m_last_tsc = 0;
    uint64_t             m_ns_per_cycle = 0;
};





inline uint32_t hhmmssToUtcSeconds(int hhmmss) noexcept {
    const int hours = hhmmss / 10000;
    const int minutes = (hhmmss / 100) % 100;
    const int seconds = hhmmss % 100;

    const auto now = std::chrono::system_clock::now();
    const auto today = std::chrono::system_clock::to_time_t(now);
    const auto utc_midnight = today - (today % 86400); // 86400=1天,28800=8小时

    return static_cast<uint32_t>(utc_midnight + hours*3600 + minutes*60 + seconds);
}


inline uint32_t hhmmssToUtcSeconds2(int hhmmss) noexcept {
    const int hours = hhmmss / 10000;
    const int minutes = (hhmmss / 100) % 100;
    const int seconds = hhmmss % 100;
    return hours*3600+minutes*60+seconds;
}




inline uint64_t yyyymmddhhmmssmsToUtcSecondsEast8(int64_t timestamp) noexcept {
    int64_t ymd = timestamp/1000000000;  //20250804
    int64_t hms =  timestamp % 1000000000; //150331000
    const int year  = ymd / 10000;
    const int month = (ymd % 10000) / 100;
    const int day   = ymd % 100;
    const int hour  = hms / 10000000;
    const int min   = (hms % 10000000) / 100000;
    const int sec   = (hms % 100000)/1000;

    std::tm tm = {};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;
    tm.tm_isdst = 0;

    std::time_t time = std::mktime(&tm);
    return static_cast<int64_t>(time + 8*3600) ;  //东八区
}


inline uint64_t yyyymmddhhmmssmsToUtcSeconds(int64_t timestamp, bool East8 = false) noexcept {
    int64_t ymd = timestamp/1000000L;  //20250804
    int64_t hms =  timestamp % 1000000L; //150331000
    std::tm tm = {};
    tm.tm_year  = (ymd / 10000) -1900;
    tm.tm_mon = ((ymd % 10000) / 100) -1;
    tm.tm_mday   = ymd % 100;
    tm.tm_hour  = hms / 10000;
    tm.tm_min   = (hms % 10000) / 100;
    tm.tm_sec   = hms % 100;
    tm.tm_isdst = 0;
    std::time_t time = std::mktime(&tm);
    if (East8){
        time += 8*3600;
    }
    return static_cast<int64_t>(time) ;
}

//yymmddhhmmssms
//20240805094523567
inline uint64_t yyyymmddhhmmssmsToUtcSeconds2(int64_t timestamp) noexcept {
    int64_t hms =  (timestamp % 1000000000)/1000; //150331000
    return hhmmssToUtcSeconds2(hms) ;  //东八区
}

USED_API static bool isLeapYear(uint32_t year){
    if((year%4==0 && year%100!=0) || year%400 == 0 ){
        return true;
    }
    return false;
}

//0-364/365
USED_API static int32_t getDayOfYear(uint32_t date){
    static uint32_t month_days[]={0,31,59,90,120,151,181,212,243,273,304,334};
    static uint32_t month_days_leap[]={0,31,60,91,121,152,182,213,244,274,305,335};
    uint32_t y = date/10000;
    uint32_t m =(date%10000)/100;
    uint32_t d = date%100;
    if(isLeapYear(y)){
        return month_days_leap[m-1]+d-1;
    }
    return month_days[m-1]+ d - 1;
}


//年月日·时分秒·20230830·185830
USED_API static long long getMkTime(uint32_t date, uint32_t hms){
    //预先存每一年的起始uts秒,即次年1.100:00:00的utc秒
    static std::unordered_map<uint32_t,long long> yearseconds ={
        {1971, 31507200},
        {1972, 63043200},
        {1973, 94665600},
        {1974, 126201600},
        {1975, 157737600},
        {1976, 189273600},
        {1977, 220896000},
        {1978, 252432000},
        {1979, 283968000},
        {1980, 315504000},
        {1981, 347126400},
        {1982, 378662400},
        {1983, 410198400},
        {1984, 441734400},
        {1985, 473356800},
        {1986, 504892800},
        {1987, 536428800},
        {1988, 567964800},
        {1989, 599587200},
        {1990, 631123200},
        {1991, 662659200},
        {1992, 694195200},
        {1993, 725817600},
        {1994, 757353600},
        {1995, 788889600},
        {1996, 820425600},
        {1997, 852048000},
        {1998, 883584000},
        {1999, 915120000},
        {2000, 946656000},
        {2001, 978278400},
        {2002, 1009814400},
        {2003, 1041350400},
        {2004, 1072886400},
        {2005, 1104508800},
        {2006, 1136044800},
        {2007, 1167580800},
        {2008, 1199116800},
        {2009, 1230739200},
        {2010, 1262275200},
        {2011, 1293811200},
        {2012, 1325347200},
        {2013, 1356969600},
        {2014, 1388505600},
        {2015, 1420041600},
        {2016, 1451577600},
        {2017, 1483200000},
        {2018, 1514736000},
        {2019, 1546272000},
        {2020, 1577808000},
        {2021, 1609430400},
        {2022, 1640966400},
        {2023, 1672502400},
        {2024, 1704038400},
        {2025, 1735660800},
        {2026, 1767196800},
        {2027, 1798732800},
        {2028, 1830268800},
        {2029, 1861891200},
        {2030, 1893427200},
        {2031, 1924963200},
        {2032, 1956499200},
        {2033, 1988121600},
        {2034, 2019657600},
        {2035, 2051193600},
        {2036, 2082729600},
        {2037, 2114352000},
        {2038, 2145888000},
        {2039, 2177424000},
        {2040, 2208960000}
    };

    uint32_t dayno = getDayOfYear(date);
    uint32_t y= date/10000;
    uint32_t h= hms/10000;
    uint32_t m=(hms/100)%100;
    uint32_t s= hms%100;
    long long ret =yearseconds.at(y)+ dayno*86400 + h*3600+ m*60 + s;
    return ret;
}

USED_API inline int32_t getWeekDay(uint32_t date){
    uint32_t y = date/10000;
    uint32_t m = (date%10000)/100;
    uint32_t d = date%100;
    if(m==1 || m==2) {  //把一月和二月换算成上一年的十三月和是四月
        m += 12;
        y--;
    }
    int32_t week = (d + 2*m + 3 * (m+1)/5 + y + y/4 - y/100 + y/400)%7;//基姆拉尔森计算公式
    return week+1;
}


USED_API static bool isTradingDay(uint32_t date){
    int32_t week = getWeekDay(date);
    //std::cout<<"isTradingDay:"<<date<<"."<<week <<"."<<g holiday.count(date)<<std::endl;
    if(week>5) return false;
    if(g_holiday.count(date)>0) return false;
    return true;
}

USED_API static uint32_t getTomorrow(uint32_t date){
    uint32_t tomorrow =0;
    uint32_t y = date/10000;
    uint32_t m = (date%10000)/100;
    uint32_t d = date%100;
    std::vector<uint32_t> monthdays = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if(isLeapYear(y)){
        monthdays[2]=29;
    }
    d++;
    if(d>monthdays[m]){
        d= 1;
        m++;
        if(m>12){
            m=1;
            y++;
        }
    }
    
    tomorrow =y*10000+ m*100+ d;
    return tomorrow;
}

USED_API static uint32_t getYesterday(uint32_t date){
    uint32_t tomorrow = 0;
    uint32_t y = date/10000;
    uint32_t m = (date%10000)/100;
    uint32_t d = date%100;
    std::vector<uint32_t> monthdays = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (isLeapYear(y)){
        monthdays[2]= 29;
    }
    if(d >1){
        tomorrow=y*10000 +m*100 +(d-1);
    }
    else if(m>1){//d=1
        tomorrow =y*10000+(m-1)*100 + monthdays[m-1];
    }
    else{ //d=1 m=1
        tomorrow=(y-1)*10000+ 1231;
    }
    return tomorrow;
}

USED_API static uint32_t getNextTradingDay(uint32_t date){
    uint32_t ret=date;
    while((ret = getTomorrow(ret))>0){
        if(isTradingDay(ret)){
            return ret;
        }
    }
    return ret;
}

USED_API static uint32_t getLastTradingDate(uint32_t date){
    uint32_t ret=date;
    while((ret = getYesterday(ret))>0){
        if(isTradingDay(ret)){
            return ret;
        }
    }
    return ret;
}

USED_API static uint32_t getNextWorkingDay(uint32_t date){
    uint32_t ret = date;
    while((ret = getTomorrow(ret))>0){
        int32_t week = getWeekDay(ret);
        if (week <= 5){
            return ret;
        }
    }
    return ret;
}


//weekday[0-6],0:sunday,yearday[0,365] start 1.1
USED_API static void getWeekInfo(uint32_t date, uint32_t &weekday, uint32_t &yearday){
    std::tm time_info = {};
    time_info.tm_year = date/10000-1900;//年份减去 1900
    time_info.tm_mon = (date%10000)/100 -1; //月份从-0-开始，所以-8-表示一月
    time_info.tm_mday = date%100;//天数
    time_info.tm_hour = 0; 
    time_info.tm_min = 0;
    time_info.tm_sec = 0;
    //将时间结构转换为时间戳
    std::time_t time_stamp = std::mktime(&time_info);
    //将时间戳转换为本地时间

    std::tm lc_time = {};
    // auto ret=gmtime r(&time_stamp, &lc_time);
    auto ret = localtime_r(&time_stamp, &lc_time);
    if(!ret){
        printf("[%s][%d],%d,gmtime_r error\n",__func__,__LINE__ ,date);
        return;
    }
    //获取该天是该年的第几周
    weekday = lc_time.tm_wday;//1-6,0 周日=0
    yearday = lc_time.tm_yday;

    return;
}


USED_API static uint32_t getWeekNumber(uint32_t date, uint32_t &year){
    uint32_t year_cur = date/10000;
    uint32_t firstday = year_cur*10000 + 101;
    uint32_t firstday_week = getWeekDay(firstday);
    //uint32_t firstday_day_no= getDayOfYear(firstday);
    //DataCommon::getWeekInfo(year_cur*10000 + 101,firstday_week, firstday_day_no);
    if(firstday_week == 0){
        firstday_week = 7;
    }
    uint32_t lastday = year_cur*10000 + 1231;
    uint32_t lastday_week = getWeekDay(lastday);
    uint32_t lastday_day_no = getDayOfYear(lastday);
    //DataCommon::getWeekInfo(year_cur*10000 + 1231,lastday_week, lastday_day_no);
    if(lastday_week ==0){
        lastday_week=7;
    }
    uint32_t date_week = getWeekDay(date);
    uint32_t date_day_no = getDayOfYear(date);
    //DataCommon::getWeekInfo(date, date week,"date day no);
    if(date_week ==0){
        date_week=7;
    }
    uint32_t wk_no = 0;
    year = year_cur;
    if(firstday_week<=4){//1.1包含周四
        wk_no=(date_day_no+(firstday_week -1))/7 + 1;
    }
    else{ //当年1.1不包含周四
        if(date_day_no<(7-firstday_week +1)){//本年开始的不足一周计入上一年
            //闰年:上一年  当年1.1
            //      周3    周5
            //      周4    周6
            //      周5    周7
            //平年: 周4    周5
            //      周5    周6
            //      周6    周7

            if(firstday_week==5){
                wk_no = 53;
            }
            else if(firstday_week == 7){
                wk_no = 52;
            }
            else if(firstday_week == 6){
                if(isLeapYear(year_cur-1)){
                    wk_no = 53;
                }
                else{
                    wk_no = 52;
                }
            }
            year = year_cur - 1;
        }
        else{
            wk_no =(date_day_no-(7-firstday_week + 1))/7 + 1;
        }
    }
    if(lastday_week <4 && date_day_no >= lastday_day_no- lastday_week + 1){ //最后不足一周的日期不包含周四，计入下一年
        wk_no = 1;
        year = year_cur + 1;
    }
    return wk_no;
}

USED_API static void GenTradingDay(uint32_t start_date, uint32_t end_date, std::vector<uint32_t> &tradedays){
    tradedays.clear();
    uint32_t date = start_date;
    // LOG_TRACE(std::to_string(start_date)+","+ std::to_string(end_date));
    std::string datesstr = "";
    while(date<= end_date){
        //LOG TRACE(std::to string(date)+","+ std::to string(end date));
        if(isTradingDay(date)){
            tradedays.emplace_back(date);
            datesstr = datesstr + std::string(" ")+ std::to_string(date);
        }
        date = getTomorrow(date);
    }
    // LOG_TRACE(datesstr);
}

USED_API static void GenTradingDay(uint32_t year,std::vector<uint32_t>&tradedays){
    uint32_t start_date= year*10000 + 101;
    uint32_t end_date= year*10000 + 1231;
    tradedays.reserve(250);
    GenTradingDay(start_date,end_date, tradedays);
}

USED_API static void GenNatureDay(uint32_t start_date, uint32_t end_date, std::vector<uint32_t> &dates){
    dates.clear();
    uint32_t date =start_date;
    // LOG_TRACE(std::to_string(start_date)+","+ std::to string(end_date));
    while(date<=end_date){
        dates.emplace_back(date);
        date = getTomorrow(date);
    }
}	


USED_API static uint32_t GetNearestTradingDay(uint32_t curdate, int32_t direction = 0){
    uint32_t date = curdate;
    if (date == 0){
        date = getsysdate2();
    }

    if (direction > 0){
        while(isTradingDay(date)==false){
            date = getTomorrow(date);
        }
    }
    else{
        while(isTradingDay(date)==false){
            date = getYesterday(date);
        }
    }
    return date;
}


