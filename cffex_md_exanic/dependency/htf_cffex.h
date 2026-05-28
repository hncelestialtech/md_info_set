#ifndef __HTF_CFFEX_H__
#define __HTF_CFFEX_H__

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define STATIC_FORCE_INLINE \
        static inline __attribute__((always_inline))

#define ROUND_UP(p, align) (((p)+(align)-1u) & ~((align)-1u))

#pragma pack(push, 1)
typedef struct CFFEXIncQuotaData{
    char             Padding1[2];
    char             InstrumentID[31];
    char             Padding2[7];   
    char             UpdateTime[9];  
    unsigned int     UpdateMillisec; 
    char             Padding3[3];    
    double           OpenPrice;      
    double           HighestPrice;
    double           LowestPrice;
    double           ClosePrice;
    double           UpperlimitPrice;
    double           LowerlimitPrice;
    double           SettlementPrice;
    double           CurrdeltaPrice;
    double           LastPrice;
    unsigned int     Volume;
    char             Padding4[4];
    double           Turnover;
    double           OpenInterest;
    double           BidPrice1;
    double           AskPrice1;
    unsigned int     BidVolume1;
    unsigned int     AskVolume1;
    double           BidPrice2;
    double           BidPrice3;
    unsigned int     BidVolume2;
    unsigned int     BidVolume3;
    double           AskPrice2;
    double           AskPrice3;
    unsigned int     AskVolume2;
    unsigned int     AskVolume3;
    double           BidPrice4;
    double           BidPrice5;
    unsigned int     BidVolume4;
    unsigned int     BidVolume5;
    double           AskPrice4;
    double           AskPrice5;
    unsigned int     AskVolume4;
    unsigned int     AskVolume5;
    double           PriceBand1;
    double           PriceBand2;
}CFFEXIncQuotaDataT;
#pragma pack(pop)


#endif //end __HTF_CFFEX_H__

