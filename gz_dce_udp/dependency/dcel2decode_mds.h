#ifndef __DCEL2DECODE_MDS_H__
#define __DCEL2DECODE_MDS_H__
#include <stdint.h>
#if (defined WIN32)||(defined _WIN64)||(defined _WINDOWS_)
#ifdef EXPORT
#define _DCEL2DECODE_MDS_API_ __declspec(dllexport)
#else
#define _DCEL2DECODE_MDS_API_ __declspec(dllimport)
#endif
#else
#define _DCEL2DECODE_MDS_API_
#endif

#ifdef __cplusplus
extern "C"{
#endif
#pragma pack(push,1)
	/*
	* 数据包头
	*/
	struct msg_header {
		uint16_t MsgSize;//total size of the packet,including the header.
		uint16_t MsgType;
		uint8_t  Version;
		uint8_t  Flag;
		uint8_t  MdgNo;
		uint64_t SeqNo;
		uint8_t  SeqNum;
		uint64_t SendTime;
		uint8_t  Reserved;
	};

	/*
	* 合约信息
	*/
	struct mds_contract_info {
		char     ExchangeId[9];   //交易所
		char     ContractId[129]; //合约代码
		char     ProductId[5];   //产品代码
		char     ProductClass[5];//产品类别
		uint32_t TradeDate;       //行情日期 YYYYMMDD
		char     GenTime[13];     //生成时间 HH:MM:SS.sss
		uint32_t ChannelNo;       //通道编号 (组播使用)
		uint32_t PackageNo;       //本通道的报文编号
	};

	/*
	* 单腿合约最优价
	*/
	struct mds_best {
		double   LastPrice;            //最新价
		double   HighPrice;            //最高价
		double   LowPrice;             //最低价
		uint32_t LastMatchQty;         //最新成交量
		uint32_t MatchTotQty;          //成交数量
		double   Turnover;             //成交金额
		uint32_t LastOpenInterest;     //昨持仓量
		uint32_t OpenInterest;         //持仓量
		int32_t  InterestChg;          //持仓量变化
		double   ClearPrice;           //今结算价
		double   LifeLow;              //历史最低价
		double   LifeHigh;             //历史最高价
		double   RiseLimit;            //最高报价
		double   FallLimit;            //最低报价
		double   LastClearPrice;       //昨结算价
		double   LastClose;            //昨收盘
		double   BuyPriceOne;          //买入价格1
		uint32_t BuyQtyOne;            //买入数量1
		uint32_t BuyImplyQtyOne;       //买1推导量
		double   SellPriceOne;         //卖出价格1
		uint32_t SellQtyOne;           //卖出数量1
		uint32_t SellImplyQtyOne;      //卖1推导量
		double   AvgPrice;             //成交均价
		double   OpenPrice;            //今开盘
		double   ClosePrice;           //今收盘
	};

	/*
	* 套利合约最优价
	*/
	struct mds_arbi_best {
		double   LastPrice;      //最新价
		uint32_t LastMatchQty;   //最新成交量
		double   LowPrice;       //最低价
		double   HighPrice;      //最高价
		double   LifeLow;        //历史最低价
		double   LifeHigh;       //历史最高价
		double   RiseLimit;      //最高报价
		double   FallLimit;      //最低报价
		double   BuyPriceOne;    //买入价格1
		uint32_t BuyQtyOne;      //买入数量1
		double   SellPriceOne;   //卖出价格1
		uint32_t SellQtyOne;     //卖出数量1
	};

	/*
	* 深度行情数据
	*/
	struct mds_deep {
		double   BuyPriceOne;      //买入价格1
		uint32_t BuyQtyOne;        //买入数量1
		uint32_t BuyImplyQtyOne;   //买1推导量
		double   BuyPriceTwo;      //买入价格2
		uint32_t BuyQtyTwo;        //买入数量2
		uint32_t BuyImplyQtyTwo;   //买2推导量
		double   BuyPriceThree;    //买入价格3
		uint32_t BuyQtyThree;      //买入数量3
		uint32_t BuyImplyQtyThree; //买3推导量
		double   BuyPriceFour;     //买入价格4
		uint32_t BuyQtyFour;       //买入数量4
		uint32_t BuyImplyQtyFour;  //买4推导量
		double   BuyPriceFive;     //买入价格5
		uint32_t BuyQtyFive;       //买入数量5
		uint32_t BuyImplyQtyFive;  //买5推导量

		double   SellPriceOne;      //卖出价格1
		uint32_t SellQtyOne;        //卖出数量1
		uint32_t SellImplyQtyOne;   //卖1推导量
		double   SellPriceTwo;      //卖出价格2
		uint32_t SellQtyTwo;        //卖出数量2
		uint32_t SellImplyQtyTwo;   //卖2推导量
		double   SellPriceThree;    //卖出价格3
		uint32_t SellQtyThree;      //卖出数量3
		uint32_t SellImplyQtyThree; //卖3推导量
		double   SellPriceFour;     //卖出价格4
		uint32_t SellQtyFour;       //卖出数量4
		uint32_t SellImplyQtyFour;  //卖4推导量
		double   SellPriceFive;     //卖出价格5
		uint32_t SellQtyFive;       //卖出数量5
		uint32_t SellImplyQtyFive;  //卖5推导量
	};

	/*
	* 最优价位上十笔委托
	*/
	struct mds_ten_entrust {
		double   BestBuyOrderPrice;     //最优买价格
		double   BestSellOrderPrice;    //最优卖价格
		uint32_t BestBuyOrderQtyOne;    //委托量1
		uint32_t BestBuyOrderQtyTwo;    //委托量2
		uint32_t BestBuyOrderQtyThree;  //委托量3
		uint32_t BestBuyOrderQtyFour;   //委托量4
		uint32_t BestBuyOrderQtyFive;   //委托量5
		uint32_t BestBuyOrderQtySix;    //委托量6
		uint32_t BestBuyOrderQtySeven;  //委托量7
		uint32_t BestBuyOrderQtyEight;  //委托量8
		uint32_t BestBuyOrderQtyNine;   //委托量9
		uint32_t BestBuyOrderQtyTen;    //委托量10
		uint32_t BestSellOrderQtyOne;   //委托量1
		uint32_t BestSellOrderQtyTwo;   //委托量2
		uint32_t BestSellOrderQtyThree; //委托量3
		uint32_t BestSellOrderQtyFour;  //委托量4
		uint32_t BestSellOrderQtyFive;  //委托量5
		uint32_t BestSellOrderQtySix;   //委托量6
		uint32_t BestSellOrderQtySeven; //委托量7
		uint32_t BestSellOrderQtyEight; //委托量8
		uint32_t BestSellOrderQtyNine;  //委托量9
		uint32_t BestSellOrderQtyTen;   //委托量10
	};

	/*
	* 期权参数
	*/
	struct mds_option_param {
		double Delta;
		double Gamma;
		double Rho;
		double Theta;
		double Vega;
	};

	/*
	* 加权平均以及委托总量行情
	*/
	struct mds_order_statistic {
		uint32_t TotalBuyOrderNum;              //买委托总量
		uint32_t TotalSellOrderNum;             //卖委托总量
		double   WeightedAverageBuyOrderPrice;  //加权平均委买价格
		double   WeightedAverageSellOrderPrice; //加权平均委卖价格
	};

	/*
	* 分价位成交
	*/
	struct mds_match_price_qty {
		double   PriceOne;        //价格
		uint32_t PriceOneBOQty;   //买开数量
		uint32_t PriceOneBEQty;   //买平数量
		uint32_t PriceOneSOQty;   //卖开数量
		uint32_t PriceOneSEQty;   //卖平数量
		double   PriceTwo;        //价格
		uint32_t PriceTwoBOQty;   //买开数量
		uint32_t PriceTwoBEQty;   //买平数量
		uint32_t PriceTwoSOQty;   //卖开数量
		uint32_t PriceTwoSEQty;   //卖平数量
		double   PriceThree;      //价格
		uint32_t PriceThreeBOQty; //买开数量
		uint32_t PriceThreeBEQty; //买平数量
		uint32_t PriceThreeSOQty; //卖开数量
		uint32_t PriceThreeSEQty; //卖平数量
		double   PriceFour;       //价格
		uint32_t PriceFourBOQty;  //买开数量
		uint32_t PriceFourBEQty;  //买平数量
		uint32_t PriceFourSOQty;  //卖开数量
		uint32_t PriceFourSEQty;  //卖平数量
		double   PriceFive;       //价格
		uint32_t PriceFiveBOQty;  //买开数量
		uint32_t PriceFiveBEQty;  //买平数量
		uint32_t PriceFiveSOQty;  //卖开数量
		uint32_t PriceFiveSEQty;  //卖平数量
	};
	struct dcel2param_mds;
	struct mds_rtn_data {
		enum {
			e_succ = 0,//全部成功
			e_no_data_yet = 1, //服务端数据未就绪
			e_connect_svr_fail = 2,//连接服务端失败
			e_send_req_fail = 3,//发送数据失败
			e_recv_data_fail = 4,//接收数据失败
			e_invalid_rtn_data = 5,//接收到非法数据
			e_invalid_svr_addr = 6,//非法的服务地址
			e_create_socket_fail = 7,//创建socket失败
		} ErrNo;
		struct dcel2param_mds* Param1;
		struct dcel2param_mds* Param2;
	};
	enum parse_result_t {
		e_parse_fail = 0,
		e_parse_succ = 1,
		e_parse_succ_and_key_updated = 2,//当分析出key，或解析过程中，遇到不能解析的数据重新分析出新的key时，返回当前值
	};
#pragma pack(pop)

	typedef void (*fn_on_logmsg_t)(void* userData, const char* logmsg);
	typedef void (*fn_on_heartbeat_t)(void* userData, const msg_header* msgHeader, void* dataAttachedInfo);
	typedef void (*fn_on_best_t)(void* userData, const msg_header* msgHeader, const mds_contract_info* contractInfo, const mds_best* best, void* dataAttachedInfo);
	typedef void (*fn_on_arbi_best_t)(void* userData, const msg_header* msgHeader, const mds_contract_info* contractInfo, const mds_arbi_best* arbiBest, void* dataAttachedInfo);
	typedef void (*fn_on_deep_t)(void* userData, const msg_header* msgHeader, const mds_contract_info* contractInfo, const mds_deep* deep, void* dataAttachedInfo);
	typedef void (*fn_on_option_param_t)(void* userData, const msg_header* msgHeader, const mds_contract_info* contractInfo, const mds_option_param* optionParam, void* dataAttachedInfo);
	typedef void (*fn_on_ten_entrust_t)(void* userData, const msg_header* msgHeader, const mds_contract_info* contractInfo, const mds_ten_entrust* tenEntrust, void* dataAttachedInfo);
	typedef void (*fn_on_order_statistic_t)(void* userData, const msg_header* msgHeader, const mds_contract_info* contractInfo, const mds_order_statistic* orderStatistic, void* dataAttachedInfo);
	typedef void (*fn_on_match_price_qty_t)(void* userData, const msg_header* msgHeader, const mds_contract_info* contractInfo, const mds_match_price_qty* matchPriceQty, void* dataAttachedInfo);

	/*
	* 返回空的解密参数，在后续处理数据过程中(调用dcel2_mds_parse来解析)，尝试分析出解密参数，或已知密钥，调用dcel2_mds_update_key更新进去。
	* 注意：针对主备路的数据，应分别调用dcel2_mds_init_empty创建不同的dcel2param_mds实例，切记不要共用同一个实例！！！
	*/
	_DCEL2DECODE_MDS_API_ struct dcel2param_mds* dcel2_mds_init_empty();

	/*
	* 向服务端请求解密参数，用于后续调用dcel2_mds_parse来解析数据。
	* 注意：针对主备路的数据，创建不同的dcel2param_mds实例，切记不要共用同一个实例！！！
	* *svrAddr: 服务端地址，格式为udp://xxx.xxx.xxx.xxx:port，或tcp://xxx.xxx.xxx.xxx:port
	* *chnanel_id: 1表示主路; 2表示备路; 0表示同时请求主备路
	*/
	_DCEL2DECODE_MDS_API_ mds_rtn_data dcel2_mds_init_from_svr(const char* svrAddr/*in*/, int channel_id/*in*/);

	/*
	* 由从服务端获取的解密参数中，获取组播地址、源地址及端口的信息
	* param为从服务端获取的解密参数。
	* 返回true为成功，返回false为失败，通常原因是解密参数不是从服务端获取的，或者服务端返回的地址信息也是空的。
	*/
	_DCEL2DECODE_MDS_API_ bool dcel2_mds_get_addr_info(struct dcel2param_mds* param/*in*/, char group_ip[16]/*out*/, char source_ip[16]/*out*/, uint16_t* port/*out*/, int* channel_id/*out*/);

	/*
	* 由非空的解密参数中，获取原始的mode及key
	* param为非空的解密参数，mode及key为保存的结果
	* 返回值为true表示获取成功；返回false表示失败，通常原因可能是解密参数内部为空，比如尚未解析出正确的key
	*/
	_DCEL2DECODE_MDS_API_ bool dcel2_mds_get_key(struct dcel2param_mds* param/*in*/, int* mode/*out*/, uint8_t key[64]/*out*/);

	/*
	* 由其它处得到key信息，更新至已存在的(空或不空的均可)参数用，用于后续解析数据。
	* param为由dcel2_mds_init_empty创建的空的解密参数。
	*/
	_DCEL2DECODE_MDS_API_ void dcel2_mds_update_key(struct dcel2param_mds* param/*in_out*/, int mode/*in*/, const uint8_t key[64]/*in*/);

	/*
	* 销毁由dcel2_mds_init_empty、mds_load_from_file或mds_load_from_buffer创建的dcel2param_mds句柄，否则可能会造成内存泄露
	*/
	_DCEL2DECODE_MDS_API_ void dcel2_mds_destroy_param(struct dcel2param_mds* param/*in*/);

	/*
	* 设置on_logmsg回调函数（可选）; logmsg主要为了把内部解析数据时的一些信息反馈出来。
	*/
	_DCEL2DECODE_MDS_API_ void dcel2_mds_set_on_logmsg(struct dcel2param_mds* param/*in*/, fn_on_logmsg_t fn/*in*/, void* userData/*in*/);

	/*
	* 设置on_heartbeat回调函数（可选）
	*/
	_DCEL2DECODE_MDS_API_ void dcel2_mds_set_on_heartbeat(struct dcel2param_mds* param/*in*/, fn_on_heartbeat_t fn/*in*/, void* userData/*in*/);

	/*
	* 设置on_best回调函数（必须设置，否则可能崩溃）
	*/
	_DCEL2DECODE_MDS_API_ void dcel2_mds_set_on_best(struct dcel2param_mds* param/*in*/, fn_on_best_t fn/*in*/, void* userData/*in*/);

	/*
	* 设置on_arbi_best回调函数（必须设置，否则可能崩溃）
	*/
	_DCEL2DECODE_MDS_API_ void dcel2_mds_set_on_arbi_best(struct dcel2param_mds* param/*in*/, fn_on_arbi_best_t fn/*in*/, void* userData/*in*/);

	/*
	* 设置on_deep回调函数（必须设置，否则可能崩溃）
	*/
	_DCEL2DECODE_MDS_API_ void dcel2_mds_set_on_deep(struct dcel2param_mds* param/*in*/, fn_on_deep_t fn/*in*/, void* userData/*in*/);

	/*
	* 设置on_option_param回调函数（可选）
	*/
	_DCEL2DECODE_MDS_API_ void dcel2_mds_set_on_option_param(struct dcel2param_mds* param/*in*/, fn_on_option_param_t fn/*in*/, void* userData/*in*/);

	/*
	* 设置on_ten_entrust回调函数（可选）
	*/
	_DCEL2DECODE_MDS_API_ void dcel2_mds_set_on_ten_entrust(struct dcel2param_mds* param/*in*/, fn_on_ten_entrust_t fn/*in*/, void* userData/*in*/);

	/*
	* 设置on_order_statistic回调函数（可选）
	*/
	_DCEL2DECODE_MDS_API_ void dcel2_mds_set_on_order_statistic(struct dcel2param_mds* param/*in*/, fn_on_order_statistic_t fn/*in*/, void* userData/*in*/);

	/*
	* 设置on_match_price_qty回调函数（可选）
	*/
	_DCEL2DECODE_MDS_API_ void dcel2_mds_set_on_match_price_qty(struct dcel2param_mds* param/*in*/, fn_on_match_price_qty_t fn/*in*/, void* userData/*in*/);

	/*
	* 尝试分析密钥并解析数据：解析后的数据由dcel2_mds_set_on_xxxx所设置的回调函数中通知。
	* data:待解析的数据
	* dataLen: 待解析数据的长度
	* dataAttachedInfo: 与数据包关联的其它数据。该内容原样在回调函数中传回，不作任何处理。
	* 注意：主备数据的param应该使用不同的实例，切记不要混用同一个dcel2param_mds!!!
	*/
	_DCEL2DECODE_MDS_API_ parse_result_t dcel2_mds_parse(struct dcel2param_mds* param/*in*/, const void* data/*in&modifiable*/, uint32_t dataLen/*in*/, void* dataAttachedInfo/*in*/);

#ifdef __cplusplus
};
#endif

#endif //__DCEL2DECODE_MDS_H__