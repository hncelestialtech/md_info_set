#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <string>
#include <map>
#include <signal.h>
#include <iostream>
#include <limits>
#include <pthread.h>
#include <errno.h>
using namespace std;

#include "my_net_interface.h"

#pragma pack(1)
struct MarketDataField
{
    int packetLen;//报文长度
    unsigned char versionNo;//版本序号
    int       updateTime;//修改时间
    char   exchangeID[3];//交易所
    char   instrumentID[30];//合约代码
    bool   stopFlag;//ͣ停牌标识
    char   statusLatestPrice;//
    double latestPrice;//最新价
    char   statusMatchAmount;//
    int    matchAmount;//�ɽ���
    char   statusPositionAmount;//
    int    positionAmount;//�ֲ���
    char   statusHighestPrice;//
    double highestPrice;//��߼�    
    char   statusLowestPrice;//
    double lowestPrice;//��ͼ�
    char   statusBuyPrice1;//
    double buyPrice1;//�����1
    char   statusSellPrice1;//
    double sellPrice1;//������1
    char   statusBuyAmount1;//
    int    buyAmount1;//������1
    char   statusSellAmount1;//
    int    sellAmount1;//������1
    char   statusMatchMoney;//
    double macthMoney;//�ɽ����
    char   statusOpenPrice;//
    double openPrice;//���̼�
    char   statusAvgPrice;//
    double avgPrice;//���վ���
};
#pragma pack()

void outToFile(FILE *fp,struct MarketDataField const *p)
{
    struct timeval tv;
    struct tm t;
    gettimeofday(&tv,NULL);
    localtime_r(&tv.tv_sec,&t);  
    fprintf(fp,""
    //"%04d-%02d-%02d "
    "%02d:%02d:%02d.%06ld "
    "%d,%d,%09d,%s,%s,%d,"
    //"%d,%.3lf,%d,%d,%d,%d,%d,%.3lf,%d,%.3lf,%d,%.3lf,%d,%.3lf,%d,%d,%d,%d,%d,%.3lf\n",
    "%x,%.3lf,%x,%d,%x,%d,%x,%.3lf,%x,%.3lf,%x,%.3lf,%x,%.3lf,%x,%d,%x,%d,%x,%.3lf,%x,%.3lf,%x,%.3lf\n",
    //1900+t.tm_year, 1+t.tm_mon, t.tm_mday, 
    t.tm_hour, t.tm_min, t.tm_sec, tv.tv_usec,
    p->packetLen,//���ĳ���
    p->versionNo,//�汾���
    p->updateTime,//�޸�ʱ��
    p->exchangeID,//������
    p->instrumentID,//��Լ����
    p->stopFlag,//ͣ�Ʊ�ʶ
    p->statusLatestPrice,//
    p->latestPrice,//���¼�
    p->statusMatchAmount,//
    p->matchAmount,//�ɽ���
    p->statusPositionAmount,//
    p->positionAmount,//�ֲ���
    p->statusHighestPrice,//
    p->highestPrice,//��߼�    
    p->statusLowestPrice,//
    p->lowestPrice,//��ͼ�
    p->statusBuyPrice1,//
    p->buyPrice1,//�����1
    p->statusSellPrice1,//
    p->sellPrice1,//������1
    p->statusBuyAmount1,//
    p->buyAmount1,//������1
    p->statusSellAmount1,//
    p->sellAmount1,//������1
    p->statusMatchMoney,//
    p->macthMoney,//�ɽ����
    p->statusOpenPrice,//
    p->openPrice,//���̼�
    p->statusAvgPrice,//
    p->avgPrice//���վ���
    );
    fflush(fp);
}


//�ж��ļ��Ƿ����
bool fileExists(const char *fileName);
//������־�ļ�
int backUpFile(const char *fileName);
//������ļ�
void outToFile(FILE *fp, const char *format, ...);
//������ļ�
void outToFile(FILE *fp, MarketDataField const *p);
//��ȡ����ʱ�䣨΢�룩
long GetLongTime();
//��ȡ�����ַ���ʱ���
void GetStrTime(char strTime[]);
//������־
void CreateLog();
//�ر���־
void DestroyLog();
//����ʱ��->�ַ���ʱ��
char * TimeLongToStr(char* strTime, int len, long nTime);
//�ַ���ʱ��->����ʱ��
long TimeStrToLong(string str);
//doubleת16����
char *DoubleToHex(double dValue, char *sValue);

#include <float.h>
#include <limits>
static const double dmax = std::numeric_limits<double>::max();
static const double dnan = std::numeric_limits<double>::quiet_NaN();
void IF_BIG_THEN_ZERO(double &x){if(x != dmax){}else{x = 0;}}
    
FILE * g_fpSpeed = NULL;//������־�ļ�ָ��
FILE * g_fpMd = NULL;//��̬������־
FILE * g_fpMdEx = NULL;//��̬������־
FILE * g_fpData = NULL;//��ͨ��־

//����汾
#define PROGRAM_VERSION "v1.0.0"
//���ջ�������С
#define MAXLINE 2048

//CTP��ʽ��־1
#define LEVEL1_MD_LOG "level1_mult.log"

//���������ȷ��
#define RECEIVE_THEN_CHECK 1
//���٣����ܲ��ԣ�
#define RECEIVE_THEN_QUIT 0

//�Ƚ��嵵�۸�
#define CHECK5LEVEL(p1,p2,p3,p4,p5) (p1>=p2&&p2>=p3&&p3>=p4&&p4>=p5)
#define CHECKEQUAL_FLOAT(x,y) (abs((x)-(y)) <= 0.00001)
#define CHECKEQUAL_DOUBLE(x,y) (abs((x)-(y)) <= 0.00000001)
#define CHECKEQUAL_STRING(str1,str2) (!strcmp(str1,str2))
#define CHECKEQUAL_INTERGER(x,y) ((x) == (y))
#define MIN(x,y) ( ((x)<(y))?(x):(y) )

//�����������IP(û���õ�)
char LOCAL_IP[20] = "127.0.0.1";
//����������ն˿�
//long LISTEN_PORT = 20000;

//����IP
char LISTEN_IP[20] = "0.0.0.0";
//���ն˿ڣ������޸ģ�
long LISTEN_PORT = 10001;
//�鲥��ַ�������޸ģ�
char MULTICAST_ADDR[20] = "229.100.100.100";

//�����˳���־
bool g_isQuit = false;
//����������
#define MAX_RECEIVIE_NUM 20000
//��Ҫ�������ݸ���
long g_needRecvedNum = 5000;
//�Ѿ��������ݸ���
long g_haveRecvedNum = 0;
//��Ҫ���մ���
long g_needLoopTime = 2;
//�Ѿ����մ���
long g_haveLoopTime = 0;

//����ʱ��������Σ�
long g_localLongTime[MAX_RECEIVIE_NUM] = {0};
//����ʱ������ַ�����
char g_localStrTime[MAX_RECEIVIE_NUM][40] = {0};
//����ʱ�����΢��ṹ��ʱ�䣩
struct timeval g_localTVTime[MAX_RECEIVIE_NUM] = {0,0};

//�����Լ����
char g_instrumentID[MAX_RECEIVIE_NUM][64] = {0};
//������ʱ���
int  g_exchangeTime[MAX_RECEIVIE_NUM] = {0};
//udp ���ջ����С
int g_rcvBufSize = /*212992*/0;

#ifndef WIN32
//�źŴ�������  
void signal_handler(int signo)  
{  
    //����ź�ֵ  
    if(SIGINT == signo||SIGQUIT == signo||SIGKILL == signo)
    {
        g_isQuit = true;
        DestroyLog();
        exit(0);
    }
}
#endif

void printHelp()
{
    printf("----help----\n");
    printf("example./level1_Demo -groupip 239.6.6.6 -port 10008\n");
    printf("-port        \n");
    printf("-groupip     \n");
    printf("-recvbuf     set udp buffer\n");
}

// ʹ�÷�ʽ��./level1_Demo -groupip 239.6.6.6 -port 10008

int main(int argc, char* argv[])
{
    bool isUseLocalIp = false;
    bool isUseMultiCast = true;
    string deviceName = "eth0";
    string deviceIp = "127.0.0.1";
    bool isGivenDeviceName = false;
    for(int i=0;i<argc;++i)
    {
        if(!strcmp(argv[i],"-h") || !strcmp(argv[i],"-help") || !strcmp(argv[i],"--h"))
        {
            printHelp();
            return 0;
        }
        else if(!strcmp(argv[i],"-localip"))
        {
            strncpy(LOCAL_IP,argv[++i],sizeof(LOCAL_IP));
            isUseLocalIp = true;
            deviceIp = LOCAL_IP;
        }
        else if(!strcmp(argv[i],"-ip"))
        {
            strncpy(LISTEN_IP,argv[++i],sizeof(LISTEN_IP));
            isUseMultiCast = false;
            deviceIp = LISTEN_IP;
        }
        else if(!strcmp(argv[i],"-port"))
        {
            LISTEN_PORT = atol(argv[++i]);
        }
        else if(!strcmp(argv[i],"-groupip"))
        {
            strncpy(MULTICAST_ADDR,argv[++i],sizeof(MULTICAST_ADDR));
            isUseMultiCast = true;
            deviceIp = MULTICAST_ADDR;
        }
		else if(!strcmp(argv[i],"-dev"))
		{
			deviceName = argv[++i];
			isGivenDeviceName = true;
		}
        else if(!strcmp(argv[i],"-recvbuf"))
		{
            int n = atoi(argv[++i]);
            if (n >= 0) {
                g_rcvBufSize = n;
            }
		}
    }
    if(!isGivenDeviceName)
    {
        char devName[20] = {0};
        getNetInterface(devName,sizeof(devName)-1);
        deviceName = devName;
        printf("dev=%s\n",devName);
    }
    
    char cmd[100];
    sprintf(cmd,"route add -net %s.0/24 dev %s",deviceIp.substr(0,deviceIp.rfind(".")).c_str(),deviceName.c_str());
    printf("%s\n",cmd);
    system(cmd);
    
    printf("PROGRAM_VERSION:%s\n",PROGRAM_VERSION);
    
    if(g_needRecvedNum > MAX_RECEIVIE_NUM)
    {
        printf("MAX_RECEIVIE_NUM:%ld\n",MAX_RECEIVIE_NUM);
        return 0;
    }
    
    if(isUseMultiCast)
    {    
        printf("multicast ip = %s,port = %ld\n",MULTICAST_ADDR,LISTEN_PORT);
    }
    else
    {
        printf("ip= %s,port= %ld\n",LISTEN_IP,LISTEN_PORT);
    }
    
    #ifndef WIN32
        
    //�����ǰ���̺�  
    printf("process id is %d\n", getpid());  
    //ΪSIGINT�ź������źŴ�������  
    signal(SIGINT, signal_handler);  
    //ΪSIGHUP�ź������źŴ�������  
    signal(SIGHUP, signal_handler);  
    //ΪSIGQUIT�ź������źŴ�������  
    signal(SIGQUIT, signal_handler);
        
    #endif
    
    CreateLog();//������־
    
    int sockfd;//�������׽���
    struct sockaddr_in servaddr;//��������ַ
    struct sockaddr_in cliaddr;//�ͻ��˵�ַ
    
	//������1������UDP�׽��֡�
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); //����UDP�׽���

    //��ʼ����������ַ
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    if(isUseLocalIp)
    {
        servaddr.sin_addr.s_addr = inet_addr(LOCAL_IP);//���ձ���ָ����������
    }
    else if(isUseMultiCast)
    {
        servaddr.sin_addr.s_addr = inet_addr(MULTICAST_ADDR);//�����鲥����
    }
    else
    {
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//���ձ����κ���������    
    }
    servaddr.sin_port = htons(LISTEN_PORT);//�����������ݶ˿�

    //�趨SO_REUSEADDR���������Ӧ�ð�ͬһ�����ض˿ڽ������ݰ�
    int reuse = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
    {
        perror("setsockopt SO_REUSEADDR error");
        //close(sockfd);
        //exit(-3);
    }

    // �鿴ϵͳĬ�ϵ�socket���ջ�������С
    int defRcvBufSize = -1;
    socklen_t optlen = sizeof(defRcvBufSize);
    if (getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &defRcvBufSize, &optlen) < 0)
    {
        printf("getsockopt error=%d(%s)!!!\n", errno, strerror(errno));
        exit(-4);
    }
    printf("getsockopt defRcvBufSize=%d\n", defRcvBufSize);
    
    // ����ִ�в�������UDP SOCKET���ջ�������С
    if (g_rcvBufSize > 0) 
    {
        optlen = sizeof(g_rcvBufSize);
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &g_rcvBufSize, optlen) < 0)
        {
            printf("setsockopt error=%d(%s)!!!\n", errno, strerror(errno));
            exit(-4);
        }
        printf("set udp socket(%d) recv buff size to %d OK!!!\n", sockfd, g_rcvBufSize);
    } 
    else 
    {
        printf("use default RcvBufSiz=%d\n", defRcvBufSize);
    }

    //������2������������ַ�ͷ������׽��ְ󶨡�
    if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind error");
        exit(-4);
    }
    
    //������3�������鲥�顿
    struct ip_mreq mreq;
    if(isUseMultiCast)
	{
        //���ûػ�����
        int loop = 1;
        if(setsockopt(sockfd,IPPROTO_IP, IP_MULTICAST_LOOP,&loop, sizeof(loop)) < 0)
        {
            perror("setsockopt():IP_MULTICAST_LOOP");
            //exit(-5);
        }
        
        bzero(&mreq, sizeof (struct ip_mreq)); 
        mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_ADDR); //�㲥��ַ
        //���÷����鲥��Ϣ��Դ�����ĵ�ַ��Ϣ
        mreq.imr_interface.s_addr = htonl (INADDR_ANY);

        //�ѱ��������鲥��ַ��������������Ϊ�鲥��Ա��ֻ�м���������յ��鲥��Ϣ
        if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,sizeof (struct ip_mreq)) == -1)
        {     
            perror ("setsockopt IP_ADD_MEMBERSHIP");
            exit(-6);
        }
        //����TTL
        unsigned char ttl = 255;
        if(setsockopt(sockfd,IPPROTO_IP,IP_MULTICAST_TTL,&ttl,sizeof(ttl)) < 0)
        {
            perror("setsockopt():IP_MULTICAST_TTL\n");
        }

    }//MODE_MULTICAST

    fcntl(sockfd,F_SETFL,O_NONBLOCK);

    int n;//���յ������ݳ���
    socklen_t len = sizeof(cliaddr);//�ͻ��˵�ַ����

    char buffer[MAXLINE];//�洢���յ�����������
    
    MarketDataField recvMarketData;//����ṹ������
    memset(&recvMarketData,0,sizeof(recvMarketData));//�������ṹ������
    map<string,MarketDataField> quoteDataMap;//����map
    
    int PacketSize = sizeof(MarketDataField);
    printf("sizeof(MarketDataField) = %d\n", sizeof(MarketDataField));
    
    char sValue[30] = "";
    char sFlag[30] = "0x7fefffffffffffff";
    double dValue = std::numeric_limits<double>::max();//(double)0x7fefffffffffffff;

    //while(!g_isQuit)
    while(1)
    {
        bzero(buffer,sizeof(buffer));//��մ洢����
        
        //������4���ȴ��������ݡ�
        n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
        printf("receive from  %s:%u,len:%d\n", inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port),n);
        if( n <= 0 ) {
            continue;
        }
        else if(n >= PacketSize){

        //��ӡ�������յ�������
        printf("receive from %s:%u\n", inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));
        
        #if RECEIVE_THEN_QUIT
        //��ȡ����ʱ��
        gettimeofday(&g_localTVTime[g_haveRecvedNum],NULL);
        #endif//RECEIVE_THEN_QUIT
        
        memcpy(&recvMarketData,buffer,sizeof(recvMarketData));

        IF_BIG_THEN_ZERO(recvMarketData.latestPrice);//���¼�
        IF_BIG_THEN_ZERO(recvMarketData.highestPrice);//��߱���
        IF_BIG_THEN_ZERO(recvMarketData.lowestPrice);//��ͱ���
        IF_BIG_THEN_ZERO(recvMarketData.buyPrice1);//�����1
        IF_BIG_THEN_ZERO(recvMarketData.sellPrice1);//������1
        IF_BIG_THEN_ZERO(recvMarketData.macthMoney);//�ɽ����
        IF_BIG_THEN_ZERO(recvMarketData.openPrice);//���̼�
        IF_BIG_THEN_ZERO(recvMarketData.avgPrice);//���վ���
        
        #if RECEIVE_THEN_QUIT

        //��ȡ��Լ����
        strcpy(g_instrumentID[g_haveRecvedNum],recvMarketData.instrumentID);
        //��ȡ������ʱ���
        g_exchangeTime[g_haveRecvedNum] = recvMarketData.updateTime;
        
        //����ָ����������Ȼ���������˳�
        if(++g_haveRecvedNum >= g_needRecvedNum)
        {
            if(g_fpRecv)
            {
                struct tm p;
                for(long i=0;i<g_needRecvedNum;i++)
                {
                    localtime_r(&g_localTVTime[i].tv_sec,&p);
                    fprintf(g_fpRecv,"%02d:%02d:%02d.%06ld,%s,%d\n", 
                        p.tm_hour, p.tm_min, p.tm_sec, g_localTVTime[i].tv_usec, //����ʱ��
                        g_instrumentID[i],//��Լ����
                        g_exchangeTime[i]//������ʱ��
                        );
                }
                fflush(g_fpRecv);
            }
            g_haveRecvedNum = 0;
            /*
            if(++g_haveLoopTime >= g_needLoopTime)
            {
                g_isQuit = true;
                break;
            }
            */
        }
        
        #elif RECEIVE_THEN_CHECK
        
        outToFile(g_fpMd,&recvMarketData);
        // string recvString = recvMarketData.instrumentID;
        // map<string,MarketDataField>::iterator iter;
        // iter = quoteDataMap.find(recvString);
        // if(iter!=quoteDataMap.end())
        // {
            
        // }
        // else
        // {
            
            // quoteDataMap.insert(map<string,MarketDataField>::value_type(recvString, recvMarketData));//�����º�Լ������

        // }

        #else    

        outToFile(stdout,&recvMarketData);//��ӡ�ṹ�����ݵ�����̨����

        #endif//RECEIVE_THEN_QUIT

    }
    else
    {
        printf("Heart Beat Packet\n");
    }

    }//end of while
    
    DestroyLog();//�ر���־

    if(isUseMultiCast)
    {
        //�˳��ಥ��
        setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP,&mreq, sizeof(mreq));
    }

    close(sockfd);

    g_isQuit = true;
    return 0;
}

//������־
void CreateLog()
{
#if RECEIVE_THEN_CHECK    
    backUpFile(LEVEL1_MD_LOG);
    g_fpMd = fopen(LEVEL1_MD_LOG,"w");
    if(g_fpMd == NULL)
    {
        perror("create LEVEL1_MD_LOG log error");
    }
    fputs("报文长度,版本序号,修改时间,交易所,合约代码,停牌标识,最新价,成交量,持仓量,最高价,最低价,申买价1,申卖价1,申买量1,申卖量1,成交金额\n",g_fpMd);
#elif RECEIVE_THEN_QUIT
    backUpFile(RECEIVE_LOG);
    g_fpRecv = fopen(RECEIVE_LOG,"w");
    if(g_fpRecv == NULL)
    {
        perror("create RECEIVE_LOG error");
    }
#else
    backUpFile(MDLOG);
    g_fpData = fopen(DATA_LOG,"w");
    if(g_fpData == NULL)
    {
        perror("create DATA_LOG error");
    }
#endif
}
//�ر���־
void DestroyLog()
{
    if(g_fpSpeed)//������־
    {
        fclose(g_fpSpeed);
        g_fpSpeed = NULL;
    }
    if(g_fpMd)
    {
        fclose(g_fpMd);
        g_fpMd = NULL;
    }
    if(g_fpMdEx)
    {
        fclose(g_fpMdEx);
        g_fpMdEx = NULL;
    }
    if(g_fpData)
    {
        fclose(g_fpData);
        g_fpData = NULL;
    }
}

//��ȡ��������ʱ���
long GetLongTime()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (tv.tv_sec*1000000 + tv.tv_usec);
}
//��ȡ�����ַ���ʱ���
void GetStrTime(char strTime[])
{
    struct timeval tv;
    struct tm p;
    gettimeofday(&tv,NULL);
    localtime_r(&tv.tv_sec,&p);
    sprintf(strTime,"%02d:%02d:%02d.%06ld",p.tm_hour, p.tm_min, p.tm_sec, tv.tv_usec);
    return;
    //sprintf(strTime,"%4d-%02d-%02d %02d:%02d:%02d.%06ld",1900+p.tm_year, 1+p.tm_mon, p.tm_mday, p.tm_hour, p.tm_min, p.tm_sec, tv.tv_usec);
    //return (tv.tv_sec*1000 + tv.tv_usec);
}
//�ж��ļ��Ƿ����
bool fileExists(const char *fileName)
{
    //return (access(fileName,0) == 0);
    FILE *fp = fopen(fileName,"r");
    if(fp)
    {
        fclose(fp);
        return true;
    }
    return false;
}
//����־�Ѿ������򱸷���־�ļ�
int backUpFile(const char *fileName)
{
    if(fileExists(fileName))
    {
        char newfileName[260] = {0};
        strncpy(newfileName,fileName,sizeof(newfileName));
        struct timeval tv;
        struct tm      p;
        gettimeofday(&tv,NULL);
        localtime_r(&tv.tv_sec,&p);
        int i = 1;
        do{
            sprintf(newfileName,"%s_%04d%02d%02d_test%d.log",fileName,p.tm_year+1900,p.tm_mon+1,p.tm_mday,i++);
        }while(fileExists(newfileName));
        return rename(fileName,newfileName);
    }
    return 0;
}
//������ļ�
void outToFile(FILE *fp,const char *format,...)
{
    if (NULL==format||0==format[0]) return;

    char buf[4096] = {0};
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    
    char strTime[30] = {0};
    struct timeval tv;
    struct tm      p;
    gettimeofday(&tv,NULL);
    localtime_r(&tv.tv_sec,&p);  
  sprintf(strTime,"%04d-%02d-%02d %02d:%02d:%02d.%06ld ",1900+p.tm_year, 1+p.tm_mon, p.tm_mday, p.tm_hour, p.tm_min, p.tm_sec, tv.tv_usec);

    if(fp){
        fputs(strTime, fp);
        fputs(buf, fp);
        fflush(fp);
    }
//    printf("[%s]%s\n", strTime, buf);
}

//������ʱ��תΪ�ַ�����95030500 -> 09:50:30.500 
char * TimeLongToStr(char* strTime, int len, long nTime)
{
    if(len < 12)
    {
        return strTime;
    }
    // if the filetime
    if (nTime > 999999999)
    {
        strftime(strTime, len, "%H:%M:%S.000", localtime(&nTime));
        return strTime;
    }
    
    char time_s[32] = {0};    
    sprintf(time_s, "%.9d", nTime);
    strTime[0] = time_s[0];
    strTime[1] = time_s[1];
    strTime[2] = ':';
    strTime[3] = time_s[2];
    strTime[4] = time_s[3];
    strTime[5] = ':';
    strTime[6] = time_s[4];
    strTime[7] = time_s[5];
    strTime[8] = '.';
    strTime[9] = time_s[6];
    strTime[10]= time_s[7];
    strTime[11]= time_s[8];
    return strTime;
}

//��ʱ���ַ���תΪ���Σ�09:50:30.500 -> 95030500
long TimeStrToLong(string str)
{
    string::iterator it;
    for (it=str.begin(); it != str.end(); ++it)
    {
        if ( *it == ':' || *it == '.')
        {
            str.erase(it);
        }
    }
    return atol(str.c_str());
}

#define IF_NOT_IN(x,minPrice,maxPrice) (x<minPrice||x>maxPrice)

//doubleת16����
char *DoubleToHex(double dValue, char *sValue)
{
    unsigned char *p = (unsigned char*)&dValue;
    sprintf(sValue, "%02x%02x%02x%02x%02x%02x%02x%02x\n", p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
    //sprintf(sValue, "%02X%02X%02X%02X%02X%02X%02X%02X\n", p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
    fprintf(stderr,"%s\n",sValue);
    return sValue;
}