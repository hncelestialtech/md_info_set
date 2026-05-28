# 飞豹行情接入说明

## 1. 行情源概述
目前国内的行情源大体上分为以下几类：
### 柜台API行情
1. 基本上所有的期货或者证券柜台都会提供行情api，例如femas柜台，ctp柜台等
2. 这类行情源接入飞豹只需少量工作，例如将api的行情数据回调结构转换为飞豹的行情结构
3. 飞豹已经提供了众多柜台api行情接入，但是这类行情的延时一般较大
3. demo_counter_api_md演示了这种行情源接入飞豹的通用做法

### 交易所datafeed行情
1. 有几家交易所提供了datafeed行情，一般都是通过UDP的方式下发行情数据
2. 包括中金所，大商所，广期所。另外中金所最近重构了老的datafeed行情，性能和易用性都有所增强
3. 飞豹也提供了上述三家datafeed行情接入，这类行情性能较好
4. 接入方式和柜台api行情基本一致，可以参考demo_counter_api_md

### 交易所协议行情
1. 包括易盛的多播行情，上期和能源的行情，上证和深证的行情，都是只提供原始协议
2. 这类行情接入需要的工作量较大，需要考虑的问题较多
3. 飞豹也提供了此类的行情接入，包括es_multi_md, sfe_md, sse_vde_md, sse_binary_md, sze_binary_md
4. demo_protocol_md演示了这种行情源的接入方法

### 硬件行情
1. 市面上的硬件行情也有不少，部分是以思科的exanic可编程硬件网卡为载体的
2. 这种行情主要应用在直接提供协议的各交易所后，基于FPGA的能力提供更小的行情延时
3. 硬件行情没有统一的输出格式，各家都不太一样。但是为了减小延时，都选择丢弃某些静态数据，例如前结算等
4. 这种行情源本身的适配工作很小，但是由于缺失部分数据，往往需要借助柜台api进行补全，综合说适配难度中等
5. 除此之外也可以借助思科的libexanic库进行补全，但是难度比较大，测试困难
6. 飞豹提供了易盛的硬件网卡接入和其他几种网卡接入插件
7. 未提供示例程序，可参考demo_counter_api_md

### 镜像行情
1. 根据行情源的不同，可以通过软件或者硬件对行情源进行镜像，方便使用
2. 这种行情解析通用的做法是使用原始套接字并设置网卡为混杂模式抓包获得数据
3. 需要考虑的问题最多，包括多播UDP的丢包恢复，TCP的过滤和丢包检测等等
4. 技术产品问题可以和飞豹支持讨论，其他不予说明
5. 未提供示例程序，可以参考demo_protocol_md

## 2. 飞豹行情api说明
### 回调接口
1. 行情接入插件继承fb_md_plugin_api后，fb_md_proxy框架按照如下顺序进行回调：
```cpp
virtual void register_spi(fb_i_md_spi *spi)                                        = 0;
virtual int  init()                                                                = 0;
virtual void subscribe_inst(const std::string &instrument_id, uint8_t exchange_id) = 0;
virtual void connect()                                                             = 0;
virtual void release()                                                             = 0;
```
2. 在fb_md_proxy针对行情插件的进行回调时，是在核心线程中进行的，不能在回调函数中进行长时间的阻塞操作。例如不能在connect回调中进行类似busyloop或者IO多路复用操作
3. 如果有需要的话插件可以另起线程
4. spi中的on_msg接口不是线程安全的，只能由插件中的工作线程专用
5. connect回调只是启动行情插件接收功能，不能在回调线程中做耗时操作
6. release回调中释放资源时不能长期阻塞，否则fb_md_proxy进程无法关闭

### 就绪通知
1. 当行情插件初始化完成后，需要调用spi的on_ready接口通知飞豹主程序，否则飞豹将会视为启动失败
2. 以柜台API行情为例，当柜台连接，登录，订阅成功后，即可视为成功，需要调用on_ready
```cpp
virtual void on_ready() {}
```

### 编译运行
1. 行情接入插件只支持linux平台动态库形式，系统版本和编译器版本可询问飞豹支持人员
2. 编译选项必须加上-fPIC，选择c++进行开发的话建议选用c++17
2. 行情接入插件名称统一为xxxx_md.so，例如femas_md.so


### 导出函数
正常情况下，linux下动态库中所有的符号都是默认导出的，但是以下接口必须要导出，并且原型要保持一致，这个在飞豹mdapi文档中已有说明：
```cpp
#ifdef __cplusplus
extern "C" {
#endif

void *create();
void destroy(void *p);
void get_md_api_version(char version[32]);

#ifdef __cplusplus
}
#endif
```
1. create需要返回插件句柄，fb_md_proxy会将其强转为cffex::fb::api::fb_md_plugin_api指针
2. get_md_api_version必须通过IO参数返回mdapi中由宏FEBAO_MD_API_VERSION定义的字符串


### 配置文件
飞豹可以配置启动多个fb_md_proxy进程，一个fb_md_proxy进程可以加个行情插件。假定一个进程启动两个插件，其配置示例如下：
```xml
<fb_md_proxy_1>
    <xlog loglevel="all" />
    <md_log dir="../log/md_log/" />
    <so_path md_plugin_dir="./proxy_plugin/" filter_filename=""/>
    <md_plugins>

        <femas_md>
            <flow_path path="./flow/"/>
            <md_xlog loglevel="all" />
            <addr addr="tcp://xxx.xxx.xxx.xxx:8005"/>
            <accounts brokerid="" userid="" passwd="" encrypt="false" investorid="">
                <item  topicid="111"/>
            </accounts>
        </femas_md>

        <ctp_md>
            <flow_path path="./flow/"/>
            <md_xlog loglevel="warning" />
            <addr addr="tcp://xxx.xxx.xxx.xxx:8005"/>
            <inquiry_mode value="false"/>
            <thread_bind cpu_no="1" />
        </ctp_md>

    </md_plugins>
</fb_md_proxy_1>
```
1. 以femas_md为例，femas_md既是插件femas_md.so的名称，也是插件配置标签的名称，两者需要保持一致
2. fb_md_proxy启动的时候会先读取配置文件中的femas_md和ctp_md，并从proxy_plugin目录中加载femas_md.so和ctp_md.so
3. 加载完成后，建议插件在init回调函数中解析自己的配置，例如femas_md.so解析accounts等配置项


### 绑核参数
每个插件都建议增加绑核配置，以减小延时，具体的绑核配置tag由插件自定义，建议设置为thread_bind
```xml
<thread_bind cpu_no="1" />
```
1. 对于柜台API的绑核，如果API本身没有提供接口，可以在柜台API的回调中每1000条行情检查一次绑核情况
2. 对于由插件创建的线程，可以完全控制


### 目录创建
飞豹有自己的维护脚本，实现了诸如启停，备份，清理等功能。当执行清理功能时，会清理掉fb_md_proxy执行目录下的flow文件夹中的内容，之后飞豹环境处于干净的状态
1. 插件有创建目录及文件需求的话，可以和支持人员讨论，并且创建工作必须由插件自身代码完成
2. 最好不要创建隐藏文件；最好不要设计跨启停周期的文件，不要在两次启动之间保留状态
3. 不要创建日志文件，日志通过spi提供的接口完成
4. 不要创建系统级别的锁及锁文件，保持插件功能单一


### 日志打印
1. 日志可以使用fb_md_helper.h中的MD_XLOG进行打印，可以自定义日志打印宏以简化代码，例如：
```cpp
extern cffex::fb::api::fb_md_xlog_helper *xlog_helper;
#define LOG_FATAL(fmt, ...)   MD_XLOG(xlog_helper, FB_XLOG_FATAL, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)   MD_XLOG(xlog_helper, FB_XLOG_ERROR, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) MD_XLOG(xlog_helper, FB_XLOG_WARNING, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)    MD_XLOG(xlog_helper, FB_XLOG_INFO, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...)   MD_XLOG(xlog_helper, FB_XLOG_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_TRACE(fmt, ...)   MD_XLOG(xlog_helper, FB_XLOG_TRACE, fmt, ##__VA_ARGS__)
#define LOG_ALL(fmt, ...)     MD_XLOG(xlog_helper, FB_XLOG_ALL, fmt, ##__VA_ARGS__)
```

2. 这些日志是同步日志，可以根据需求增加异步日志，但是日志文件必须在febao_kernel/log或其子目录中，方便清理
3. 这些日志打印都是线程安全的

