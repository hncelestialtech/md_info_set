/**
 * 上交、深交行情回调接口类
 **/
#ifndef __XELE_MD_SPI_H__
#define __XELE_MD_SPI_H__
#include "SseXeleMdStruct.h"
#include "SzseXeleMdStruct.h"

/**
 * 上交行情回调接口类
 **/
class XeleMdSseSpi
{
public:
	virtual ~XeleMdSseSpi() {}

	/*
	*上交所L2回调函数
	*/
	//上交L2股票快照
	virtual void onMarketDataSnapshotSse(MarketDataSnapshotSse* p) {};

	//上交L2订单详情
	virtual void onBestOrdersSse(BestOrdersSse* p) {};

	//上交L2指数
	virtual void onIndexSse(IndexSse* p) {};

	//上交股票盘后逐笔成交
	virtual void onAfterTradeSse(AfterTradeSse* p) {};

	//上交股票盘后快照
	virtual void onAfterSnapshotSse(AfterSnapshotSse* p) {};

	//上交L2债券快照
	virtual void onBondSnapshotSse(BondSnapshotSse* p) {};

	//上交债券逐笔
	virtual void onBondTickSse(BondTickSse* p) {};

	//上交逐笔合并
	virtual void onTickMergeSse(TickMergeSse* p) {};

	//上交EFT快照
	virtual void onETFSnapshotSse(ETFSnapshotSse* p) {};

	//上交静态行情
	virtual void onStaticInfoSse(StaticInfoSse* p) {};

	//上交逐笔转快照行情（订单簿）
	virtual void onMarketDataTreeSnapSse(MarketDataTreeSnapSse* p) {};
	
	//上交债券逐笔转快照行情（订单簿）
	virtual void onBondTreeSnapSse(MarketDataTreeSnapSse* p) {};

	/*
	*上交所L1回调函数
	*/
	//上交L1指数
	virtual void onIndexSseL1(IndexSseL1* p) {};

	//上交L1股票快照
	virtual void onStockSnapSseL1(StockBondSnapSseL1* p) {};

	//上交L1债券分销快照，对应原始行情类别为MD003
	virtual void onBondSnapSseL1(StockBondSnapSseL1* p) {};

	//上交L1基金快照
	virtual void onFundSseL1(FundSseL1* p) {};
	
	//上交L1 IOPV
	virtual void onIOPVSseL1(FundSseL1* p) {};

	//上交L1期权快照
	virtual void onOptionSseL1(OptionSseL1* p) {};

	//上交L1国债快照
	virtual void onNationalDebtSseL1(NationalDebtSseL1* p) {};

	//上交L1盘后固定价格
	virtual void onAfterTradeSseL1(AfterTradeSseL1* p) {};

	//上交L1债券快照，对应原始行情类别为MD201
	virtual void onBond201SnapSseL1(StockBondSnapSseL1* p) {};
};


/**
 * 深交行情回调接口类
 **/
class XeleMdSzseSpi
{
public:
	virtual ~XeleMdSzseSpi() {}
	
	//深交股票L2快照
	virtual void onMarketDataSnapshotSz(MarketDataSnapshotSz* p) {};

	//深交逐笔转快照行情（订单簿）
	virtual void onMarketDataTreeSnapSz(MarketDataTreeSnapSz* p) {};

	//深交订单详情
	virtual void onBestOrdersSz(BestOrdersSz* p) {};

	//深交指数行情
	virtual void onIndexSz(IndexSz* p) {};

	//深交逐笔成交
	virtual void onTradeSz(TradeSz* p) {};

	//深交逐笔委托
	virtual void onOrderSz(OrderSz* p) {};

	//深交股票L2盘后快照
	virtual void onAfterSnapshotSz(AfterSnapshotSz* p) {};

	//深交基金实时参考值
	virtual void onIOPVSz(IOPVSnapshotSz* p) {};

	//深交盘后大宗成交快照
	virtual void onBlockTradeSz(BlockTradeSz* p) {};

	//深交债券L2快照
	virtual void onBondSnapshotSz(BondSnapshotSz* p) {};

	//深交债券逐笔转快照行情（订单簿）
	virtual void onBondTreeSnapSz(MarketDataTreeSnapSz* p) {};

	//深交债券订单详情
	virtual void onBondBestOrdersSz(BondBestOrdersSz* p) {};

	//深交债券逐笔成交
	virtual void onBondTradeSz(BondTradeSz* p) {};

	//深交债券大额逐笔成交
	virtual void onBondBlockTradeSz(BondBlockTradeSz* p) {};

	//深交债券竞买逐笔成交
	virtual void onBondBidTradeSz(BondBidTradeSz* p) {};

	//深交债券逐笔委托
	virtual void onBondOrderSz(BondOrderSz* p) {};

	//深交债券大额逐笔委托
	virtual void onBondBlockOrderSz(BondBlockOrderSz* p) {};

	//深交债券竞买逐笔委托
	virtual void onBondBidOrderSz(BondBidOrderSz* p) {};
	
	//深交股票L1快照
	virtual void onL1MarketDataSnapSz(L1MarketDataSnapSz* p) {};
	
	//深交债券L1快照
	virtual void onL1BondSnapSz(L1BondSnapSz* p) {};
};

#endif