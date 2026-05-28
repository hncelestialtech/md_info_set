#include <algorithm>
#include <ctime>
#include <chrono>
#include <float.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <unordered_map>
#include <functional>
#include <cmath>
#include <sched.h>
#include <cinttypes>
#include "zt_cffex_md.h"
#include "SockProto.h"

#include "fb_md_type.h"



#include "inst_map.h"

#include "HSNsqStruct.h"




// 定义插件的导出函数
#ifdef __cplusplus
extern "C" {
#endif

void *create() {
    printf("create CFFEXExanicQuota[%s]\n");
    return new CFFEXExanicQuota();
}
void destroy(void *p) {
    printf("destroy CFFEXExanicQuota\n");
    delete (CFFEXExanicQuota*)p;
}
void get_md_api_version(char version[32]) {
    printf("[%s][%s]\n",__func__,FEBAO_MD_API_VERSION);
    strcpy(version, FEBAO_MD_API_VERSION);
}

#ifdef __cplusplus
}
#endif

std::atomic<bool> g_running = false;






inline auto MyTransU16(uint16_t a) -> uint16_t
{
    return ((a) >> 8) | ((a) << 8);
}
inline auto MyTransU32(uint32_t a) -> uint32_t
{
    return (a = (((a) >> 16) | ((a) << 16))), (((a) >> 8) & 0x00ff00ff) | (((a) << 8) & 0xff00ff00);
}

inline auto MyTransU64(uint64_t a) -> uint64_t
{
    return (a = (((a) >> 32) | ((a) << 32))), (a = ((((a) >> 16) & 0x0000ffff0000ffff) | (((a) << 16) & 0xffff0000ffff0000))), ((((a) >> 8) & 0x00ff00ff00ff00ff) | (((a) << 8) & 0xff00ff00ff00ff00));
}


int EraseSpace(std::string &str)
{
    if (str.empty())
    {
        return 0;
    }

    std::size_t start = 0;
    std::size_t end = str.size();

    while (std::isspace((int)str[start]) && str[start])
    {
        ++start;
    }
    while (end > start)
    {
        if (!std::isspace((int)str[end - 1]))
        {
            break;
        }
        --end;
    }
    str = str.substr(start, end - start);
    // 如果字符串为全空格，返回0
    return (int)(end - start);
}



int EraseSpace(char *str)
{
    if (NULL == str)
    {
        return 0;
    }

    char *start = str;
    char *end = str + strlen(str);

    while (std::isspace((int)(unsigned char)*start) && *start)
    {
        ++start;
    }
    while (end > start)
    {
        if (!std::isspace((int)(unsigned char)*(end - 1)))
        {
            break;
        }
        --end;
    }
    *end = '\0';
    memmove(str, start, end - start + 1);

    return (int)(end - start);
}



#define READ_SUB_CODES(file, name, out, market) \
    do                                          \
    {                                           \
        setSubCodes(file, #name, out, market);  \
    } while (0)
static void setSubCodes(const std::map<std::string, std::string> &user_file, const char *codes_name, MarketCodeCache &codes, const char *market)
{
    std::map<std::string, std::string>::const_iterator it = user_file.find(codes_name);
    if (it != user_file.end())
    {
        if (it->second.length() != 0)
        {
            SplitString(it->second, ",", codes[market]);
        }
        else
        {
            printf("%s is empty, using default all market subscription!\n", codes_name);
        }
    }
}


void CFFEXExanicQuota::readUserFile(UserInfo &user_info)
{
    std::ifstream fin(m_config.user_config);
    if (!fin.is_open())
    {
        printf("Failed to open userlogin file user.properties, using default username/password!\n");
        return;
    }
    std::string line;
    std::map<std::string, std::string> user_file;
    char key[128];
    char value[1024];
    while (std::getline(fin, line))
    {
        // 过滤注释行
        if (line.empty() || '#' == line[0] || ';' == line[0])
        {
            continue;
        }
        else if (2 == std::sscanf(line.c_str(), "%127[^=] = \"%1023[^\"]\"", key, value) ||
                 2 == std::sscanf(line.c_str(), "%127[^=] = '%1023[^']'", key, value) ||
                 2 == std::sscanf(line.c_str(), "%127[^=] = %1023[^#;\r]", key, value))
        {
            (void)EraseSpace(key);
            (void)EraseSpace(value);
            user_file[key] = value;
        }
    }
    fin.close();

    // 设置用户名和密码
    auto &reqLoginField = user_info.reqLoginField;
    std::map<std::string, std::string>::iterator it = user_file.find("username");
    if (it != user_file.end())
    {
        if (it->second.length() != 0 && it->second.length() < 18)
        {
            strncpy(reqLoginField.AccountID, it->second.c_str(), sizeof(reqLoginField.AccountID) - 1);
            std::clog <<__func__<<","<< __LINE__<<",AccountID:"<<reqLoginField.AccountID<< std::endl;
        }
        else
        {
            printf("UserName is too long, must be less than 18 characters!\n");
        }
    }
    it = user_file.find("password");
    if (it != user_file.end())
    {
        if (it->second.length() != 0 && it->second.length() < 16)
        {
            strncpy(reqLoginField.Password, it->second.c_str(), sizeof(reqLoginField.Password) - 1);
            std::clog <<__func__<<","<< __LINE__<<",Password:"<<reqLoginField.Password<< std::endl;
        }
        else
        {
            printf("Password is too long, must be less than 16 characters!\n");
        }
    }


    // 读取需订阅的代码

    READ_SUB_CODES(user_file, cffex_codes, user_info.market_codes, HS_EI_CFFEX);

    for(auto & it : user_info.market_codes){
        for(auto & inst : it.second){
            std::clog <<__func__<<","<< __LINE__<<",market:"<<it.first<<",inst:"<<inst<< std::endl;
        }
    }



    // 读取重建请求参数
    auto &reqTransRebuild = user_info.reqTransRebuild;
    it = user_file.find("ExchangeID");
    if (it != user_file.end())
    {
        strncpy(reqTransRebuild.ExchangeID, it->second.c_str(), sizeof(reqTransRebuild.ExchangeID));
    }
    it = user_file.find("ChannelNo");
    if (it != user_file.end())
    {
        reqTransRebuild.ChannelNo = atoi(it->second.c_str());
    }
    it = user_file.find("BeginSeqNo");
    if (it != user_file.end())
    {
        reqTransRebuild.BeginSeqNo = atoll(it->second.c_str());
    }
    it = user_file.find("EndSeqNo");
    if (it != user_file.end())
    {
        reqTransRebuild.EndSeqNo = atoll(it->second.c_str());
    }
    printf("Load username[%s] from user.properties file successfully!\n", reqLoginField.AccountID);


    printf("username: %s\n", reqLoginField.AccountID);
    printf("password: %s\n", reqLoginField.Password);
    printf("ExchangeID: %s\n", reqTransRebuild.ExchangeID);
    printf("ChannelNo: %d\n", reqTransRebuild.ChannelNo);
    printf("BeginSeqNo: %ld\n", reqTransRebuild.BeginSeqNo);
    printf("EndSeqNo: %ld\n", reqTransRebuild.EndSeqNo);
    // printf("RebuildType: %c\n", reqTransRebuild.RebuildType);

}





void CFFEXExanicQuota::readSDKConfig(UserInfo &user_info)
{
    std::ifstream file(m_config.sdk_config);

    if (!file)
    {
        std::cerr << "Failed to open the file." << std::endl;
        return;
    }

    std::string line;
    std::string targetString = "support_markets";
    bool found = false;
    std::string Hqdfline;
    std::string HqdfTargetString = "rcgw_markets";
    bool HqdfFound = false;

    std::string TempLine;
    while (std::getline(file, TempLine)){
        // 去除行首空格
        TempLine.erase(TempLine.begin(), std::find_if(TempLine.begin(), TempLine.end(), [](int ch){ return !std::isspace(ch); }));
        // 查找support_markets配置项所在行
        if (0 == TempLine.find(targetString)){
            found = true;
            line = TempLine;
        }
        // 查找rcgw_markets配置项所在行
        if (0 == TempLine.find(HqdfTargetString)){
            HqdfFound = true;
            Hqdfline = TempLine;
        }
    }

    // 解析support_markets配置项所在行
    char key[128];
    char value[1024];
    if (found && 2 == std::sscanf(line.c_str(), "%127[^=] = %1023[^#;\r]", key, value))
    {
        std::vector<std::string> markets;
        SplitString(value, ",", markets);
        for (auto &val : markets)
        {
            (void)EraseSpace(val);
            if (MARET_TYPE_CONVERT.find(val) == MARET_TYPE_CONVERT.cend()){
                continue;                
            }
            user_info.sdk_config_market.insert(MARET_TYPE_CONVERT.at(val));
            std::clog <<__func__<<","<< __LINE__<<",market:"<<MARET_TYPE_CONVERT.at(val)<< std::endl;
        }
    }
    // support_markets没有配置市场，则解析rcgw_markets配置项所在行
    if (user_info.sdk_config_market.empty() && HqdfFound && 2 == std::sscanf(Hqdfline.c_str(), "%127[^=] = %1023[^#;\r]", key, value))
    {
        std::vector<std::string> markets;
        SplitString(value, ",", markets);
        for (auto &val : markets)
        {
            (void)EraseSpace(val);
            if (MARET_TYPE_CONVERT.find(val) == MARET_TYPE_CONVERT.cend()){
                continue;                
            }
            user_info.sdk_config_market.insert(MARET_TYPE_CONVERT.at(val));
            std::clog <<__func__<<","<< __LINE__<<",market:"<<MARET_TYPE_CONVERT.at(val)<< std::endl;
        }
    }
}

// 期货市场码表请求，包括上期所、上期能源、大商所、郑商所、中金所
void CFFEXExanicQuota::ReqAllFutInstruments(const char *market, int len, int &nRequestID)
{
    CHSNsqReqFutuDepthMarketDataField futuReqField;
    memcpy(futuReqField.ExchangeID, market, len);
    int ret = m_NsqApi->ReqQryFutuInstruments(&futuReqField, 0, nRequestID);
    std::clog <<__func__<<","<< __LINE__<<",ReqQryFutuInstruments market:"<<market<<",nRequestID:"<<nRequestID<<",ret:"<<ret<< std::endl;
    if (0 != ret){            // 参数错误、无权限等因素，会导致请求失败
        std::clog <<__func__<<","<< __LINE__<<",ReqQryFutuInstruments failed nRequestID:"<<nRequestID<<",ret:"<<ret<<",ErrorMsg:"<<m_NsqApi->GetApiErrorMsg(ret)<< std::endl;
    }
    nRequestID++;
    
}


void CFFEXExanicQuota::ReqFutInstruments(const char *market, int len, int &nRequestID)
{
    int nCount = 0;
    CHSNsqReqFutuDepthMarketDataField reqFutu_sub[1000];
    for (auto inst : m_subscribe_insts) {
        memcpy(reqFutu_sub[nCount].ExchangeID, market, len);
        memcpy(reqFutu_sub[nCount].InstrumentID, inst.c_str(), inst.length() + 1);
        nCount++;
        std::clog <<__func__<<","<< __LINE__<<",nCount:"<<nCount<<", inst:"<<inst<< std::endl;
    }
    int ret = m_NsqApi->ReqQryFutuInstruments(reqFutu_sub, nCount, nRequestID);
    std::clog <<__func__<<","<< __LINE__<<",ReqQryFutuInstruments market:"<<market<<",nRequestID:"<<nRequestID<<",ret:"<<ret<< std::endl;
    if (0 != ret){            // 参数错误、无权限等因素，会导致请求失败
        std::clog <<__func__<<","<< __LINE__<<",ReqQryFutuInstruments failed nRequestID:"<<nRequestID<<",ret:"<<ret<<",ErrorMsg:"<<m_NsqApi->GetApiErrorMsg(ret)<< std::endl;
    }
    nRequestID++;   
}


void CFFEXExanicQuota::FutSubscribe(const char *market, int len, int &nRequestID)
{
    int nCount = 0;
    CHSNsqReqFutuDepthMarketDataField reqFutu_sub[1000];
    for (auto inst : m_subscribe_insts) {
        memcpy(reqFutu_sub[nCount].ExchangeID, market, len);
        memcpy(reqFutu_sub[nCount].InstrumentID, inst.c_str(), inst.length() + 1);
        nCount++;
        std::clog <<__func__<<","<< __LINE__<<",nCount:"<<nCount<<", inst:"<<inst<< std::endl;
    }

    int ret = m_NsqApi->ReqFutuDepthMarketDataSubscribe(reqFutu_sub, nCount, nRequestID);
    if (0 != ret){            // 参数错误、无权限等因素，会导致请求失败
        std::clog <<__func__<<","<< __LINE__<<",ReqFutuDepthMarketDataSubscribe failed nRequestID:"<<nRequestID<<",ret:"<<ret<<",ErrorMsg:"<<m_NsqApi->GetApiErrorMsg(ret)<< std::endl;
    }
    std::clog <<__func__<<","<< __LINE__<<",ReqFutuDepthMarketDataSubscribe nRequestID:"<<nRequestID<<",ret:"<<ret<<",ErrorMsg:"<<m_NsqApi->GetApiErrorMsg(ret)<< std::endl;
    nRequestID++;
}

void CFFEXExanicQuota::Quota_CallBack(void * ctx, CHSNsqFutuDepthMarketDataField *quota){
    CFFEXExanicQuota *pthis = (CFFEXExanicQuota*)ctx;
    pthis->Quota_Recv(quota);
}

void CFFEXExanicQuota::StaticInfo_CallBack(void * ctx, CHSNsqFutuInstrumentStaticInfoField *info, bool bIsLast){
    CFFEXExanicQuota *pthis = (CFFEXExanicQuota*)ctx;
    pthis->StaticInfo_Recv(info, bIsLast);
}

CFFEXExanicQuota::CFFEXExanicQuota(): 
    m_fb_initialized(false),
    m_fb_spi(nullptr),
    m_fb_md(cffex::fb::api::market_data_entity::create_entity())
{
    std::clog << std::unitbuf; //调试用

    // std::string testfile = "/home/febao/shenzx/test.csv";  // TODO stub
    // m_quota_stub.Load(testfile);   // TODO stub
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;

}


CFFEXExanicQuota::~CFFEXExanicQuota(){
    std::clog << std::nounitbuf;
}

void CFFEXExanicQuota::register_spi(cffex::fb::api::fb_i_md_spi *spi){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    m_fb_spi = spi;
}


inline void CFFEXExanicQuota::LoadJsonCfg(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    cffex::fb::api::fb_md_config_helper *parser = m_fb_spi->get_config_helper();
    std::string md_jsoncfgfile;
    parser->get_attribute("file", "/md_config_path", md_jsoncfgfile);

    // cfg
    std::ifstream jsonfile(md_jsoncfgfile);
    nlohmann::json json_parser = nlohmann::json::parse(jsonfile);

    auto& eth = json_parser["nsq"];
    m_config.sdk_config = eth["sdk_config"];
    m_config.user_config = eth["user_config"];

    std::clog <<__func__<<","<< __LINE__<<",sdk_config[" <<m_config.sdk_config<<"]"<< std::endl;
    std::clog <<__func__<<","<< __LINE__<<",user_config[" <<m_config.user_config<<"]"<< std::endl;

    auto& worker = json_parser["worker"];
    m_config.cpu_id = worker["cpuid"].get<uint32_t>();

    auto& filter = json_parser["filter"];
    m_config.filter_path = filter["filter_instrument_info"];
    std::clog <<__func__<<","<< __LINE__<<",filter_path[" <<m_config.filter_path<<"]"<< std::endl;
    std::unordered_map<std::string, std::string> underlying_inst_map;
    LoadFebaoInstrumentInfo(m_config.filter_path, underlying_inst_map);

    for (const auto& item : filter["underlying"]) {
        m_subscribe_insts.emplace(item.get<std::string>());
        std::clog <<__func__<<","<< __LINE__<<",underlying[" <<item.get<std::string>()<<"]"<< std::endl;               
    }

    for(auto &itor : underlying_inst_map){
        m_subscribe_insts.emplace(itor.first);
        m_subscribe_insts.emplace(itor.second);
    }

    for (auto inst : m_subscribe_insts) {
        m_quota_cache.InitOnce(inst);
        std::clog <<__func__<<","<< __LINE__<<",inst[" <<inst.c_str()<<"]"<< std::endl;
    }
    m_quota_cache.Sort();
    std::clog <<__func__<<","<< __LINE__<<",read_cpu[" <<m_config.cpu_id<<"]"<< std::endl;

}

void on_exit(int sig){
    g_running.store(false, std::memory_order_release);
}


int CFFEXExanicQuota::init(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;

    LoadJsonCfg();

    m_NsqApi = NewNsqApiExt("./log/", m_config.sdk_config.c_str());
    if (NULL == m_NsqApi){
        std::clog <<__func__<<","<< __LINE__<<",ERROR: NewNsqApiExt failed!"<< std::endl;
        return -1;
    }

    std::clog <<__func__<<","<< __LINE__<<",NewNsqApiExt success"<< std::endl;

    readSDKConfig(m_user_info);
    readUserFile(m_user_info);

    m_NsqSpi = new CHSNsqSpiImpl(m_NsqApi);

    m_NsqSpi->SetUserInfo(&m_user_info);
    m_NsqSpi->SetFutQuotaCallBack(this, Quota_CallBack, StaticInfo_CallBack);

    m_NsqApi->RegisterSpi(m_NsqSpi);

    (void)m_NsqApi->RegisterFront("");
    int ret = m_NsqApi->Init("");
    if (ret != 0){
        std::clog <<__func__<<","<< __LINE__<<",ERROR,ret:"<<ret<<",ErrorMsg:"<<m_NsqApi->GetApiErrorMsg(ret)<< std::endl;
        m_NsqApi->ReleaseApi();
        return -1;
    }

    sleep(3);
    while (!m_NsqSpi->GetConnectStatus()){
        std::clog <<__func__<<","<< __LINE__<<",ERROR: Connect to sailfish_service or HQReceiverGW failed, please check the ip/port or waiting to switch address!"<< std::endl;
        sleep(3);
    }

    if (!m_NsqSpi->GetConnectStatus()){
        std::clog <<__func__<<","<< __LINE__<<",ERROR: Connect failed, error exit!"<< std::endl;
        m_NsqApi->ReleaseApi();
        return -1;
    }

    int iRetries = 3000; /* 等待登录成功，等待30秒超时 */
    while (!m_NsqSpi->GetLoginStatus() && (iRetries--) > 0){
        usleep(10000);
    }
    // 当登录出现错误时，程序会打印出错误原因，之后直接退出。
    if (!m_NsqSpi->GetLoginStatus()){
        std::clog <<__func__<<","<< __LINE__<<",ErrorMsg:"<<m_NsqApi->GetApiErrorMsg(0)<< std::endl;
        m_NsqApi->ReleaseApi();
        return -1;
    }

    sleep(2);

    int nRequestID = 0;
    ReqAllFutInstruments(HS_EI_CFFEX, sizeof(HS_EI_CFFEX), nRequestID); // 中金所

    iRetries = 500; /* 等待所有代码就绪，等待5秒超时 */
    /* m_isAllInstrReady 在 CHSNsqSpiImpl::OnRspQrySecuAllInstruments 中收到最后一个代码置成true */
    while (!m_NsqSpi->GetInstrumentsStatus() && (iRetries--) > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (!m_NsqSpi->GetInstrumentsStatus())
    {
        printf("WARNING: All Instruments are not ready after ReqQrySecuInstruments!\n");
        // m_NsqApi->ReleaseApi();
        // return -1; /// NOTE: 请根据自身业务逻辑判断是否要退出
    }
    else
    {
        printf("All Instruments are ready, count %d!\n", m_NsqSpi->GetInstrumentsCount());
    }

    FutSubscribe(HS_EI_CFFEX, sizeof(HS_EI_CFFEX), nRequestID); // 中金所
    sleep(3);

    // m_worker = new std::thread(&CFFEXExanicQuota::Routine, this);   //和febao沟通，由于 on_msg的不可重入性，暂时只能有一个线程
    // m_exanic_initialized.store(true,std::memory_order_release);
    // m_fb_initialized.store(true,std::memory_order_release);
    m_fb_spi->on_ready();
    std::clog <<__func__<<","<< __LINE__<<",success" << std::endl;
    return 0;
}

void CFFEXExanicQuota::release(){
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    g_running.store(false,std::memory_order_release);
    if(m_worker->joinable()){
        m_worker->join();
    }
    delete m_worker;
    m_worker = nullptr;

    m_NsqApi->ReleaseApi();

    delete m_NsqSpi;


}

void CFFEXExanicQuota::connect(){
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
    g_running.store(true,std::memory_order_release);
}

void CFFEXExanicQuota::subscribe_inst(const std::string &instrument_id, uint8_t exchange_id){
    std::clog <<__func__<<","<< __LINE__<< ","<<instrument_id<<","<< exchange_id<< std::endl;
}

void CFFEXExanicQuota::StaticInfo_Recv(CHSNsqFutuInstrumentStaticInfoField *info, bool bIsLast){

    QuotaInfo *cache = m_quota_cache.Find(info->InstrumentID);
    if (cache == nullptr){
        return;
    }

    




}



void CFFEXExanicQuota::Quota_Recv(CHSNsqFutuDepthMarketDataField *msgdata){
    
    // printf("msgdata: ExchangeID %s, InstrumentID %s, TradeDate %d, UpdateTime %d, LastPrice %lf, "
    //         "OpenPrice %lf, HighPrice %lf, LowPrice %lf, TradeVolume %" PRId64 ", TradeBalance %lf, PreSettlementPrice %lf,"
    //         "PreClosePrice %lf, OpenInterest %" PRId64 ", ClosePrice %lf, SettlementPrice %lf, UpLimitPx %lf, DownLimitPx %lf, "
    //         "AveragePrice %lf, PreOpenInterest %" PRId64 ", \n "
    //         "\tBid1Price %lf, Bid1Volume %" PRId64 "\n"
    //         "\tAsk1Price %lf, Ask1Volume %" PRId64 "\n",
    //         msgdata->ExchangeID,
    //         msgdata->InstrumentID,
    //         msgdata->TradingDay,
    //         msgdata->UpdateTime,
    //         msgdata->LastPrice,
    //         msgdata->OpenPrice,
    //         msgdata->HighestPrice,
    //         msgdata->LowestPrice,
    //         msgdata->TradeVolume,
    //         msgdata->TradeBalance,
    //         msgdata->PreSettlementPrice,
    //         msgdata->PreClosePrice,
    //         msgdata->OpenInterest,
    //         msgdata->ClosePrice,
    //         msgdata->SettlementPrice,
    //         msgdata->UpperLimitPrice,
    //         msgdata->LowerLimitPrice,
    //         msgdata->AveragePrice,
    //         msgdata->PreOpenInterest,
    //         msgdata->BidPrice[0], msgdata->BidVolume[0],
    //         msgdata->AskPrice[0], msgdata->AskVolume[0]);
    int64_t local_time_ns = get_nanoseconds();
    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_DEBUG, "recv [%d][%llu],inst:%s,extime[%d],[%.2f],vol[%d][%.2f][%llu]\n",__LINE__,local_time_ns,
    //         msgdata->InstrumentID,msgdata->UpdateTime,msgdata->LastPrice,msgdata->TradeVolume,msgdata->TradeBalance,msgdata->OpenInterest);

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "recv [%d],inst[%s],extime[%d],[%.2f],vol[%d][%.2f][%.2f],bp[%.2f][%.2f][%.2f][%.2f][%.2f],bv[%llu][%llu][%llu][%llu][%llu],ap[%.2f][%.2f][%.2f][%.2f][%.2f],av[%llu][%llu][%llu][%llu][%llu]\n",__LINE__,
    //         msgdata->InstrumentID,msgdata->UpdateTime,msgdata->LastPrice,msgdata->TradeVolume,msgdata->TradeBalance,msgdata->OpenInterest,
    //         msgdata->BidPrice1,msgdata->BidPrice2,msgdata->BidPrice3,msgdata->BidPrice4,msgdata->BidPrice5,
    //         msgdata->BidVolume1,msgdata->BidVolume2,msgdata->BidVolume3,msgdata->BidVolume4,msgdata->BidVolume5,
    //         msgdata->AskPrice1,msgdata->AskPrice2,msgdata->AskPrice3,msgdata->AskPrice4,msgdata->AskPrice5,
    //         msgdata->AskVolume1,msgdata->AskVolume2,msgdata->AskVolume3,msgdata->AskVolume4,msgdata->AskVolume5);



    //130532900
    uint64_t hhmm = msgdata->UpdateTime/100000L;
    uint64_t extime_s = (hhmm/100L)*3600 + (hhmm%100L)*60L + msgdata->UpdateTime%100000L/1000L;

    m_fb_md->reset_entity();
    m_fb_md->set_local_timestamp(local_time_ns); //utc ns
    m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
    m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_CFFEX);
    m_fb_md->set_update_sec(extime_s);
    m_fb_md->set_update_msec(msgdata->UpdateTime%1000L);
    m_fb_md->set_max_depth(5);                              // 设置行情深度5
    m_fb_md->set_instrument_id(msgdata->InstrumentID);
    m_fb_md->set_pre_settlement(msgdata->PreSettlementPrice);
    m_fb_md->set_pre_close(msgdata->PreClosePrice);
    m_fb_md->set_pre_open_interest(msgdata->PreOpenInterest);
    m_fb_md->set_upper_limit_price(convert_price_if_zero(msgdata->UpperLimitPrice));
    m_fb_md->set_down_limit_price(convert_price_if_zero(msgdata->LowerLimitPrice));
    m_fb_md->set_open_interest(msgdata->OpenInterest);
    m_fb_md->set_open(convert_price_if_zero(msgdata->OpenPrice));
    m_fb_md->set_close(convert_price_if_zero(msgdata->ClosePrice));
    m_fb_md->set_high_price(convert_price_if_zero(msgdata->HighestPrice));
    m_fb_md->set_low_price(convert_price_if_zero(msgdata->LowestPrice));
    m_fb_md->set_last_price(convert_price_if_zero(msgdata->LastPrice));
    m_fb_md->set_volume(msgdata->TradeVolume);
    m_fb_md->set_turn_over(msgdata->TradeBalance);
    m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata->AskPrice[0],msgdata->AskVolume[0]));
    m_fb_md->set_ask2_price(convert_price_if_volume_zero(msgdata->AskPrice[1],msgdata->AskVolume[1]));
    m_fb_md->set_ask3_price(convert_price_if_volume_zero(msgdata->AskPrice[2],msgdata->AskVolume[2]));
    m_fb_md->set_ask4_price(convert_price_if_volume_zero(msgdata->AskPrice[3],msgdata->AskVolume[3]));
    m_fb_md->set_ask5_price(convert_price_if_volume_zero(msgdata->AskPrice[4],msgdata->AskVolume[4]));
    m_fb_md->set_ask1_volume(msgdata->AskVolume[0]);
    m_fb_md->set_ask2_volume(msgdata->AskVolume[1]);
    m_fb_md->set_ask3_volume(msgdata->AskVolume[2]);
    m_fb_md->set_ask4_volume(msgdata->AskVolume[3]);
    m_fb_md->set_ask5_volume(msgdata->AskVolume[4]);
    m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata->BidPrice[0],msgdata->BidVolume[0]));
    m_fb_md->set_bid2_price(convert_price_if_volume_zero(msgdata->BidPrice[1],msgdata->BidVolume[1]));
    m_fb_md->set_bid3_price(convert_price_if_volume_zero(msgdata->BidPrice[2],msgdata->BidVolume[2]));
    m_fb_md->set_bid4_price(convert_price_if_volume_zero(msgdata->BidPrice[3],msgdata->BidVolume[3]));
    m_fb_md->set_bid5_price(convert_price_if_volume_zero(msgdata->BidPrice[4],msgdata->BidVolume[4]));
    m_fb_md->set_bid1_volume(msgdata->BidVolume[0]);
    m_fb_md->set_bid2_volume(msgdata->BidVolume[1]);
    m_fb_md->set_bid3_volume(msgdata->BidVolume[2]);
    m_fb_md->set_bid4_volume(msgdata->BidVolume[3]);
    m_fb_md->set_bid5_volume(msgdata->BidVolume[4]);
    m_fb_md->set_iopv(20000);


    int64_t local_time_ns_0 = get_nanoseconds();

    m_fb_spi->on_msg(m_fb_md);
    int64_t local_time_ns_end = get_nanoseconds();

    static uint64_t delay_sum = 0;
    static uint64_t delay_sum_0 = 0; //  -0
    static uint64_t delay_sum_1 = 0; // 0-end

    static uint64_t count_stats = 0;
    delay_sum += (local_time_ns_end - local_time_ns);
    delay_sum_0 += (local_time_ns_0 - local_time_ns);
    delay_sum_1 += (local_time_ns_end - local_time_ns_0);

    static uint64_t  period = 500;
    count_stats ++;
    if (count_stats%period == 0){
        int cpu = sched_getcpu();
        m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"CPU[%d],per[%llu], ave allcost[%llu], section fill[%llu],send[%llu]\n",
        cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period);
        delay_sum   = 0;        
        delay_sum_0 = 0;
        delay_sum_1 = 0;
    }



}




