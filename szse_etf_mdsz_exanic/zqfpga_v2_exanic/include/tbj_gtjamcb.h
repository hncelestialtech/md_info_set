/**
 * @file tbj_gtjamcb.h
 * @author zhouzhengxin (zhouzhengxin@gtjas.com)
 * @brief 北交所统一组播行情协议
 * @version 1.0
 * @date 2022-03-22
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#define MD_TYPE_BJ_OFFSET 0x00100000U           // 北交所的行情类型偏移
#define MD_TYPE_BJ_L2 0x00100001U               // 北交所L2十档行情
#define MD_TYPE_BJ_L1 0x00100002U               // 北交所L1五档行情
#define MD_TYPE_BJ_INDEX 0x00100003U               // 北交所指数行情

#pragma pack(push, 1)
typedef struct t_bj_mdL2_mcb
{
	char    sSecurityCode[8];                    //证券代码
	int		nTime;				                 //时间(HHMMSSmmmm)
	int		nStatus;			                 //状态
	unsigned int       uPreClose;			     //前收盘价
	unsigned int       uOpen;				     //开盘价
	unsigned int       uHigh;				     //最高价
	unsigned int       uLow;				     //最低价
	unsigned int       uMatch;				     //最新价
	unsigned int       uAskPrice[10];	         //申卖价
	unsigned int       uAskVol[10];		         //申卖量
	unsigned int       uBidPrice[10];		     //申买价
	unsigned int       uBidVol[10];		         //申买量
	unsigned int       uNumTrades;			     //成交笔数
	long long	       iVolume;				     //成交总量
	long long	       iTurnover;				 //成交总金额
	long long	       iTotalBidVol;			 //委托买入总量
	long long	       iTotalAskVol;			 //委托卖出总量
	unsigned int       uWeightedAvgBidPrice;	 //加权平均委买价格
	unsigned int       uWeightedAvgAskPrice;     //加权平均委卖价格
	int		           nIOPV;					 //IOPV净值估值
	int		           nYieldToMaturity;		 //到期收益率
	unsigned int       uHighLimited;			 //涨停价
	unsigned int       uLowLimited;			     //跌停价
	char	           sPrefix[4];				 //证券信息前缀
	int		           nSyl1;					 //市盈率1 2 位小数 股票：价格/上年每股利润 债券：每百元应计利息
	int		           nSyl2;					 //市盈率2 2 位小数 股票：价格/本年每股利润 债券：到期收益率 基金：每百份的IOPV 或净值 权证：溢价率
	int		           nSD2;					 //升跌2（对比上一笔）
	char               sTradingPhraseCode[8];  //北交所无意义
	int                nPreIOPV;               //基金T-1日收盘时刻IOPV 仅标的为基金时有效
}T_BJ_MarketDataMCB, * PBJ_MarketDataMCB;

typedef struct t_bj_mdL1_mcb
{
	char               sSecurityCode[8];         //证券代码
	int		           nTime;				     //时间(HHMMSSmmm)
	int		           nStatus;			         //状态
	unsigned int       uPreClose;			     //前收盘价
	unsigned int       uOpen;				     //开盘价
	unsigned int       uHigh;				     //最高价
	unsigned int       uLow;				     //最低价
	unsigned int       uMatch;				     //最新价
	unsigned int       uAskPrice[5];	         //申卖价
	unsigned int       uAskVol[5];				 //申卖量
	unsigned int       uBidPrice[5];		     //申买价
	unsigned int       uBidVol[5];				 //申买量
	unsigned int       uNumTrades;			     //成交笔数
	long long	       iVolume;				     //成交总量
	long long	       iTurnover;				 //成交总金额
	unsigned int       uHighLimited;			 //涨停价
	unsigned int       uLowLimited;			     //跌停价
	char               sTradingPhraseCode[8];    //北交所无意义
	int                nPreIOPV;                 //基金T-1日收盘时刻IOPV 仅标的为基金时有效
	int                nIOPV;                    //基金IOPV  仅标的为基金时有效
}T_BJ_MarketDataMCB_L1, * PBJ_MarketDataMCB_L1;

typedef struct bj_mdindex_mcb
{
	char sSecurityCode[8];         //证券代码
	int nTime;		               //时间(HHMMSSmmm)
	int nOpenIndex;	               //今开盘指数
	int nHighIndex;	               //最高指数
	int nLowIndex;	               //最低指数
	int nLastIndex;	               //最新指数
	long long iTotalVolume;	       //参与计算相应指数的交易数量
	long long iTurnover;	       //参与计算相应指数的成交金额
	int nPreCloseIndex;            //前盘指数
} T_BJ_IndexDataMCB, * PBJ_IndexDataMCB;

#pragma pack(pop)