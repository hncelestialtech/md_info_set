#pragma once

#define MD_TYPE_SH_OFFSET 0x00010000U	 // 上交所的行情类型偏移
#define MD_TYPE_SH_L2 0x00010002U		 // 上交所L2快照 0x00010002
#define MD_TYPE_SH_L2_ZK 0x00010003U	 // 上交所L2网关指数行情 0x00010003
#define MD_TYPE_SH_ZC 0x00010005U		 // 上交所L2逐笔成交 0x00010005
#define MD_TYPE_SH_ZW 0x00010008U		 // 上交所L2逐笔委托 0x00010008
#define MD_TYPE_SH_L1_OPTION 0x00010007U // 上交所L1网关期权快照 0x00010007
#define MD_TYPE_SHZQ_L2 0x0001000AU		 // 上交所新债券L2快照 0x0001000A
#define MD_TYPE_SHZQ_ZB 0x0001000BU		 // 上交所新债券逐笔 0x0001000B
#define MD_TYPE_SHMERGE_ZB 0x0001000CU   // 上交所新逐笔合并 0x0001000C
#pragma pack(push, 1)

struct Lev2WholeData
{
	int msg_type;						   // 消息类型
	int msg_len;						   // 消息长度
	char security_id[8];				   // 产品代码
	unsigned int time_stamp;			   // 最新订单时间, e.g.103020表示10:30:20
	unsigned int pre_close_price;		   // 昨收盘价,除以1000
	unsigned int open_price;			   // 开盘价,除以1000
	unsigned int high_price;			   // 最高价,除以1000
	unsigned int low_price;				   // 最低价,除以1000
	unsigned int match_price;			   // 最新价,除以1000
	unsigned int num_trade;				   // 成交笔数
	unsigned long long total_volume_trade; // 成交总量,除以1000
	unsigned long long total_value_trade;  // 成交总金额,除以100000
	unsigned long long total_bid_qty;	   // 委托买入总量,除以1000
	unsigned long long total_offer_qty;	   // 委托卖出总量,除以1000
	unsigned int weighted_avg_bid_price;   // 加权平均委买价格,除以1000
	unsigned int weighted_avg_offer_price; // 加权平均委卖价格,除以1000
	char trading_phase_code[8];			   // 当前产品状态
	unsigned int reserve1;				   // 保留字段

	unsigned int bid_price[10];			 // 申买价,除以1000
	unsigned long long bid_qty[10];		 // 申买量,除以1000
	unsigned int offer_price[10];		 // 申卖价,除以1000
	unsigned long long offer_qty[10];	 // 申卖量,除以1000
	unsigned int bid_one_num_order;		 // 买一总委托笔数
	unsigned int bid_one_pos;			 // 买一揭示委托笔数,不大于50
	unsigned int offer_one_num_order;	 // 卖一总委托笔数
	unsigned int offer_one_pos;			 // 卖一揭示委托笔数,不大于50
	unsigned int bid_order[50];			 // 买一揭示订单明细,除以1000
	unsigned int offer_order[50];		 // 卖一揭示订单明细,除以1000
	unsigned long long high_limit_price; // 涨停价,除以10000
	unsigned long long low_limit_price;	 // 跌停价,除以10000
};

struct Lev2Index
{
	int msg_type;			   // 消息类型
	int msg_len;			   // 消息长度
	char trade_security_id[8]; // 产品代码

	unsigned int index_data_time_stamp;	   // 行情时间, e.g.103020即10:30:20
	unsigned long long index_pre_close;	   // 前收盘指数,除以100000
	unsigned long long index_open;		   // 今开盘指数,除以100000
	unsigned long long index_turnover;	   // 成交金额,除以10
	unsigned long long index_high;		   // 最高指数,除以100000
	unsigned long long index_low;		   // 最低指数,除以100000
	unsigned long long index_last;		   // 最新指数,除以100000
	unsigned int index_trade_time;		   // 成交时间, e.g.10302019表示10:30:20.19（百分之一秒）
	unsigned long long index_total_volume; // 成交数量,除以100000
	unsigned long long index_close;		   // 今收盘指数,除以100000
	unsigned long long reserve1;		   // 保留字段
};

struct Lev2Trade
{
	int msg_type;			   // 消息类型
	int msg_len;			   // 消息长度
	char trade_security_id[8]; // 产品代码

	unsigned int trade_channel;		  // 成交通道
	unsigned int trade_index;		  // 成交序号
	unsigned int trade_time;		  // 成交时间, e.g.10302019表示10:30:20.19（百分之一秒）
	unsigned int trade_price;		  // 成交价格,除以1000
	unsigned long long trade_qty;	  // 成交数量,除以1000
	unsigned long long trade_money;	  // 成交金额,除以100000
	unsigned long long trade_buy_no;  // 买方订单号
	unsigned long long trade_sell_no; // 卖方订单号
	char trade_bs_flag;				  // 内外盘标志
	char reserve1[3];				  // 保留字段
	long long trade_biz_index;		  // 业务序列号，与委托统一编号，从1开始，按channel连续
	char reserve2[4];				  // 保留字段
};

struct Lev2Order
{
	int msg_type;			   // 消息类型
	int msg_len;			   // 消息长度
	char order_security_id[8]; // 产品代码

	int order_channel;		   // 通道
	int order_index;		   // 委托序号，从1开始，按channel连续
	int order_time;			   // 委托时间，e.g.14302506表示14:30:25.06（百分之一秒）
	char order_type;		   // 订单类型，A-委托订单增加，D-委托订单删除
	char reserve1[3];		   // 保留字段
	long long order_no;		   // 原始订单号
	int order_price;		   // 委托价格（元）
	long long order_balance;   // 剩余委托量
	char order_bs_flag;		   // 委托标识，B-买单，S-卖单
	char reserve2[3];		   // 保留字段
	long long order_biz_index; // 业务序列号，与成交统一编号，从1开始，按channel连续
};

//----------------new bond-------------------------//
struct Lev2BondWholeData
{
	int msg_type;							   // 消息类型
	int msg_len;							   // 消息长度
	char security_id[8];					   // 产品代码
	unsigned int time_stamp;				   // 最新订单时间, e.g.103025009表示10:30:25.009（毫秒）
	unsigned int pre_close_price;			   // 昨收盘价,除以1000
	unsigned int open_price;				   // 开盘价,除以1000
	unsigned int high_price;				   // 最高价,除以1000
	unsigned int low_price;					   // 最低价,除以1000
	unsigned int match_price;				   // 最新价,除以1000
	unsigned int num_trade;					   // 成交笔数
	unsigned long long total_volume_trade;	   // 成交总量,除以1000
	unsigned long long total_value_trade;	   // 成交总金额,除以100000
	unsigned long long total_bid_qty;		   // 委托买入总量,除以1000
	unsigned long long total_offer_qty;		   // 委托卖出总量,除以1000
	unsigned int alt_weighted_avg_bid_price;   // 债券加权平均委买价格,除以1000
	unsigned int alt_weighted_avg_offer_price; // 债券加权平均委卖价格,除以1000
	char trading_phase_code[8];				   // 当前产品状态
	unsigned int reserve1;					   // 保留字段

	unsigned int bid_price[10];		  // 申买价,除以1000
	unsigned long long bid_qty[10];	  // 申买量,除以1000
	unsigned int offer_price[10];	  // 申卖价,除以1000
	unsigned long long offer_qty[10]; // 申卖量,除以1000
	unsigned int bid_one_num_order;	  // 买一总委托笔数
	unsigned int bid_one_pos;		  // 买一揭示委托笔数,不大于50
	unsigned int offer_one_num_order; // 卖一总委托笔数
	unsigned int offer_one_pos;		  // 卖一揭示委托笔数,不大于50
	unsigned int bid_order[50];		  // 买一揭示订单明细,除以1000
	unsigned int offer_order[50];	  // 卖一揭示订单明细,除以1000
};

struct Lev2BondTick
{
	int msg_type;			  // 消息类型
	int msg_len;			  // 消息长度
	char tick_security_id[8]; // 产品代码

	int tick_channel; // 通道
	int tick_index;	  // 逐笔序号，从1开始，按tick_channel连续
	int tick_time;	  // 订单或成交时间，e.g.103025009表示10:30:25.009（毫秒）
	char tick_type;	  // 类型，A – 新增委托订单，D – 删除委托订单，S – 产品状态订单，T – 成交
	char reserve1[3]; // 保留字段

	long long tick_buy_no;	// 买房订单
	long long tick_sell_no; // 卖方订单
	int tick_price;			// 价格，除以1000
	long long tick_qty;		// 数量（手），除以1000
	long long tick_money;	// 成交金额，除以100000
	char tick_bs_flag[8];	// 标志，若为新增或删除委托订单（A/D）：B – 买单；S – 卖单；
						  // 若为产品状态订单（S）： ADD – 产品未上市；START – 启动；OCALL – 开市集合竞价；TRADE – 连续自动撮合；SUSP – 停牌；CLOSE – 闭市；ENDTR – 交易结束；
						  //  若为成交（T）：B – 外盘，主动买；S – 内盘，主动卖；N – 未知；
	char reserve2[4]; // 保留字段
};

struct Lev1Option
{
	int msg_type;				   // 消息类型
	int msg_len;				   // 消息长度
	char security_id[8];		   // 产品代码
	int data_timestamp;			   // 时间戳 HHMMSSmmm
	long long pre_settle_price;	   // 昨日结算价,除以100000
	long long today_settle_price;  // 今日结算价
	long long open_price;		   // 今开盘价，除以100000
	long long high_price;		   // 最高价，除以100000
	long long low_price;		   // 最低价，除以100000
	long long last_price;		   // 现价，除以100000
	long long auction_price;	   // 动态参考价格，除以100000
	long long auction_qty;		   // 虚拟匹配数量
	long long total_long_position; // 当前合约未平仓数量
	long long bid_qty[5];		   // 买盘数量
	long long bid_price[5];		   // 买盘价格,除以100000
	long long offer_qty[5];		   // 卖盘数量
	long long offer_price[5];	   // 卖盘价格，除以100000
	long long total_volume_trade;  // 成交数量
	long long total_value_trade;   // 成交金额,除以100
	char trading_phase_code[8];	   // 成交阶段代码,交易时间段由4位扩至8位
								//    期权交易状态，取值范围如下：
								// 该字段为4位字符串，左起每位表示特定的含义，无定义则填空格。
								//?	第1位：‘S’表示启动（开市前）时段，‘C’表示集合竞价时段，‘T’表示连续交易时段，‘B’表示休市时段，‘E’表示闭市时段，‘V’表示波动性中断，‘P’表示临时停牌、‘U’收盘集合竞价。‘M’表示可恢复交易的熔断（盘中集合竞价）,‘N’表示不可恢复交易的熔断（暂停交易至闭市）
								//?	第2位：‘0’表示未连续停牌，‘1’表示连续停牌。（预留，暂填空格）
								//?	第3位：‘0’表示不限制开仓，‘1’表示限制备兑开仓，‘2’表示卖出开仓，‘3’表示限制卖出开仓、备兑开仓，‘4’表示限制买入开仓，‘5’表示限制买入开仓、备兑开仓，‘6’表示限制买入开仓、卖出开仓，‘7’表示限制买入开仓、卖出开仓、备兑开仓
								//?	第4位：‘0’表示此产品在当前时段不接受进行新订单申报，‘1’ 表示此产品在当前时段可接受进行新订单申报。
								//?	第5位至第8位，预留（暂填空格）
	char transact_time_only[12]; // 暂无
};

struct Lev2MergeTick
{
 int    msg_type;				                  //消息类型 
 int    msg_len;                                  //消息长度 
 char   tick_security_id[8];                      //产品代码 

 int    tick_channel;                             //通道
 long long    tick_biz_index;					  //逐笔序号，从1开始，按tick_channel连续
 int    tick_time;								  //订单或成交时间，e.g.14302506表示14:30:25.06（百分之一秒）
 char   tick_type;								  //类型，A – 新增委托订单，D – 删除委托订单，S – 产品状态订单，T – 成交
 char   reserve1[3];							  //保留字段

 long long  tick_buy_no;						  //买房订单
 long long  tick_sell_no;						  //卖方订单
 int    tick_price;								  //价格，除以1000
 long long  tick_qty;							  //数量（手），除以1000
 long long  tick_money;							  //对于新增委托，已成交的委托数量（精度为三位，除以1000）;对于成交，成交金额（单位为元，精度为五位，除以100000）
 char   tick_bs_flag[8];						  //标志，若为新增或删除委托订单（A/D）：B – 买单；S – 卖单；
 												  //若为产品状态订单（S）： START – 启动；OCALL – 开市集合竞价；TRADE – 连续自动撮合；SUSP – 停牌；CCALL-收盘集合竞价；CLOSE – 闭市；ENDTR – 交易结束；
 												  // 若为成交（T）：B – 外盘，主动买；S – 内盘，主动卖；N – 未知；
};

#pragma pack(pop)