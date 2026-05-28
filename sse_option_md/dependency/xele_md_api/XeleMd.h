/**
 * 行情接口类
 **/

#ifndef __MD_XELE_H__
#define __MD_XELE_H__
#include "XeleMdSpi.h"

#define XELE_MD_API_VERSION  "2.2.2"                       // api版本号

//网卡类型枚举
enum ENicType
{
    E_NIC_NORMAL,                                          //普通网卡，通过操作系统协议栈接收
    E_NIC_SOLARFLARE_EFVI,                                 //solarflare网卡通过efvi接收，能降低延迟和丢包可能性
    E_NIC_EXANIC,                                          //exanic网卡，通过exanic库提供的接口加速
};

//接收线程传入结构体定义
typedef struct _MdParam
{
    char                     m_interfaceName[64];          //接收网卡名
    char                     m_localIp[32];                //接收网卡ip
    char                     m_mcastIp[32];                //组播ip
    uint16_t                 m_mcastPort;                  //组播端口
    int                      m_bindCpuId;                  //接收线程绑定cpuid，-1代表不绑核
    ENicType                 m_nicType;                    //网卡类型
    bool                     m_polling = true;             //是否使用忙等待模式收包，忙等待模式延迟更小，但会增大cpu占用
    int                      m_cache = 0;                  //api缓存报文的chche大小，单位MB，0代表不使用cache
    int                      m_cacheCpuId = -1;            //cache模式下消费线程绑核cpuid，-1代表不绑核
    
    /**
        * 主备自动切换功能，仅支持盘中切换，在主组播上一定时间内收不到包自动切换到备用组播，
        * 主组播有包后再自动切回来
    **/
    char                     m_backupIntName[64];          //备用接收网卡名
    char                     m_backupLocalIp[32];          //备用接收网卡ip
    char                     m_backupMcastIp[32];          //备用组播ip
    uint16_t                 m_backupMcastPort = 0;        //备用组播端口,配成0时不启用主备切换功能
    ENicType                 m_backupNicType;              //备用网卡类型
    int                      m_backupCpuId;                //备用报文接收线程绑核cpuid，-1代表不绑核
    int                      m_backupSwitchTime;           //多长时间收不到包切换，单位秒
    //逐笔构建快照， 0代表不构建，1代表上交股票构建，2代表深交股票构建,3代表上交债券,4代表构建上交股票和债券                                                      
    int                      m_tickToSnap = 0;             
    int                      m_tickSnapCpuId = -1;         //逐笔转快照绑核cpuid，-1代表不绑核
    char                     m_tickSnapSubscribe[256];     //逐笔构建快照合约订阅文件路径
    bool                     m_lastFlag = false;           //上交逐笔行情是否标记当前报文为该合约在fast压缩包中最后一次出现
    bool                     m_tickToSnapDealLost = false; //发现channel丢包时用交易所快照内容填充构建的快照，该channel不再使用逐笔构建快照 
} MdParam;

typedef struct _Version
{
    char                     api_ver[16];                  //API版本
    char                     sse_ver[16];                  //上交结构体版本
    char                     sze_ver[16];                  //深交结构体版本
} Version;

//行情系统类型定义，对应MulticastAddr结构体md_system字段
#define SH_SSE_MAIN          0x01                          //上海发出的上交主行情
#define SH_SSE_BACKUP        0x02                          //上海发出的上交备行情
#define SH_SZE_MAIN          0x04                          //上海发出的深交主行情
#define SH_SZE_BACKUP        0x08                          //上海发出的深交备行情
#define SZ_SZE_MAIN          0x10                          //深圳发出的深交主行情
#define SZ_SZE_BACKUP        0x20                          //深圳发出的深交备行情
#define SZ_SSE_MAIN          0x40                          //深圳发出的上交主行情
#define SZ_SSE_BACKUP        0x80                          //深圳发出的上交备行情

struct MulticastAddr
{
    uint32_t                 md_system;                    // 组播地址所属的行情系统
    char                     ip[16];
    uint16_t                 port;
    char                     md_type[128];                 // 对应的行情类型，取值为行情结构体中的type字段, 逗号分割，可包含多个类型
} __attribute__((packed));

struct LoginReq 
{
    char                     server_ip[16];                //鉴权服务器IP
    uint16_t                 server_port;                  //鉴权服务器端口
    char                     username[16];                 //用户名
    char                     password[16];                 //用户密码
    char                     eth_name[32];                 //本机接收行情的网卡名
};

struct LoginRsp
{
    int                      err_code;                     //错误码
    char                     err_msg[40];                  //错误信息
    uint32_t                 numAddr;                      //返回的有效组播地址数量, 后面跟numAddr个MulticastAddr
};

class XeleMd
{
public:
    /**
        * @brief 创建类实例
    **/
    static XeleMd* CreatInstance();
    /**
        * @brief 获取API以及结构体定义版本号
        * @param[in] ver：指向传入版本结构体的指针
    **/
    virtual void GetVersion(Version *ver) = 0;

    /**
     * @brief 发送鉴权请求报文给服务器，并接收鉴权响应以及组播信息
     * @param[in] login_req：鉴权请求报文
     * @param login_rsp: 鉴权响应报文
     * @param addrBegin: 组播信息首地址
     * @return bool
     * @retval true: 鉴权成功
     * @retval false: 鉴权失败
     **/
    virtual bool Login(LoginReq* login_req, LoginRsp *login_rsp, struct MulticastAddr **addrBegin) = 0;

    /**
        * @brief 初始化API模块
        * @param[in] param：指向传入结构体指针数组的指针
        * @param[in] count：传入结构体指针数组大小
        * @return 初始化结果,0代表成功，-1代表失败
    **/
    virtual int Init(MdParam* param, const int count) = 0;
    
    /**
        * @brief 注册上交行情回调接口
        * @param pSpi: 派生自上交回调接口类的实例
        * @return 
        **/
    virtual void RegisterSseSpi(XeleMdSseSpi* pSpi) = 0;
    
    /**
        * @brief 注册深交行情回调接口
        * @param pSpi: 派生自深交回调接口类的实例
        * @return 
        **/
    virtual void RegisterSzseSpi(XeleMdSzseSpi* pSpi) = 0;
    
    /**
        * @brief 开始接收行情
        * @return 接收执行结果,0代表成功，-1代表失败
        **/
    virtual int Start() = 0;
    
    /**
        * @brief 查询指定合约的静态信息
        * @param[in] security：合约ID
        * @return 查询到的静态信息结构体指针，查不到返回nullptr
        **/
    virtual const StaticInfo *QueryStatic(const char *securityId) = 0;
    
    /**
        * @brief 停止接收行情
        * @return 停止执行结果,0代表成功，-1代表失败
        **/
    virtual int Stop() = 0;
	
    virtual ~XeleMd();

    /**
     * 废弃，为兼容性保留
    **/
    virtual int Init(MdParam** param, const int count) = 0;
};

/******
 行情回补接口
******/
enum ERROR_NO_DEF
{
    LOGON_SUCCESS = 0,                                     //登录成功
    WRONG_USER_OR_PASSWD,                                  //错误的用户名或密码
    WRONG_EXCHANGE_TYPE,                                   //错误的交易所ID
    CONNET_FAILED,                                         //连接失败，可能url错误或服务未启动

    QUERY_RETURN_All = 100,                                //回补成功
    QUERY_RETURN_PART,                                     //回补部分成功
    QUERY_RETURN_NO_PERMISSION,                            //没有权限
    QUERY_RETURN_DATA_USELESS,                             //无效数据
    NO_MATCH_DATA,                                         //无匹配数据
    TCP_CLOSED,                                            //未收到数据
    INTERNAL_ERROR,                                        //内部错误
};

typedef struct _RestoreQueryFeild
{
    char                     clientId[11];                 //用户ID
    char                     password[11];                 //用户密码
    uint8_t                  exchangeType;                 //交易所类型 1: sse 2:sz.
    uint64_t                 appSeqStart;                  //起始序列号
    uint64_t                 appSeqEnd;                    //结束序列号
    uint16_t                 channelId;                    //通道号
    uint32_t                 categoryId;                   //上交需要填写
} __attribute__((packed)) RestoreQueryFeild;

class XeleMdRestoreSpi
{
public:
    ///行情通知
    virtual void OnRestoreData(int errorno, uint8_t *pdata, bool bLast){};
};

class XeleMdRestoreApi
{
public:
    //创建restore实例
    static XeleMdRestoreApi *CreateRestore(XeleMdRestoreSpi *p);

    //同步，直到所有回补行情都发送给客户，该函数才返回，回调函数和请求在同一个线程
    virtual int RestoreQuery(const char *restoreUrl, const RestoreQueryFeild &_logonReq) = 0;
};

#endif