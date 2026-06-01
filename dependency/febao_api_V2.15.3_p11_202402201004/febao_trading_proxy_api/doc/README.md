## 交易接入2.0版本接口api封装说明文档

### 较1.0交易接入api封装主要改动点

| 改动点       | 改动说明                 |
| ------------ | ------------------------ |
| 查询接口     | 查询接口需传入柜台请求id |
| 配置文件     | 配置文件精简，并有必填项 |
| 异步文件     | 异步文件接口             |
| 通用接口封装 |                          |

### 查询接口

飞豹trading模块启动后会按顺序执行如下查询：

| 查询内容       | 主调函数                 |
| ------------ | ------------------------ |
| 持仓          | req_position              |
| 证券持仓      | req_security_position     |
| 成交          | req_qry_trade             |
| 组保         |    req_qry_comb_detail     |
| 组保仓位占用  | req_qry_comb_strategy_position |

这些查询由飞豹自动发起，且在完成本项查询前不会发起下一项查询，接入程序中需要实现响应的req函数。
自定义交易接入必须实现req_position、req_security_position、req_qry_trade，否则飞豹程序启动会失败
组保、组保仓位占用查询暂不支持自定义实现，fb_trading_proxy_api.so内部仅作空响应
#### 请求

```
    virtual int  req_position(int &request_id) = 0;
    virtual int  req_security_position(int &request_id) = 0;
    virtual int  req_qry_trade(int &request_id) = 0;
```

* 较1.0 新增req_security_position，查询现货持仓接口
* + 参数int &request_id
  + req_xxx(int &request_id)的调用可能由mm_trading_proxy_api.so内部发起，request_id视为出参，用户需要维护
  + req_xxx(int &request_id)调用柜台成功需要返回0，失败返回-3。否则mm_trading_proxy_api.so可能不能正常工作

```
class demo_trading_proxy_adapter : public mm_trading_proxy_callback{
    int request_id_;                // 用户需要维护request_id_
    std::set<int> request_ids_;     // 用户视需求，可选择是否定义维护历史request_id_的集合
}

int demo_trading_api_adapter::req_qry_order(int &request_id) {
    MM_XLOG(MM_XLOG_INFO, "demo_trading_api_adapter::%s, query trade.\n", __FUNCTION__);

    CUstpFtdcQryTradeField ff;  //飞马柜台为例
    memset(&ff, 0, sizeof(ff));
    strcpy(ff.BrokerID, param_.brokerid.c_str());
    strcpy(ff.InvestorID, param_.investorid.c_str());
    strcpy(ff.InstrumentID, "IF2203");
    ReqQryTrade(ff, ++request_id_);
    request_ids_.insert(request_id_);

    return 0;
}
```


####  应答

```
    /* response interface */
    virtual void response_qry_position(int request_id, rtn_position_entity *msg = NULL, bool is_last = true) = 0;
    virtual void response_qry_security_position(int request_id, rtn_security_position_entity *msg = NULL, bool is_last = true) = 0;
    virtual void response_qry_trade(int request_id, rtn_trade_entity *msg = NULL, bool is_last = true) = 0;

```

+ 较1.0，查询的方式内部实现发生变化。1.0请求路径和查询结果是两个不同的数据链路，查询结果走的是发布模式。2.0统一为请求应答模式。

  + 接口中request_id为请求中对应的请求号。如果查询错误或者无查询结果，支持第二个和第三个参数默认返回。

  + 如果有查询数据，最后一条数据需注明is_last为true。通知api内部该查询业务结束。

    ​

```
virtual void response_qry_trade(int request_id, rtn_trade_entity *msg = NULL, bool is_last = true){
    mm_mock_counter_db::TRADE_TABLE::const_iterator itor = counter_.trade_.begin();
    int count = counter_.position_.size();
    if(count == 0) {
        api_->response_qry_trade(request_id);
        return 0;
    }
    for(; itor != counter_.trade_.end(); ++itor) {
        --count;
        if(count == 0) {
            is_last = true;
        }
        const trade_record *r = (*itor);
        rtn_trade_entity *en = rtn_trade_entity::create_entity();
        en->set_instrument_id(r->instrument_id.c_str());
        en->set_exchange_id(r->exchange_id);
        en->set_trade_id(r->trade_id);
        en->set_order_id(r->order_id);
        en->set_order_sys_id(r->order_sys_id.c_str());
        en->set_trade_sys_id(r->trade_sys_id.c_str());
        en->set_direction(r->direction);
        en->set_offset_flag(r->offset_flag);
        en->set_hedge_flag(r->hedge_flag);
        en->set_price(r->price);
        en->set_volume(r->volume);
        en->set_trade_time(r->trade_time);
        api_->response_qry_trade(request_id, en, is_last);
        delete en;
    }
}
```

### 配置文件

​	配置文件相关接口已经封装起来，如日志的级别和文件名。内部请求超时时间等等，api内部都需要根据xml自动匹配。故配置文件需安装下述demo设置。api内部的配置归结到proxy下面。柜台相关的配置归结到counter下面。便于清晰管理。

```
    <mm_demo_trading_proxy_1>
        <xlog loglevel="all" dir="../log/" />
        <flow flow_dir="../flow/"/>
        <async_log>
            <async_monitor logfile="../log/mm_demo_trading_proxy_async_monitor.log"/>
            <requester_responser logfile="../log/mm_demo_trading_proxy_requester_responser.log"/>
            <local_order logfile="../log/mm_demo_trading_proxy_local_order_1.log"/>
            <counter_biz logfile="../log/mm_demo_trading_proxy_counter_biz_1.log"/>
        </async_log>
        <counter>
            <flow_path path="./flow/"/>
            <log request_log="../log/femas.log" response_log="../log/femas.log" />
            <addr front='tcp://172.31.194.195:9002' query=''/>
            <ds   ds_check='1' appid='cffex_febao_001' auth_code='A123456789012345' />
            <accounts>
                  <item brokerid="0001"  userid="febao"  passwd="111111" encrypt="false" investorid="2032004"  seatno=""/>
            </accounts>
        </counter>
        <proxy>
            <flow_path path="./flow/"/>
            <addr addr="tcp://172.31.194.195:8002"/>
            <trading_account_name value="femas1"/>
            <req_timeout value="5"/>
            <outside_self_trade_timeout value="0.5"/>
            <receive_outside value="true"/>
            <req_qry_fund_interval value='5'/>
            <local_id_prefix value="10000000000"/>
        </proxy>
    </mm_demo_trading_proxy_1>

```

+ 必填项包括如下：
  + <xlog loglevel="all" logfile="../log/mm_demo_trading_proxy_1.log" />
  + <flow flow_dir="../flow/"/>，内部发布线程异步流落地路径
  + <proxy>下的必填项
    + 2.0支持多交易账户，trading_account_name设置该交易接入的名称，由合约工具生成交易账户信息上场。
    + req_timeout， 内部请求超时时间限制，用于发给柜台的请求，如果规定时间内无响应，则报错给交易等下游模块。
    + outside_self_trade_timeout，针对外部风控系统的请求超时设置时间
    + req_qry_fund_interval，交易接入查询柜台资金的定时器间隔时间
+ 选填项包括如下：
  + <async_log>，该配置主要用于相关业务的异步落地是否启用，如果配置，则启用相关异步落地文件。各异步文件解释如下：
    + <requester_responser logfile="../log/mm_demo_trading_proxy_requester_responser_1.log"/> 飞豹请求应答数据落地文件，如各查询信息。
    + <local_order logfile="../log/mm_demo_trading_proxy_local_order_1.log"/> 飞豹内部orderid和柜台的loaclid映射关系，相关id生成时，根据文件是否配置来落地与否。
    + <counter_biz logfile="../log/mm_demo_trading_proxy_counter_biz_1.log"/> 用于落地柜台的rsp rtn等所有的柜台数据。
+ 柜台等配置，如飞马main入口读配置

```
//femas param for adapter
    cffex::xml_config_parser *config_parser = cffex::fb::config_parser::get_instance()->get_exe_config();
    cffex::fb::femas_param param;
    param.flow_path         =  config_parser->get_attribute("path",       get_path(node_id, "/counter/flow_path"));
    param.front             =  config_parser->get_attribute("front",  get_path(node_id, "/counter/addr"));
    param.query             =  config_parser->get_attribute("query",  get_path(node_id, "/counter/addr"));
    param.ds_check          =  atoi(config_parser->get_attribute("ds_check",  get_path(node_id, "/counter/ds")).c_str());
    param.appid             =  config_parser->get_attribute("appid",  get_path(node_id, "/counter/ds"));
    param.auth_code         =  config_parser->get_attribute("auth_code",  get_path(node_id, "/counter/ds"));
    param.receive_outside   =  strcmp(config_parser->get_attribute("value",  get_path(node_id, "/proxy/receive_outside")).c_str(), "true") == 0 ? true : false;
    param.exchange_id       =  atoi(config_parser->get_attribute("exchange_id",  get_path(node_id, "/proxy/exchange")).c_str());
    param.request_log       =  config_parser->get_attribute("request_log",  get_path(node_id, "/counter/log"));
    param.response_log      =  config_parser->get_attribute("response_log",  get_path(node_id, "/counter/log"));
    param.local_id_prefix   =  atoll(config_parser->get_attribute("value",  get_path(node_id, "/proxy/local_id_prefix")).c_str());
    std::vector<cffex::xml_config_parser::element> eles = config_parser->get_elements(get_path(node_id, "/counter/accounts/item"));
    for(uint32_t i = 0; i < eles.size(); ++i) {
        param.brokerid       =   config_parser->get_attribute("brokerid",  NULL, eles[i]);
        param.userid         =   config_parser->get_attribute("userid",  NULL, eles[i]);
        param.investorid     =   config_parser->get_attribute("investorid",  NULL, eles[i]);
        param.seatno         =   atoi(config_parser->get_attribute("seatno",  NULL, eles[i]).c_str());
        param.passwd         = cffex::fb::config_parser::parse_passwd(config_parser->get_attribute("passwd",  NULL, eles[i]),
                                                                      config_parser->get_attribute("encrypt",  NULL, eles[i]));
        GXLOG(XLOG_DEBUG, "%s, [%s]\n", __FUNCTION__, param.to_string());
        cffex::fb::femas_trading_api_adapter::get_instance()->init(api_inner, param, node_id);
    }
```

### 新增接口

  新增如下接口。

```
    /* async event*/
    virtual void post_counter_biz_event(const char *buf, int len) = 0;


    /*获取xml的item*/
    virtual std::string get_attribute(const char *name, const char *path) = 0;
```

  ​

+ post_counter_biz_event 柜台的rsp rtn异步逻辑文件。

+ get_attribute，用于读取柜台的配置文件信息。如demo中获取userid和brokerid等信息

```
    std::string front        =  api->get_attribute("front", "mm_demo_trading_proxy_1/counter/addr");
    std::string brokerid        =  api->get_attribute("brokerid", "mm_demo_trading_proxy_1/counter/accounts/item");
    std::string userid        =  api->get_attribute("userid", "mm_demo_trading_proxy_1/counter/accounts/item");

    printf("end! front:%s, brokerid:%s, userid:%s! \n", front.c_str(), brokerid.c_str(), userid.c_str());

```


### 开发易错点

+ 接入登录成功后需要调用mm_local_id_manager::set_trading_day，否则无法生成外部流水号
+ rtn_comb与组合保证金业务的仓位变动无关，仅影响状态；如果组保测试发现可用仓位变化不符合预期，检查是否设置comb_single_position_entity
+ 组合保证金拆分成功，comb_single_position_entity的set_position需要设置为负数
