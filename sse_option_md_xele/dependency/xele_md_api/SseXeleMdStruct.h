/*****************************************************************************
艾科朗克上交所行情结构体定义，字节序为小端
******************************************************************************/
#ifndef SSE_XELE_MD_STRUCT_H
#define SSE_XELE_MD_STRUCT_H
#include <stdint.h>


#define SSE_STRUCT_VERSION                    "1.4"      //结构体定义版本号
//L2消息类型                                  
#define MSG_TYPE_SNAPSHOT                      0x01      //上交快照
#define MSG_TYPE_BEST_ORDERS                   0x02      //订单明细，最多揭示50笔
#define MSG_TYPE_INDEX                         0x03      //指数行情
#define MSG_TYPE_AFTER_SNAP_SSE                0x07      //上交盘后快照 
#define MSG_TYPE_AFTER_TRADE_SSE               0x08      //上交盘后逐笔成交
#define MSG_TYPE_BOND_SNAPSHOT                 0x0C      //上交债券快照
#define MSG_TYPE_BOND_TICK                     0x0D      //上交债券逐笔
#define MSG_TYPE_ETF_SNAPSHOT                  0x10      //上交ETF统计
#define MSG_TYPE_TICK_MERGE                    0x11      //上交逐笔合并消息
#define MSG_TYPE_TREE_SNAP_SSE                 0x12      //上交逐笔构建快照消息（订单簿）
#define MSG_TYPE_BOND_TREE_SNAP_SSE            0x13      //上交债券逐笔构建快照消息（订单簿）
#define MSG_TYPE_STATIC_SSE                    0x0F      //上交静态信息
//L1消息类型                                  
#define MSG_TYPE_INDEX_SSE_L1                  0x01      //指数行情，交易所类型MD001
#define MSG_TYPE_STOCK_SNAP_SSE_L1             0x02      //股票快照，交易所类型MD002
#define MSG_TYPE_BOND_SNAP_SSE_L1              0x03      //债券分销快照，交易所类型MD003
#define MSG_TYPE_FUND_SSE_L1                   0x04      //基金，交易所类型MD004
#define MSG_TYPE_OPTION_SSE_L1                 0x05      //期权，交易所类型MD301
#define MSG_TYPE_NATIONAL_DEBT_SSE_L1          0x06      //国债，交易所类型MD101
#define MSG_TYPE_AFTER_TRADE_SSE_L1            0x07      //盘后固定价格，交易所类型MD102
#define MSG_TYPE_MD201_BOND_SNAP               0x08      //债券快照，交易所类型MD201
#define MSG_TYPE_IOPV_L1                       0x09      //IOPV，交易所类型MDE01
#define SNAPSHOT_LEVEL                         10


#pragma pack(push, 1)
struct BidAskPriceQtySse {
    uint32_t           price;                            //申买、申卖价格，实际值除以1000
    uint64_t           qty;                              //申买、申卖数量，实际值除以1000
};
    
/*
* 行情快照
*/
struct MarketDataSnapshotSse {
    uint8_t            messageType;                      //消息类型，快照为0x1
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所：1，深交所：2
    char               securityID[9];                    //证券代码
    uint8_t            resv[2];                          //保留字段
    /*
    产品所处的交易阶段代码
    该字段为8位字符串，左起每位表示特定的含义，无定义则填空格。
    第1位：‘S’表示启动（开市前）时段，‘C’表示开盘集合竞价时段，‘T’表示连续交易时段，‘E’表示闭市时段，‘P’表示产品停牌，
    	‘M’表示可恢复交易的熔断时段（盘中集合竞价），‘N’表示不可恢复交易的熔断时段（暂停交易至闭市），‘U’表示收盘集合竞价时段。
    第2位：‘0’表示此产品不可正常交易，‘1’表示此产品可正常交易，无意义填空格。
    第3位：‘0’表示未上市，‘1’表示已上市。
    第4位：‘0’表示此产品在当前时段不接受订单申报，‘1’ 表示此产品在当前时段可接受订单申报。无意义填空格
    */
    char               tradingPhaseCode[8];
    /*
    * 当前品种交易状态：
    * - START：启动
    * - OCALL：开市集合竞价
    * - TRADE：连续自动撮合
    * - SUSP：停牌
    * - CCALL：收盘集合竞价
    * - CLOSE：闭市，自动计算闭市价格
    * - ENDTR：交易结束
    * 详见上交所接口说明文档
    */
    char               instrumentStatus[6];
    uint8_t            resv2[5];                         //保留字段
    uint32_t           timeStamp;                        //最新订单时间，格式HHMMSS，例如143025 表示 14:30:25
    uint32_t           preClosePrice;                    //昨收价（来源消息头)，实际值除以1000
    uint32_t           numTrades;                        //总成交笔数
    uint64_t           totalVolumeTrade;                 //总成交量，实际值除以1000
    uint64_t           totalValueTrade;                  //总成交金额，实际值除以100000
    uint32_t           lastPrice;                        //最近价，实际值除以1000
    uint32_t           openPrice;                        //开盘价，实际值除以1000
    uint32_t           closePrice;                       //今日收盘价，实际值除以1000
    uint32_t           highPrice;                        //最高价，实际值除以1000
    uint32_t           lowPrice;                         //最低价，实际值除以1000
    uint64_t           IOPV;                             //ETF净值估值，实际值除以1000
    uint64_t           warUpperPx;                       //IOPV高精度值，实际值除以100000
    uint32_t           bidAvgPrice;                      //买入委托加权平均价，实际值除以1000
    uint64_t           bidTotalQty;                      //买入委托总数量，实际值除以1000
    uint32_t           askAvgPrice;                      //卖出委托加权平均价，实际值除以1000
    uint64_t           askTotalQty;                      //卖出委托总数量，实际值除以1000
    BidAskPriceQtySse  bidInfo[SNAPSHOT_LEVEL];          //申买信息
    BidAskPriceQtySse  askInfo[SNAPSHOT_LEVEL];          //申卖信息
    uint32_t           msgSeqID;                         //消息序号
    uint32_t           withdrawBuyNum;                   //买入撤单笔数
    uint64_t           withdrawBuyAmount;                //买入撤单数量，实际值除以1000
    uint64_t           withdrawBuyMoney;                 //买入撤单金额，实际值除以100000
    uint32_t           withdrawSellNum;                  //卖出撤单笔数
    uint64_t           withdrawSellAmount;               //卖出撤单数量，实际值除以1000
    uint64_t           withdrawSellMoney;                //卖出撤单金额，实际值除以100000
};
    
/*
* 订单明细，最多揭示50笔
*/
struct BestOrdersSse {
    uint8_t            messageType;                      //消息类型，订单明细为0x2
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所：1，深交所：2
    char               securityID[9];                    //证券代码
    char               recv[3];                          //保留字段
    uint32_t           msgSeqID;                         //消息序号
    uint8_t            side;                             //买卖标识：买：1，卖：2
    uint8_t            number;                           //明细个数
    uint32_t           timeStamp;                        //最新订单时间，格式HHMMSS，例如143025 表示 14:30:25
    uint32_t           price;                            //委托价格，实际值除以1000
    uint64_t           orders;                           //申买/卖数量，实际值除以1000
    uint64_t           volume[50];                       //委托数量，最多揭示50笔，实际值除以1000
};
    
/*
* 指数行情
*/
struct IndexSse {
    uint8_t            messageType;                      //消息类型，指数为0x3
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所：1，深交所：2
    char               securityID[9];                    //证券代码
    uint8_t            resv[5];                          //保留字段
    uint32_t           timeStamp;                        //最新订单时间，格式HHMMSS，例如143025 表示 14:30:25
    uint32_t           tradeTime;                        //成交时间，格式HHMMSS，例如143025 表示 14:30:25
    uint32_t           msgSeqID;                         //消息序号
    uint64_t           preClosePrice;                    //前盘指数（来源扩展字段），实际值除以100000
    uint64_t           openPrice;                        //开盘指数，实际值除以100000
    uint64_t           lastPrice;                        //最新指数，实际值除以100000
    uint64_t           highPrice;                        //最高指数，实际值除以100000
    uint64_t           lowPrice;                         //最低指数，实际值除以100000
    uint64_t           closePrice;                       //今日收盘指数，实际值除以100000
    uint64_t           totalVolume;                      //成交总量，实际值除以100000
    uint64_t           totalValue;                       //成交总金额，实际值除以10
};
    
/*
* 上交盘后逐笔成交
*/
struct AfterTradeSse {
    uint8_t            messageType;                      //消息类型，上交盘后逐笔成交为0x8
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所：1，深交所：2
    char               securityID[9];                    //证券代码
    char               tradeBSFlag;                      //内外盘标志 ：B-外盘，主动买，S-内盘，主动买，N-未知
    uint32_t           tradeIndex;                       //上交所：成交序号
    uint32_t           channelNo;                        //成交通道
    uint32_t           transactTime;                     //成交时间，格式HHMMSSmm，精确到10毫秒，例如10031945代表10点03分19秒450毫秒
    uint32_t           tradePrice;                       //成交价格，实际值除以1000
    uint64_t           tradeQty;                         //成交数量，实际值除以1000
    uint64_t           tradeMoney;                       //成交金额，实际值除以100000
    uint64_t           bidapplSeqnum;                    //买方委托索引
    uint64_t           offerapplSeqnum;                  //卖方委托索引
    uint32_t           msgSeqID;                         //消息序号
    char               resv2[4];                         //保留字段
};
    
/*
*上交盘后行情快照
*/
struct AfterSnapshotSse {
    uint8_t            messageType;                      //消息类型，上交快照为0x7
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所：1，深交所：2
    char               securityID[9];                    //证券代码
    uint8_t            resv;                             //保留字段
    /*
    * 当前品种交易状态：
    * - START：启动
    * - OCALL：开市集合竞价
    * - TRADE：连续自动撮合
    * - SUSP：停牌
    * - CCALL：收盘集合竞价
    * - CLOSE：闭市，自动计算闭市价格
    * - ENDTR：交易结束
    * 详见上交所接口说明文档
    */
    char               instrumentStatus[6];
    uint32_t           timeStamp;                        //最新订单时间，格式HHMMSS，例如143025 表示 14:30:25
    uint32_t           closePrice;                       //今日收价，实际值除以1000
    uint32_t           numTrades;                        //总成交笔数
    uint64_t           totalVolumeTrade;                 //总成交量，实际值除以1000
    uint64_t           totalValueTrade;                  //总成交金额，实际值除以100000
    uint64_t           totalBidQty;                      //买入委托总数量，实际值除以1000
    uint64_t           totalAskQty;                      //卖出委托总数量，实际值除以1000
    uint32_t           withdrawBuyNumber;                //买入撤单笔数
    uint64_t           withdrawBuyAmount;                //买入撤单数量，实际值除以1000
    uint32_t           withdrawSellNumber;               //卖出撤单笔数
    uint64_t           withdrawSellAmount;               //卖出撤单数量，实际值除以1000
    uint32_t           msgSeqID;                         //消息序号
    uint32_t           noBidLevel;                       //买盘价位数量
    uint32_t           noOfferLevel;                     //卖盘价位数量
    uint64_t           bidQty;                           //一档申买量，实际值除以1000
    uint64_t           askQty;                           //一档申卖量，实际值除以1000
    uint64_t           bidOrderNum;                      //买一价申买量揭示笔数
    uint64_t           askOrderNum;                      //卖一价申卖量揭示笔数
};
    
/*
* 债券快照
*/
struct BondSnapshotSse {
    uint8_t            messageType;                      //消息类型，债券快照为0x0C
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所：1，深交所：2
    char               securityID[9];                    //证券代码
    char               resv[2];                          //保留字段
    char               instrumentStatus[6];              //当前品种交易状态,详见上交所接口说明文档
    char               resv2;                            //保留字段
    uint32_t           timeStamp;                        //最新订单时间，精确到毫秒，格式HHMMSSmmm，例如143025001 表示 14:30:25:001
    uint32_t           preClosePrice;                    //昨收价（来源消息头)，实际值除以1000
    uint32_t           openPrice;                        //开盘价，实际值除以1000
    uint32_t           highPrice;                        //最高价，实际值除以1000
    uint32_t           lowPrice;                         //最低价，实际值除以1000
    uint32_t           lastPrice;                        //最近价，实际值除以1000
    uint32_t           closePrice;                       //今日收盘价，实际值除以1000
    uint32_t           numTrades;                        //总成交笔数
    uint64_t           totalVolumeTrade;                 //总成交量，实际值除以1000
    uint64_t           totalValueTrade;                  //总成交金额，实际值除以100000
    uint64_t           bidTotalQty;                      //买入委托总数量，实际值除以1000
    uint32_t           bidAvgPrice;                      //买入委托加权平均价，实际值除以1000
    uint64_t           askTotalQty;                      //卖出委托总数量，实际值除以1000
    uint32_t           askAvgPrice;                      //卖出委托加权平均价，实际值除以1000
    uint32_t           avgPrice;                         //加权平均价，实际值除以1000
    BidAskPriceQtySse  bidInfo[SNAPSHOT_LEVEL];          //申买信息
    BidAskPriceQtySse  askInfo[SNAPSHOT_LEVEL];          //申卖信息
    uint32_t           msgSeqID;                         //消息序号
    uint32_t           withdrawBuyNum;                   //买入撤单笔数
    uint64_t           withdrawBuyAmount;                //买入撤单数量，实际值除以1000
    uint64_t           withdrawBuyMoney;                 //买入撤单金额，实际值除以100000
    uint32_t           withdrawSellNum;                  //卖出撤单笔数
    uint64_t           withdrawSellAmount;               //卖出撤单数量，实际值除以1000
    uint64_t           withdrawSellMoney;                //卖出撤单金额，实际值除以100000    
};
    
/*
* 债券逐笔
*/
struct BondTickSse {
    uint8_t            messageType;                      //消息类型，债券逐笔为0x0D
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所：1，深交所：2
    char               securityID[9];                    //证券代码
    char               tickType;                         //类型：A=新增委托订单 D=删除委托订单 S=产品状态订单 T=成交
    /*
    订单状态
    为新增或删除委托订单时：
    0-买单  1-卖单
    为产品状态订单时；
    0=产品未上市 1=启动 2=开市集合竞价 3=连续自动撮合 4=停牌 5=闭市 6=交易结束
    为成交时：
    0-外盘，主动买 1-内盘，主动卖 2-未知
    */
    char               tickBSFlag;
    uint32_t           tickIndex;                        //逐笔序号,从1开始，按channel连续
    uint32_t           channelNo;                        //频道代码
    uint32_t           tickTime;                         //订单或成交时间，精确到毫秒，格式HHMMSSmmm，例如143025001 表示 14:30:25:001
    uint64_t           buyOrderNo;                       //买方订单号
    uint64_t           sellOrderNo;                      //卖方订单号
    uint32_t           price;                            //价格，实际值除以1000
    uint64_t           qty;                              //数量，实际值除以1000
    uint64_t           tradeMoney;                       //成交金额，仅适用于TickType=T时，实际值除以100000
    uint32_t           msgSeqID;                         //消息序号（仅定位调试使用）
    /* 标记当前包是否是同一个fast消息中解出的最后一笔行情
    如果开启了标记合约在fast压缩包中最后一次出现的功能，
    则含义变为当前包是否为该合约在fast压缩包中最后一次出现
    0-非最后  1-最后 */
    uint8_t            isLast;
    char               resv[2];                          //保留字段
};
    
/*
* 竞价逐笔合并
*/
struct TickMergeSse {
    uint8_t            messageType;                      //消息类型，竞价逐笔合并为0x11
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所：1，深交所：2
    char               securityID[9];                    //证券代码
    char               tickType;                         //类型：A=新增委托订单 D=删除委托订单 S=产品状态订单 T=成交
    /*
    订单状态
    为新增或删除委托订单时：
    0-买单  1-卖单
    为产品状态订单时；
    0=启动 1=开市集合竞价 2=连续自动撮合 3=停牌 4=收盘集合竞价 5=闭市 6=交易结束
    为成交时：
    0-外盘，主动买 1-内盘，主动卖 2-未知
    */
    char               tickBSFlag;
    uint64_t           bizIndex;                         //逐笔序号,从1开始，按channel连续
    uint32_t           channelNo;                        //频道代码
    uint32_t           tickTime;                         //订单或成交时间，精确到10毫秒，格式HHMMSSmm，例如14302501 表示 14:30:25:01
    uint64_t           buyOrderNo;                       //买方订单号
    uint64_t           sellOrderNo;                      //卖方订单号
    uint32_t           price;                            //价格，实际值除以1000
    uint64_t           qty;                              //数量，实际值除以1000
    //TickType=T时，代表成交金额，实际值除以100000；TickType=A时，代表已成交委托数量，实际值除以1000，其他无意义·
    uint64_t           tradeMoney;     
    uint32_t           msgSeqID;                         //消息序号（仅定位调试使用）
    /* 标记当前包是否是同一个fast消息中解出的最后一笔行情
    如果开启了标记合约在fast压缩包中最后一次出现的功能，
    则含义变为当前包是否为该合约在fast压缩包中最后一次出现
    0-非最后  1-最后 */
    uint8_t            isLast;
    char               resv[6];                          //保留字段
};
    
/*
* ETF行情统计信息
*/
struct ETFSnapshotSse {
    uint8_t            messageType;                      //消息类型，ETF快照为0x10
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所：1，深交所：2
    char               securityID[9];                    //证券代码
    uint32_t           timeStamp;                        //更新时间，格式HHMMSS，例如143025 表示 14:30:25
    uint32_t           etfBuyNum;                        //ETF申购笔数
    uint64_t           etfBuyAmount;                     //ETF申购数量，实际值除以1000
    uint64_t           etfBuyMoney;                      //ETF申购金额，实际值除以100000
    uint32_t           etfSellNum;                       //ETF赎回笔数
    uint64_t           etfSellAmount;                    //ETF赎回数量，实际值除以1000
    uint64_t           etfSellMoney;                     //ETF赎回金额，实际值除以100000
    uint32_t           msgSeqID;                         //消息序号（仅定位调试使用）
    char               resv[1];                          //保留字段
};
    
//上交静态信息结构体
typedef struct StaticInfo
{
    uint8_t            messageType;                      //消息类型，静态消息类型为0x0F
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所：1，深交所：2
    char               securityID[9];                    //证券代码
    /*
    涨跌停限制类型
    ‘N’表示交易规则（2013修订版）3.4.13规定的有涨跌幅限制类型或者权证管理办法第22条规定
    ‘R’表示交易规则（2013修订版）3.4.15和3.4.16规定的无涨跌幅限制类型
    ‘S’表示回购涨跌幅控制类型
    ‘F’表示基于参考价格的涨跌幅控制
    ‘P’表示IPO上市首日的涨跌幅控制类型
    ‘U’表示无任何价格涨跌幅控制类型
    */
    char               priceLimitType;
    double             upperLimitPrice;                  //涨停价
    double             lowerLimitPrice;                  //跌停价
    uint64_t           buyUnit;                          //买数量单位
    uint64_t           sellUnit;                         //卖数量单位
    uint64_t           upperQuantityLimitPriceDeclare;   //限价申报数量上限
    uint64_t           lowerQuantityLimitPriceDeclare;   //限价申报数量下限
    double             priceGear;                        //价格档位,申报价格的最小变动单位
    uint64_t           upperQuantityMarketPriceDeclare;  //市价申报数量上限
    uint64_t           lowerQuantityMarketPriceDeclare;  //市价申报数量下限
    /** 
    * 证券类别
    * ‘ES’表示股票；‘EU’表示基金；‘D’表示债券； ‘RWS’表示权证；‘FF’表示期货；
    * 'CB'表示公募REITs。（参考ISO10962），集合资产管理计划、债券预发行、定向可转债取‘D’
    */
    char               securityType[7];
    char               securityName[9];                  //证券名称                      
    char               fileDate[9];                      //文件日期(YYYYMMDD)
    char               resv[3];                          //保留字段                   
    char               securitySubType[4];               //详细证券类别,参考《上海证券市场竞价撮合平台市场参与者接口规格说明书》
    char               financeFlag;                      //融资标的标志,‘T’表示是融资标的证券,‘F’表示不是融资标的证券
    char               shortSaleFlag;                    //融券标的标志,‘T’表示是融券标的证券,‘F’表示不是融券标的证券
    char               productStatus[21];                //产品状态,参考《上海证券市场竞价撮合平台市场参与者接口规格说明书》
    char               listDate[9];                      //上市日期(YYYYMMDD)
    
} StaticInfoSse;
    
/*
* 股票、债券逐笔合并转行情快照结构体
*/
struct MarketDataTreeSnapSse {
    uint8_t            messageType;                      //消息类型，股票快照为0x12，债券快照为0x13
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所：1，深交所：2
    char               securityID[9];                    //证券代码
    uint8_t            signLoss;                         //丢包标志,1表示丢包， 0表示未丢包
    uint64_t           applSeqNum;                       //构建行情的逐笔序号
    uint32_t           timeStamp;                        //更新时间，取的对应的逐笔里的时间，股票精确到10ms，债券精确到1ms
    uint32_t           preClosePrice;                    //昨收价（来源消息头)，实际值除以1000
    uint32_t           numTrades;                        //总成交笔数
    uint64_t           totalVolumeTrade;                 //总成交量，实际值除以1000
    uint64_t           totalValueTrade;                  //总成交金额，实际值除以100000
    uint32_t           lastPrice;                        //最近价，实际值除以1000
    uint32_t           openPrice;                        //开盘价，实际值除以1000
    uint32_t           highPrice;                        //最高价，实际值除以1000
    uint32_t           lowPrice;                         //最低价，实际值除以1000
    uint32_t           bidAvgPrice;                      //买入委托加权平均价，实际值除以1000
    uint64_t           bidTotalQty;                      //买入委托总数量，实际值除以1000
    uint32_t           askAvgPrice;                      //卖出委托加权平均价，实际值除以1000
    uint64_t           askTotalQty;                      //卖出委托总数量，实际值除以1000
    BidAskPriceQtySse  bidInfo[10];                      //10档申买信息
    BidAskPriceQtySse  askInfo[10];                      //10档申卖信息
};
    
    
/********************
* L1消息结构体
* *****************/
struct priceQty {
    uint64_t           price;                            //实际值除以100000
    uint64_t           qty;
};
    
//指数
struct IndexSseL1 {
    uint8_t            messageType;                      //消息类型，指数为0x1
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所L2：1，深交所：2，上交L1：3
    char               securityID[9];                    //证券代码
    uint16_t           flag;
    char               recv[3];
    uint8_t            securityType;
    uint8_t            tradSesMode;                      //交易盘交易模式：1 = 系统测试，2 = 模拟交易，3 = 产品（正常交易）
    uint64_t           sendingTime;                      //发送时间，格式： YYYYMMDDHHmmSSsss	
    uint32_t           tradeDate;                        //交易日期 YYYYMMDD 
    uint32_t           lastUpdateTime;                   //最新更新时间 HHMMSSsss 
    uint64_t           preClosePrice;                    //实际值除以100000
    uint64_t           totalVolumeTraded;                
    uint64_t           tradeNum;                         
    uint64_t           totalValueTraded;                 //实际值除以100
    char               tradingPhaseCode[8];              
    uint64_t           lastIndex;                        //实际值除以100000
    uint64_t           openIndex;                        //实际值除以100000
    uint64_t           closeIndex;                       //实际值除以100000
    uint64_t           highIndex;                        //实际值除以100000
    uint64_t           lowIndex;                         //实际值除以100000
};
    
//股票、债券快照
struct StockBondSnapSseL1 {
    uint8_t            messageType;                      //消息类型，股票快照类型为0x2,债券分销为0x3,债券为0x8
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所L2：1，深交所：2，上交L1：3
    char               securityID[9];                    //证券代码
    uint16_t           flag;
    char               recv[3];
    uint8_t            securityType;
    uint8_t            tradSesMode;                      //交易盘交易模式：1 = 系统测试，2 = 模拟交易，3 = 产品（正常交易）
    uint64_t           sendingTime;                      //发送时间，格式： YYYYMMDDHHmmSSsss	
    uint32_t           tradeDate;                        //交易日期 YYYYMMDD 
    uint32_t           lastUpdateTime;                   //最新更新时间 HHMMSSsss 
    uint64_t           preClosePrice;                    //实际值除以100000
    uint64_t           totalVolumeTraded;                
    uint64_t           tradeNum;                         
    uint64_t           totalValueTraded;                 //实际值除以100
    char               tradingPhaseCode[8];              
    uint64_t           lastPrice;                        //实际值除以100000
    uint64_t           openPrice;                        //实际值除以100000
    uint64_t           closePrice;                       //实际值除以100000
    uint64_t           settlePrice;                      //实际值除以100000
    uint64_t           highPrice;                        //实际值除以100000
    uint64_t           lowPrice;                         //实际值除以100000
    uint64_t           preSettlePrice;                   //实际值除以100000
    struct priceQty    bidPriceQty[5];
    struct priceQty    askPriceQty[5];
};
    
//国债
struct NationalDebtSseL1 {
    uint8_t            messageType;                      //消息类型，国债为0x6
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所L2：1，深交所：2，上交L1：3
    char               securityID[9];                    //证券代码
    uint16_t           flag;
    char               recv[3];
    uint8_t            securityType;
    uint8_t            tradSesMode;                      //交易盘交易模式：1 = 系统测试，2 = 模拟交易，3 = 产品（正常交易）
    uint64_t           sendingTime;                      //发送时间，格式： YYYYMMDDHHmmSSsss	
    uint32_t           tradeDate;                        //交易日期 YYYYMMDD 
    uint32_t           lastUpdateTime;                   //最新更新时间 HHMMSSsss 
    uint64_t           preClosePrice;                    //实际值除以100000
    uint64_t           totalVolumeTraded;                
    uint64_t           tradeNum;                         
    uint64_t           totalValueTraded;                 //实际值除以100
    char               tradingPhaseCode[8];              
    uint64_t           lastPrice;                        //实际值除以100000
    uint64_t           openPrice;                        //实际值除以100000
    uint64_t           closePrice;                       //实际值除以100000
    uint64_t           settlePrice;                      //实际值除以100000
    uint64_t           highPrice;                        //实际值除以100000
    uint64_t           lowPrice;                         //实际值除以100000
    uint64_t           preSettlePrice;                   //实际值除以100000
    struct priceQty    bidPriceQty[5];
    struct priceQty    askPriceQty[5];
};
    
//基金,IOPV
struct FundSseL1 {
    uint8_t            messageType;                      //消息类型，基金为0x4
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所L2：1，深交所：2，上交L1：3
    char               securityID[9];                    //证券代码
    uint16_t           flag;
    char               recv[3];
    uint8_t            securityType;
    uint8_t            tradSesMode;                      //交易盘交易模式：1 = 系统测试，2 = 模拟交易，3 = 产品（正常交易）
    uint64_t           sendingTime;                      //发送时间，格式： YYYYMMDDHHmmSSsss	
    uint32_t           tradeDate;                        //交易日期 YYYYMMDD 
    uint32_t           lastUpdateTime;                   //最新更新时间 HHMMSSsss 
    uint64_t           preClosePrice;                    //实际值除以100000
    uint64_t           totalVolumeTraded;
    uint64_t           tradeNum;
    uint64_t           totalValueTraded;                 //实际值除以100
    char               tradingPhaseCode[8];              
    uint64_t           lastPrice;                        //实际值除以100000
    uint64_t           openPrice;                        //实际值除以100000
    uint64_t           closePrice;                       //实际值除以100000
    uint64_t           settlePrice;                      //实际值除以100000
    uint64_t           highPrice;                        //实际值除以100000
    uint64_t           lowPrice;                         //实际值除以100000
    uint64_t           preSettlePrice;                   //实际值除以100000
    uint64_t           IOPV;                             //实际值除以100000
    uint64_t           preCloseIOPV;                     //实际值除以100000
    struct priceQty    bidPriceQty[5];
    struct priceQty    askPriceQty[5];
};
    
//期权
struct OptionSseL1 {
    uint8_t            messageType;                      //消息类型，期权为0x5
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所L2：1，深交所：2，上交L1：3
    char               securityID[9];                    //证券代码
    uint16_t           flag;
    char               recv[3];
    uint8_t            securityType;
    uint8_t            tradSesMode;                      //交易盘交易模式：1 = 系统测试，2 = 模拟交易，3 = 产品（正常交易）
    uint64_t           sendingTime;                      //发送时间，格式： YYYYMMDDHHmmSSsss	
    uint32_t           tradeDate;                        //交易日期 YYYYMMDD 
    uint32_t           lastUpdateTime;                   //最新更新时间 HHMMSSsss 
    uint64_t           preClosePrice;                    //实际值除以100000
    uint64_t           totalVolumeTraded;                
    uint64_t           tradeNum;                         
    uint64_t           totalValueTraded;                 //实际值除以100
    char               tradingPhaseCode[8];              //实际值除以100000
    uint64_t           lastPrice;                        //实际值除以100000
    uint64_t           openPrice;                        //实际值除以100000  
    uint64_t           closePrice;                       //实际值除以100000
    uint64_t           settlePrice;                      //实际值除以100000
    uint64_t           highPrice;                        //实际值除以100000
    uint64_t           lowPrice;                         //实际值除以100000
    uint64_t           preSettlePrice;                   //实际值除以100000
    uint64_t           auctionPrice;                     //实际值除以100000
    uint64_t           auctionQty;                       //实际值除以100000
    uint64_t           totalLongPosition;
    struct priceQty    bidPriceQty[5];
    struct priceQty    askPriceQty[5];
};
    
//盘后
struct AfterTradeSseL1 {
    uint8_t            messageType;                      //消息类型，盘后为0x7
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所L2：1，深交所：2，上交L1：3
    char               securityID[9];                    //证券代码
    uint16_t           flag;
    char               recv[3];
    uint8_t            securityType;
    uint8_t            tradSesMode;                      //交易盘交易模式：1 = 系统测试，2 = 模拟交易，3 = 产品（正常交易）
    uint64_t           sendingTime;                      //发送时间，格式： YYYYMMDDHHmmSSsss	
    uint32_t           tradeDate;                        //交易日期 YYYYMMDD 
    uint32_t           lastUpdateTime;                   //最新更新时间 HHMMSSsss 
    uint64_t           preClosePrice;                    //实际值除以100000  
    uint64_t           totalVolumeTraded;                
    uint64_t           tradeNum;                         
    uint64_t           totalValueTraded;                 //实际值除以100
    char               tradingPhaseCode[8];              
    uint64_t           notTradedBidVolume;               
    uint64_t           notTradedAskVolume;               
    uint64_t           closePrice;                       //实际值除以100000
};
#pragma pack(pop)
#endif
