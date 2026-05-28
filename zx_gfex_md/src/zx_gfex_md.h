#pragma once

#include <string> 
#include <set>
#include <atomic>
#include <thread>

#include <utility>  


#include <algorithm>

#include "fb_md_plugin_api.h"
#include "common.h"

#include <exanic/exanic.h>
#include <exanic/fifo_rx.h>
#include <exanic/config.h>

#include "inst_map.h"

#include "udp_receiver.h"


#include "macro.h"

#include "ringbuffer.h"

#include "exanic_receiver.h"


#define FLOAT64_NAN std::numeric_limits<double>::quiet_NaN()


#define PRICE_NROMAL       0
#define PRICE_UP_LIMIT     1
#define PRICE_DOWN_LIMIT   2





using fb_market_data_t = cffex::fb::api::market_data_entity;

#pragma pack(push, 1)

struct MarketDataField
{
    int packetLen;//报文长度
    unsigned char versionNo;//版本序号
    int       updateTime;//修改时间
    char   exchangeID[3];//交易所
    char   instrumentID[30];//合约代码
    bool   stopFlag;//停牌标识
    char   statusLatestPrice;//
    double latestPrice;//最新价
    char   statusMatchAmount;//
    int    matchAmount;//成交量
    char   statusPositionAmount;//
    int    positionAmount;//持仓量
    char   statusHighestPrice;//
    double highestPrice;//最高价    
    char   statusLowestPrice;//
    double lowestPrice;//最低价
    char   statusBuyPrice1;//
    double buyPrice1;//申买价1
    char   statusSellPrice1;//
    double sellPrice1;//申卖价1
    char   statusBuyAmount1;//
    int    buyAmount1;//申买量1
    char   statusSellAmount1;//
    int    sellAmount1;//申卖量1
    char   statusMatchMoney;//
    double macthMoney;//成交金额
    char   statusOpenPrice;//
    double openPrice;//开盘价
    char   statusAvgPrice;//
    double avgPrice;//当日均价
};










struct Lv2_OB_Quote
{
    int32_t PacketId;
    int32_t PacketLen;
    char    Date[9];
    char    Symbol[0x47];
    double  LastPrice;
    double  HighPrice;
    double  LowPrice;
    int32_t _unk1;
    int32_t Volume;
    double  Turnover;
    int32_t _lastOpenInterest;
    int32_t OpenInterest;
    int32_t _unk2;
    double  _clearPrice;
    int32_t  _unk3;
    int32_t  _unk4;
    int32_t  _unk5;
    int32_t  _unk6;
    double  PriceCeil;
    double  PriceFloor;
    double  _lastClear;
    double  _lastClose;

	// offset 0xc4 ?
    double  _bestBid;
    int32_t _bestBidQ;
    int32_t _bestBidImplQ;
    double  _bestAsk;
    int32_t _bestAskQ;
    int32_t _bestAskImplQ;
    double  _avgPrice;
    char    Time[16];
    double  _openPrice;
    double  _closePrice;
    int32_t _tableCount;

    struct Line
    {
        double  Price;
        int32_t Volume;
        int32_t _VolumeImpl;
        int8_t  _align[16];
    };

    Line Bid[5];
    Line Ask[5];

    uint8_t _align[84];



};

struct L2Quote_212
{
    char type;
    char Symbol[0x47];
    char res[12];
    double     bp1;
    uint32_t   bv1;
    uint32_t v2;
    uint32_t v3;
    uint64_t v4;
    uint64_t v5;
    char res2[12];
    double     ap1;
    uint32_t   av1;

};

struct L2Quote_128
{
    uint32_t type;
    char Symbol[0x47];




};



struct L2Quote_224
{
    uint32_t type;
    char     Symbol[0x47];
    char     res[13];
    double   price;


};

struct L2Quote_112
{
    uint32_t type;
    char Symbol[0x47];
    char res[13];
    double     price;
    int32_t Volume;



};



#pragma pack(pop)




namespace GFEX
{
	static const int g_iMaxFrameSize = 2048;

	//�Զ�����Ϣ
	const int32_t MsgZx_BestDeep = 1;
	const int32_t MsgZx_ArbBestDeep = 2;
	const int32_t MsgZx_TenEntrust = 3;
	const int32_t MsgZx_Real = 4;
	const int32_t MsgZx_OrderStatistic = 5;

	const int32_t MsgZx_MatchPriceQty = 6;


#pragma pack(push, 1)
	struct Zx_PacketHead
	{
		uint16_t iPktSize;				//����Zx_Packetͷ��payload����
		uint32_t iReserved;
	};

	struct Zx_MsgHead
	{
		uint32_t iSequence;				//����ת��ϵͳsequence�����鲥ͨ����1���������������ж��Ƿ񶪰�
		uint16_t iMsgSize;
		uint16_t iMsgType;
	};

	//�������_����0x31
	struct Zx_BestDeep
	{
		uint32_t iSequence;	//������ԭʼsequence
		char arrInstrument[23];
		char arrGenTime[12];

		double dLastPrice;
		double dHighPrice;
		double dLowPrice;
		int32_t iLastMatchQty;

		int32_t iMatchTotQty;
		double dTurnover;
		int32_t iInitOpenInterest;
		int32_t iOpenInterest;
		int32_t iInterestChg;

		double dClearPrice;
		double dLastClearPrice;
		double dClosePrice;
		double dLastClosePrice;
		double dOpenPrice;

		double dLifeLow;
		double dLifeHigh;
		double dRiseLimit;
		double dFallLimit;
		double dAvePrice;


		double dBidPrice1;
		int32_t iBidVolume1;
		int32_t iBidImplyVolume1;
		double dBidPrice2;
		int32_t iBidVolume2;
		int32_t iBidImplyVolume2;

		double dBidPrice3;
		int32_t iBidVolume3;
		int32_t iBidImplyVolume3;
		double dBidPrice4;
		int32_t iBidVolume4;
		int32_t iBidImplyVolume4;

		double dBidPrice5;
		int32_t iBidVolume5;
		int32_t iBidImplyVolume5;

		double dAskPrice1;
		int32_t iAskVolume1;
		int32_t iAskImplyVolume1;
		double dAskPrice2;
		int32_t iAskVolume2;
		int32_t iAskImplyVolume2;

		double dAskPrice3;
		int32_t iAskVolume3;
		int32_t iAskImplyVolume3;
		double dAskPrice4;
		int32_t iAskVolume4;
		int32_t iAskImplyVolume4;

		double dAskPrice5;
		int32_t iAskVolume5;
		int32_t iAskImplyVolume5;
	};

	//�������_���0x32
	struct Zx_ArbBestDeep
	{
		uint32_t iSequence;	//������ԭʼsequence
		char arrInstrument[23];
		char arrGenTime[12];
		double dLastPrice;
		double dLowPrice;

		double dHighPrice;
		double dLifeLow;
		double dLifeHigh;
		double dRiseLimit;
		double dFallLimit;

		double dBidPrice1;
		int32_t iBidVolume1;
		int32_t iBidImplyVolume1;
		double dBidPrice2;
		int32_t iBidVolume2;
		int32_t iBidImplyVolume2;

		double dBidPrice3;
		int32_t iBidVolume3;
		int32_t iBidImplyVolume3;
		double dBidPrice4;
		int32_t iBidVolume4;
		int32_t iBidImplyVolume4;

		double dBidPrice5;
		int32_t iBidVolume5;
		int32_t iBidImplyVolume5;

		double dAskPrice1;
		int32_t iAskVolume1;
		int32_t iAskImplyVolume1;
		double dAskPrice2;
		int32_t iAskVolume2;
		int32_t iAskImplyVolume2;

		double dAskPrice3;
		int32_t iAskVolume3;
		int32_t iAskImplyVolume3;
		double dAskPrice4;
		int32_t iAskVolume4;
		int32_t iAskImplyVolume4;

		double dAskPrice5;
		int32_t iAskVolume5;
		int32_t iAskImplyVolume5;
	};

	struct Zx_TenEntrust//0x34
	{
		uint32_t iSequence;	//������ԭʼsequence
		char arrInstrument[23];
		char arrGenTime[12];
		double dBestBuyOrderPrice;

		int32_t iBestBuyOrderQtyOne;
		int32_t iBestBuyOrderQtyTwo;
		int32_t iBestBuyOrderQtyThree;
		int32_t iBestBuyOrderQtyFour;
		int32_t iBestBuyOrderQtyFive;

		int32_t iBestBuyOrderQtySix;
		int32_t iBestBuyOrderQtySeven;
		int32_t iBestBuyOrderQtyEight;
		int32_t iBestBuyOrderQtyNine;
		int32_t iBestBuyOrderQtyTen;

		double dBestSellOrderPrice;

		int32_t iBestSellOrderQtyOne;
		int32_t iBestSellOrderQtyTwo;
		int32_t iBestSellOrderQtyThree;
		int32_t iBestSellOrderQtyFour;
		int32_t iBestSellOrderQtyFive;

		int32_t iBestSellOrderQtySix;
		int32_t iBestSellOrderQtySeven;
		int32_t iBestSellOrderQtyEight;
		int32_t iBestSellOrderQtyNine;
		int32_t iBestSellOrderQtyTen;
	};

	struct Zx_OrderStatistic//0x36
	{
		uint32_t iSequence;	//������ԭʼsequence
		char arrInstrument[23];

		int32_t iTotalBuyOrderNum;
		int32_t iTotalSellOrderNum;
		double dWeightedAverageBuyOrderPrice;
		double dWeightedAverageSellOrderPrice;
	};

	struct Zx_Real//0x35
	{
		uint32_t iSequence;	//������ԭʼsequence
		char arrInstrument[23];
		double dRealTimePrice;
	};

	struct Zx_MatchPriceQty//0x37
	{
		uint32_t iSequence;	//������ԭʼsequence
		char arrInstrument[23];

		double dPriceOne;
		int32_t iPriceOneBOQty;
		int32_t iPriceOneBEQty;
		int32_t iPriceOneSOQty;
		int32_t iPriceOneSEQty;

		double dPriceTwo;
		int32_t iPriceTwoBOQty;
		int32_t iPriceTwoBEQty;
		int32_t iPriceTwoSOQty;
		int32_t iPriceTwoSEQty;

		double dPriceThree;
		int32_t iPriceThreeBOQty;
		int32_t iPriceThreeBEQty;
		int32_t iPriceThreeSOQty;
		int32_t iPriceThreeSEQty;

		double dPriceFour;
		int32_t iPriceFourBOQty;
		int32_t iPriceFourBEQty;
		int32_t iPriceFourSOQty;
		int32_t iPriceFourSEQty;

		double dPriceFive;
		int32_t iPriceFiveBOQty;
		int32_t iPriceFiveBEQty;
		int32_t iPriceFiveSOQty;
		int32_t iPriceFiveSEQty;
	};

#pragma pack(pop)
}




inline uint64_t time_str_to_utc_s(const char *time_str){
    // 直接访问字符数组
    const char* h  = time_str;
    const char* m  = time_str + 3;
    const char* s  = time_str + 6;
    const char* ms = time_str + 9;

    int hours        = (h[0] - '0') * 10 + (h[1] - '0');
    int minutes      = (m[0] - '0') * 10 + (m[1] - '0');
    int seconds      = (s[0] - '0') * 10 + (s[1] - '0');
    int milliseconds = (ms[0] - '0') * 100 +(ms[1] - '0') * 10 +(ms[2] - '0');

    const int64_t HOURS_TO_MS = 3600000LL;
    const int64_t MIN_TO_MS   = 60000LL;
    const int64_t SEC_TO_MS   = 1000LL;
    return (hours * HOURS_TO_MS) +(minutes * MIN_TO_MS) + (seconds * SEC_TO_MS) +milliseconds;
}



struct IpPort
{
    uint32_t    ip;   
    uint16_t    port;
    IpPort(uint32_t a, uint16_t b):ip(a),port(b){}
};

struct MulticastConfig{
    std::string     ifname;

    std::string     group_ip;
    uint16_t        group_port;
    uint32_t        net_group_ip; 
    uint16_t        net_group_port; 
};

struct TCPConfig{
    std::string     ifname;

    std::string     ip;   
    uint16_t        port;
    uint32_t        net_ip;   
    uint16_t        net_port;
};


struct Config_t{
    MulticastConfig      multiconfig;
    TCPConfig            tcpconfig;
    std::vector<int>     cpu_id;
    std::string          filter_path;
    std::string          zx_ini;
};

//lc2601-p-234000



enum COM_RESULT {
    COM_RESULT_OK = 0,
    COM_RESULT_TIME_ERROR   = 1,     // 时间倒流
    COM_RESULT_VOL_ERROR    = 2,     // vol倒流
    COM_RESULT_REPEAT_L1    = 3,     // 无用的Lv1
    COM_RESULT_REPEAT_L2    = 4,     // 无用的Lv2
};

#define RING_BUFFER_SIZE 4096

#define IOPV_TAG_LV1     100000
#define IOPV_TAG_LV2     200000

struct QuotaInfo{
    uint64_t time;
    uint64_t volume;
    double  open;
    double  high;
    double  low;    
    double  upper;
    double  lower;
    int32_t last_openinterest;
    int32_t openinterest;    
    double  last_settle_price;
    double  settle_price;

    double   ap1;
    double   bp1;
    uint32_t av1;
    uint32_t bv1;

    uint32_t level;
};

class GFEXQuota : public cffex::fb::api::fb_md_plugin_api{
public:
    GFEXQuota();
    ~GFEXQuota();

private:
    inline void LoadJsonCfg();

    inline bool level2_belongTo(uint32_t ip, uint16_t port);
    inline bool level1_belongTo(uint32_t ip, uint16_t port);

    void ProcessMsg(int32_t cpu_id);
    void DispatchMessage(const Slot& slot);

    void DecodeMsg(const char* data, int32_t size);

    void OnData_lv2(const char* data, size_t size);

    inline int32_t IsValidQuota(QuotaInfo *cache, uint64_t extime, uint64_t vol, uint32_t level, double ap1, double bp1, uint32_t av1, uint32_t bv1);

    void OnData_lv1(const char* data, size_t size);

    static void UdpHandler(void *ctx, const char* data, size_t size);
    // static void ExanicHandler(void *ctx, const char* data, size_t size);


public:           //override  cffex::fb::api::fb_i_md_spi
    int  init() override; 
    void release() override;
    void connect() override;
    void subscribe_inst(const std::string &instrument_id, uint8_t exchange_id) override;
    void register_spi(cffex::fb::api::fb_i_md_spi *spi) override;

private:
    Config_t                   m_config;

    std::shared_ptr<MulticastReceiver>          m_udp_receiver;

    // std::shared_ptr<ExanicReceiver>             m_exanic_receiver;

    InstMap<QuotaInfo>                              m_quota_cache; 

    LockFreeRingBuffer        *m_ringbuffer;
    std::thread               *m_dispatcher;          


    std::set<std::string> m_inst_set;


    uint64_t m_l1_time_err_sum = 0;
    uint64_t m_l1_vol_err_sum = 0;
    uint64_t m_l1_useless_lv1_sum = 0;

    uint64_t m_l2_time_err_sum = 0;
    uint64_t m_l2_vol_err_sum = 0;


    uint64_t m_l2_apbp_err_sum = 0;

    //febao
    cffex::fb::api::fb_i_md_spi                      *m_fb_spi;
    fb_market_data_t                                 *m_fb_md;


	unsigned char m_arrBuf[GFEX::g_iMaxFrameSize] = { 0 };
	int m_iLen_packetHead = sizeof(GFEX::Zx_PacketHead);
	int m_iLen_msgHead = sizeof(GFEX::Zx_MsgHead);

};
