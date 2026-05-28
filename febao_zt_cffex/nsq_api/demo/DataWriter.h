#ifndef __LAND_FILE_DATA_WRITE_H_
#define __LAND_FILE_DATA_WRITE_H_
#include "HSNsqStruct.h"
#include "InnerData.h"
#include <stdio.h>

class DataWriter;
typedef void (DataWriter::*fn_write_file)(char *pData);

class DataWriter
{
public:
    DataWriter();
    ~DataWriter();
    void WriteFile(DataType data_type, char *pData);

private:
    FILE *GetMarketFP(DataType data_type, const char *ExchangeID, int channel_no = 0);

    void WriteSnapshot(char *pData);
    void WriteTrade(char *pData);
    void WriteOrder(char *pData);
    void WriteOptSnapshot(char *pData);
    void WriteAtpSnapshot(char *pData);
    void WriteSnapshotPlus(char *pData);
    void WriteHktSnapshot(char *pData);
    void WriteBondTrade(char *pData);
    void WriteBondOrder(char *pData);
    void WriteFutSnapshot(char *pData);
    void WriteFutOptSnapshot(char *pData);

    FILE *OpenSnapFile(const char *ExchangeID);
    FILE *OpenTickFile(const char *ExchangeID, int channel_no);
    FILE *OpenOptSnapFile(const char *ExchangeID);
    FILE *OpenAtpSnapFile(const char *ExchangeID);
    FILE *OpenSnapPlusFile(const char *ExchangeID);
    FILE *OpenHktSnapFile(const char *ExchangeID);
    FILE *OpenBondTickFile(const char *ExchangeID, int channel_no);
    FILE *OpenFutSnapFile(const char *ExchangeID, bool is_opt);

    void CreateCsvFolder();

private:
    fn_write_file m_fun_write[(int)DataType::DT_MAX];

    FILE *m_file_fp[MAREKT_MAX_INDEX][(int)DataType::DT_MAX];

    FILE *m_tick_fp[MAREKT_MAX_INDEX][CHANNEL_NO_MAX_INDEX];

    // csv文件夹名字
    char m_csv_name[64];
    // 本次运行是否已创建csv文件夹
    bool m_is_create_csv{false};
};

#endif