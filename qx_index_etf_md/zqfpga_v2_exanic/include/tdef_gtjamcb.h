#pragma once
// 行情应用层定义
typedef struct
{
    unsigned int type; // MD_TYPE
    unsigned int len;  // length of data
    char data[];       // data
} GTJAMDHeader;

typedef struct
{
    unsigned int type; // MD_TYPE
    unsigned int len;  // length of data
    char data[768];    // data
} StaticGTJAMDHeader;