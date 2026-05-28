#ifndef __LAND_FILE_FUNCTION_H_
#define __LAND_FILE_FUNCTION_H_

#include <string>
#include <vector>
#include <map>
#include "HSNsqStruct.h"
#include "InnerData.h"

class CHSNsqApi;
class CHSNsqSpiImpl;

// 删除空格
int EraseSpace(std::string &str);
int EraseSpace(char *str);
// 按指定分隔符c分离字符串s并保存到v中
void SplitString(const std::string &s, std::vector<std::string> &v, const std::string &c);
// 读取user.properties
void readUserFile(UserInfo &user_info);
// 写线程工作函数
void thr_write_fn(CHSNsqSpiImpl *lpNsqSpi);
// 信号处理函数
void sigHandler(int sig);
// 检查NSQ API返回值
void checkFunctionRet(CHSNsqApi *lpNsqApi, int iRet, int nRequestID);
// 读取配置中的期货代码
void readFutCode(CHSNsqReqFutuDepthMarketDataField *reqFutu_sub, int &nCount, std::vector<std::string> &fut_code, const char *market);
// 读取sdk_config.ini中支持的市场
void readSDKConfig(UserInfo &user_info, std::string &sdk_cfg_path);
// 现货市场静态码表请求，包括沪深交易所、北交所、股转、申万指数
void ReqStockInstruments(CHSNsqApi *lpNsqApi, const char *market, int len, int &nRequestID);
// 期货市场静态码表请求，包括上期所、上期能源、大商所、郑商所、中金所
void ReqFutInstruments(CHSNsqApi *lpNsqApi, const char *market, int len, int &nRequestID);
// 现货市场订阅，包括沪深交易所、北交所、股转、申万指数
void StockSubscribe(CHSNsqApi *lpNsqApi, const char *market, int len, int &nRequestID);
// 期货市场订阅，包括上期所、上期能源、大商所、郑商所、中金所
void FutSubscribe(CHSNsqApi *lpNsqApi, const char *market, int len, int &nRequestID);
// 沪深逐笔重建
void Rebuild(CHSNsqApi *lpNsqApi, int &nRequestID);
// // ProductID码表请求
// void ReqProductIDInstruments(CHSNsqApi *lpNsqApi, HSQuoteProductID nProductID, int &nRequestID);
// // ProductID订阅
// void ProductIDSubscribe(CHSNsqApi *lpNsqApi, CHSNsqReqProductIDField pReqSubscribeField[], int nCount, int &nRequestID);
// // ProductID取消订阅
// void ProductIDUnSubscribe(CHSNsqApi *lpNsqApi, CHSNsqReqProductIDField pReqSubscribeField[], int nCount, int &nRequestID);
#endif