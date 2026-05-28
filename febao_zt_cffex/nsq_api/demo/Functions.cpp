#include "Functions.h"
#include "InnerData.h"
#include "HSNsqApi.h"
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <map>
#include <cctype>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <signal.h>
#include <memory>
#include <string.h>
#include "DataWriter.h"
#include "HSNsqSpiImpl.h"
#include "HSDataType.h"

#ifdef _WIN32
#define SIGQUIT SIGBREAK
#endif // _WIN32

extern volatile int keepRunning;
extern volatile int g_quit;
extern UserInfo g_user_info;

// 删除空格
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

// 按指定分隔符c分离字符串s并保存到v中
void SplitString(const std::string &s, std::vector<std::string> &v, const std::string &c)
{
#if 0
	size_t n = s.find_last_not_of("\r\n");
	if(n != std::string::npos)
	{
		s.erase(n+1, s.size() -n);
	}
	n = s.find_first_not_of("\r\n");
	if(n != std::string::npos)
	{
		s.erase(0, n);
	}
#endif
    size_t pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    std::string tmp;
    while (std::string::npos != pos2)
    {
        tmp = s.substr(pos1, pos2 - pos1);
        if (0 != EraseSpace(tmp))
            v.push_back(tmp);
        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if (s.length() > pos1)
    {
        tmp = s.substr(pos1);
        if (0 != EraseSpace(tmp))
            v.push_back(tmp);
    }
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
            SplitString(it->second, codes[market], ",");
        }
        else
        {
            printf("%s is empty, using default all market subscription!\n", codes_name);
        }
    }
}

void readUserFile(UserInfo &user_info)
{
    std::ifstream fin("user.properties");
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
        }
        else
        {
            printf("Password is too long, must be less than 16 characters!\n");
        }
    }
    // 读取行情输出位置
    it = user_file.find("output_dest");
    if (it != user_file.end() && 1 == it->second.size() && it->second[0] >= '0' && it->second[0] <= '2')
    {
        user_info.output_data = (OutPutData)(it->second[0] - '0');
    }
    else
    {
        user_info.output_data = OutPutData::SCREEN;
    }
    // 32位系统只支持数据输出到屏幕
    if (sizeof(void *) == 4)
    {
        user_info.output_data = OutPutData::SCREEN;
    }

    // 读取需订阅的代码
    READ_SUB_CODES(user_file, sh_codes, user_info.market_codes, HS_EI_SSE);
    READ_SUB_CODES(user_file, sz_codes, user_info.market_codes, HS_EI_SZSE);
    READ_SUB_CODES(user_file, shhkt_codes, user_info.market_codes, HS_EI_SSEHK);
    READ_SUB_CODES(user_file, szhkt_codes, user_info.market_codes, HS_EI_SZSEHK);
    READ_SUB_CODES(user_file, neeq_codes, user_info.market_codes, HS_EI_TZASE);
    READ_SUB_CODES(user_file, swi_codes, user_info.market_codes, HS_EI_SWI);
    READ_SUB_CODES(user_file, bj_codes, user_info.market_codes, HS_EI_BJSE);
    READ_SUB_CODES(user_file, shfe_codes, user_info.market_codes, HS_EI_SHFE);
    READ_SUB_CODES(user_file, ine_codes, user_info.market_codes, HS_EI_INE);
    READ_SUB_CODES(user_file, cffex_codes, user_info.market_codes, HS_EI_CFFEX);
    READ_SUB_CODES(user_file, czce_codes, user_info.market_codes, HS_EI_CZCE);
    READ_SUB_CODES(user_file, dce_codes, user_info.market_codes, HS_EI_DCE);

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

#ifdef DEBUG
    printf("username: %s\n", reqLoginField.AccountID);
    printf("password: %s\n", reqLoginField.Password);
    printf("sh_codes:");
    for (size_t i = 0; i < user_info.sh_codes.size(); ++i)
        printf("%s ", user_info.sh_codes[i].c_str());
    printf("\nsz_codes:");
    for (size_t i = 0; i < user_info.sz_codes.size(); ++i)
        printf("%s ", user_info.sz_codes[i].c_str());
    printf("\nshhkt_codes:");
    for (size_t i = 0; i < user_info.shhkt_codes.size(); ++i)
        printf("%s ", user_info.shhkt_codes[i].c_str());
    printf("\nszhkt_codes:");
    for (size_t i = 0; i < user_info.szhkt_codes.size(); ++i)
        printf("%s ", user_info.szhkt_codes[i].c_str());
    printf("\nneeq_codes:");
    for (size_t i = 0; i < user_info.neeq_codes.size(); ++i)
        printf("%s ", user_info.neeq_codes[i].c_str());
    // printf("\nfut_codes:");
    // for (size_t i = 0; i < user_info.nfut_codes.size(); ++i)
    // 	printf("%s ", user_info.nfut_codes[i].c_str());
    printf("\n");
    printf("ExchangeID: %s\n", reqTransRebuild.ExchangeID);
    printf("ChannelNo: %d\n", reqTransRebuild.ChannelNo);
    printf("BeginSeqNo: %ld\n", reqTransRebuild.BeginSeqNo);
    printf("EndSeqNo: %ld\n", reqTransRebuild.EndSeqNo);
    // printf("RebuildType: %c\n", reqTransRebuild.RebuildType);
#endif
}

void sigHandler(int sig)
{
    if (sig == SIGINT || sig == SIGABRT || sig == SIGTERM || sig == SIGQUIT)
    {
        keepRunning = 0;
        // printf("catch singal_num[%d],press any key to quit!\n", sig);
        printf("catch singal_num[%d], begin to quit...\n", sig);
    }
}

void checkFunctionRet(CHSNsqApi *lpNsqApi, int iRet, int nRequestID)
{
    if (0 != iRet)
    {
        // 参数错误、无权限等因素，会导致请求失败
        printf("nRequestID[%d] iRet[%d] error:%s\n", nRequestID, iRet, lpNsqApi->GetApiErrorMsg(iRet));
    }
}

// 写文件
void thr_write_fn(CHSNsqSpiImpl *lpNsqSpi)
{
    printf("Write thread start\n");
    while (!g_quit)
    {
        lpNsqSpi->WriteData();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void readFutCode(CHSNsqReqFutuDepthMarketDataField *reqFutu_sub, int &nCount, std::vector<std::string> &fut_code, const char *market)
{
    nCount = 0;
    strncpy(reqFutu_sub[0].ExchangeID, market, strlen(market) + 1);
    for (size_t i = 0; i < fut_code.size() && i < 1000; i++)
    {
        strncpy(reqFutu_sub[nCount].ExchangeID, market, strlen(market) + 1);
        strncpy(reqFutu_sub[nCount++].InstrumentID, fut_code[i].c_str(), fut_code[i].length() + 1);
    }
}

void readSDKConfig(UserInfo &user_info, std::string &sdk_cfg_path)
{
    std::ifstream file(sdk_cfg_path);

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

    while (std::getline(file, TempLine))
    {
        // 去除行首空格
        TempLine.erase(TempLine.begin(), std::find_if(TempLine.begin(), TempLine.end(), [](int ch)
                                                      { return !std::isspace(ch); }));
        // 查找support_markets配置项所在行
        if (0 == TempLine.find(targetString))
        {
            found = true;
            line = TempLine;
        }
        // 查找rcgw_markets配置项所在行
        if (0 == TempLine.find(HqdfTargetString))
        {
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
        SplitString(value, markets, ",");
        for (auto &val : markets)
        {
            (void)EraseSpace(val);
            if (MARET_TYPE_CONVERT.find(val) == MARET_TYPE_CONVERT.cend())
                continue;
            user_info.sdk_config_market.insert(MARET_TYPE_CONVERT.at(val));
        }
    }
    // support_markets没有配置市场，则解析rcgw_markets配置项所在行
    if (user_info.sdk_config_market.empty() && HqdfFound && 2 == std::sscanf(Hqdfline.c_str(), "%127[^=] = %1023[^#;\r]", key, value))
    {
        std::vector<std::string> markets;
        SplitString(value, markets, ",");
        for (auto &val : markets)
        {
            (void)EraseSpace(val);
            if (MARET_TYPE_CONVERT.find(val) == MARET_TYPE_CONVERT.cend())
                continue;
            user_info.sdk_config_market.insert(MARET_TYPE_CONVERT.at(val));
        }
    }
}

// 现货市场码表请求，包括沪深交易所、北交所、股转、申万指数
void ReqStockInstruments(CHSNsqApi *lpNsqApi, const char *market, int len, int &nRequestID)
{
    if (!g_user_info.sdk_config_market.count(market))
        return;

    // 现货码表请求
    CHSNsqReqSecuDepthMarketDataField reqField;
    memcpy(reqField.ExchangeID, market, len);
    int iRet = lpNsqApi->ReqQrySecuInstruments(&reqField, 0, nRequestID); // count为0表示请求全市场合约
    printf("ReqQrySecuInstruments ExChangeID[%s] nRequestID[%d] iRet[%d]\n", market, nRequestID, iRet);
    checkFunctionRet(lpNsqApi, iRet, nRequestID++);

    // 非沪深不需要请求期权、港股通码表
    if (HS_EI_SZSE[0] != market[0] && HS_EI_SSE[0] != market[0])
        return;

    // 沪深期权码表请求
    CHSNsqReqOptDepthMarketDataField optReqField;
    memcpy(optReqField.ExchangeID, market, len);
    iRet = lpNsqApi->ReqQryOptInstruments(&optReqField, 0, nRequestID);
    printf("ReqQryOptInstruments ExChangeID[%s] nRequestID[%d] iRet[%d]\n", market, nRequestID, iRet);
    checkFunctionRet(lpNsqApi, iRet, nRequestID++);
    // 沪深港股通码表请求
    CHSNsqReqHktDepthMarketDataField hktReqField;
    memcpy(hktReqField.ExchangeID, HS_EI_SSE[0] == market[0] ? HS_EI_SSEHK : HS_EI_SZSEHK, sizeof(HS_EI_SSEHK));
    iRet = lpNsqApi->ReqQryHktInstruments(&hktReqField, 0, nRequestID);
    printf("ReqQryHktInstruments ExChangeID[%s] nRequestID[%d] iRet[%d]\n", market, nRequestID, iRet);
    checkFunctionRet(lpNsqApi, iRet, nRequestID++);
}

// 期货市场码表请求，包括上期所、上期能源、大商所、郑商所、中金所
void ReqFutInstruments(CHSNsqApi *lpNsqApi, const char *market, int len, int &nRequestID)
{
    if (g_user_info.sdk_config_market.count(market))
    {
        CHSNsqReqFutuDepthMarketDataField futuReqField;
        memcpy(futuReqField.ExchangeID, market, len);
        int iRet = lpNsqApi->ReqQryFutuInstruments(&futuReqField, 0, nRequestID);
        printf("ReqQryFutuInstruments ExChangeID[%s] nRequestID[%d] iRet[%d]\n", market, nRequestID, iRet);
        checkFunctionRet(lpNsqApi, iRet, nRequestID++);
    }
}

// 现货市场订阅，包括沪深交易所、北交所、股转、申万指数
void StockSubscribe(CHSNsqApi *lpNsqApi, const char *market, int len, int &nRequestID)
{
    // 配置文件未配置则不订阅
    if (!g_user_info.sdk_config_market.count(market))
        return;

    int nCount = 0, iRet;
    CHSNsqReqSecuDepthMarketDataField reqField_sub[1000];
    memcpy(reqField_sub[0].ExchangeID, market, len); // nCount=0时，全市场订阅
    if (g_user_info.market_codes.count(market))
    {
        auto &_codes = g_user_info.market_codes[market];
        for (size_t i = 0; i < _codes.size() && i < 1000; i++)
        {
            if (_codes[i].length() != 6)
                continue;
            memcpy(reqField_sub[nCount].ExchangeID, market, len);
            memcpy(reqField_sub[nCount++].InstrumentID, _codes[i].c_str(), _codes[i].size() + 1);
        }
    }
    /* 订阅快照行情，nCount为0表示全市场订阅，否则进行代码池订阅 */
    iRet = lpNsqApi->ReqSecuDepthMarketDataSubscribe(reqField_sub, nCount, nRequestID);
    checkFunctionRet(lpNsqApi, iRet, nRequestID++);

    // 非沪深市场不需要订阅plus、逐笔、期权、港股通
    if (HS_EI_SZSE[0] != market[0] && HS_EI_SSE[0] != market[0])
        return;

    /* 订阅快照Plus行情，count为0表示全市场订阅 */
    iRet = lpNsqApi->ReqSecuDepthMarketDataPlusSubscribe(reqField_sub, nCount, nRequestID);
    checkFunctionRet(lpNsqApi, iRet, nRequestID++);
    /* 订阅逐笔成交行情，nCount为0表示全市场订阅，否则进行代码池订阅 */
    iRet = lpNsqApi->ReqSecuTransactionSubscribe(HS_TRANS_Trade, reqField_sub, nCount, nRequestID);
    checkFunctionRet(lpNsqApi, iRet, nRequestID++);
    /* 订阅逐笔委托，nCount为0表示全市场订阅，否则进行代码池订阅 */
    iRet = lpNsqApi->ReqSecuTransactionSubscribe(HS_TRANS_Entrust, reqField_sub, nCount, nRequestID);
    checkFunctionRet(lpNsqApi, iRet, nRequestID++);
    /* 订阅期权快照行情，nCount为0表示全市场订阅，否则进行代码池订阅 */
    CHSNsqReqOptDepthMarketDataField optReqField;
    memcpy(optReqField.ExchangeID, market, len);
    iRet = lpNsqApi->ReqOptDepthMarketDataSubscribe(&optReqField, 0, nRequestID);
    checkFunctionRet(lpNsqApi, iRet, nRequestID++);
    /// 港股通行情订阅
    nCount = 0;
    memcpy(reqField_sub[0].ExchangeID, HS_EI_SSE[0] == market[0] ? HS_EI_SSEHK : HS_EI_SZSEHK, sizeof(HS_EI_SSEHK)); // nCount=0时，全市场订阅
    auto &hkt_codes = g_user_info.market_codes[HS_EI_SSE[0] == market[0] ? HS_EI_SSEHK : HS_EI_SZSEHK];
    for (size_t i = 0; i < hkt_codes.size() && i < 1000; i++)
    {
        if (hkt_codes[i].length() != 5)
            continue;
        memcpy(reqField_sub[nCount].ExchangeID, market[0] ? HS_EI_SSEHK : HS_EI_SZSEHK, sizeof(HS_EI_SSEHK));
        memcpy(reqField_sub[nCount++].InstrumentID, hkt_codes[i].c_str(), hkt_codes[i].size() + 1);
    }
    iRet = lpNsqApi->ReqSecuDepthMarketDataSubscribe(reqField_sub, nCount, nRequestID);
    checkFunctionRet(lpNsqApi, iRet, nRequestID++);
}

// 期货市场订阅，包括上期所、上期能源、大商所、郑商所、中金所
void FutSubscribe(CHSNsqApi *lpNsqApi, const char *market, int len, int &nRequestID)
{
    if (g_user_info.sdk_config_market.count(market))
    {
        int nCount = 0, iRet;
        CHSNsqReqFutuDepthMarketDataField reqFutu_sub[1000];
        memcpy(reqFutu_sub[0].ExchangeID, market, len);
        if (g_user_info.market_codes.count(market))
        {
            auto &_codes = g_user_info.market_codes[market];
            for (size_t i = 0; i < _codes.size() && i < 1000; i++)
            {
                memcpy(reqFutu_sub[nCount].ExchangeID, market, len);
                memcpy(reqFutu_sub[nCount++].InstrumentID, _codes[i].c_str(), _codes[i].length() + 1);
            }
        }
        iRet = lpNsqApi->ReqFutuDepthMarketDataSubscribe(reqFutu_sub, nCount, nRequestID);
        checkFunctionRet(lpNsqApi, iRet, nRequestID++);
    }
}

// 沪深逐笔重建
void Rebuild(CHSNsqApi *lpNsqApi, int &nRequestID)
{
    // 逐笔重建请求
    // if (g_user_info.sdk_config_market.count(g_user_info.reqTransRebuild.ExchangeID))
    {
        int iRet = lpNsqApi->ReqSecuTransactionRebuild(&g_user_info.reqTransRebuild, nRequestID);
        checkFunctionRet(lpNsqApi, iRet, nRequestID++);
    }
}

// /**
//  * @brief 请求获取指定产品ID的证券信息
//  *
//  * 通过指定的产品ID请求获取相应的证券信息。
//  *
//  * @param lpNsqApi 指向CHSNsqApi对象的指针
//  * @param nProductID 产品ID
//  * @param nRequestID 请求ID，用于标识请求的唯一性
//  */
// void ReqProductIDInstruments(CHSNsqApi *lpNsqApi, HSQuoteProductID nProductID, int &nRequestID)
// {
//     int iRet = lpNsqApi->ReqProIDQrySecuInstruments(nProductID, nRequestID);
//     printf("ReqProIDQrySecuInstruments ProductID[%u] nRequestID[%d] iRet[%d]\n", nProductID, nRequestID, iRet);
//     checkFunctionRet(lpNsqApi, iRet, nRequestID++);
// }

// /**
//  * @brief 订阅产品ID
//  *
//  * 调用CHSNsqApi的ReqProIDSubscribe方法订阅产品ID，并打印请求参数和返回值。
//  *
//  * @param lpNsqApi 指向CHSNsqApi对象的指针
//  * @param pReqSubscribeField 订阅请求字段数组
//  * @param nCount 订阅请求字段数组的大小
//  * @param nRequestID 请求ID，函数执行后将递增
//  */
// void ProductIDSubscribe(CHSNsqApi *lpNsqApi, CHSNsqReqProductIDField pReqSubscribeField[], int nCount, int &nRequestID)
// {
//     int iRet = lpNsqApi->ReqProIDSubscribe(pReqSubscribeField, nCount, nRequestID);
//     printf("ReqProIDSubscribe nCount[%d] nRequestID[%d] iRet[%d]\n", nCount, nRequestID, iRet);
//     checkFunctionRet(lpNsqApi, iRet, nRequestID++);
// }

// /**
//  * @brief 取消产品ID订阅
//  *
//  * 通过指定的API接口取消产品ID的订阅。
//  *
//  * @param lpNsqApi          指向CHSNsqApi对象的指针，用于调用API接口
//  * @param pReqSubscribeField 指向包含订阅请求字段的数组，每个元素包含需要取消订阅的产品ID
//  * @param nCount            订阅请求字段数组的大小
//  * @param nRequestID        请求ID，用于标识请求，调用后会自动递增
//  */
// void ProductIDUnSubscribe(CHSNsqApi *lpNsqApi, CHSNsqReqProductIDField pReqSubscribeField[], int nCount, int &nRequestID)
// {
//     int iRet = lpNsqApi->ReqProIDSubscribeCancel(pReqSubscribeField, nCount, nRequestID);
//     printf("ReqProIDSubscribe nCount[%d] nRequestID[%d] iRet[%d]\n", nCount, nRequestID, iRet);
//     checkFunctionRet(lpNsqApi, iRet, nRequestID++);
// }