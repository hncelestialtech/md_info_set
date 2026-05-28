需安装好[exanic软件包](https://github.com/cisco/exanic-software)
## 组播地址[addr1] 239.4.1.1:11990
    上交所level2行情(包含委托队列)
    上交所指数行情
    上交所逐笔委托
    上交所逐笔成交
    上交所新债券行情
    上交所逐笔合并
## 组播地址[addr2]239.4.1.2:11991
    深交所level2行情
    深交所指数行情
    深交所逐笔委托
    深交所逐笔成交
    深交所委托队列
## 组播地址[addr3]239.4.1.3:11992
    沪深期权
## 组播地址[addr4]239.4.1.5:11994
    北交所L1快照

## 【解析数据并保存】
### cd 到bin目录, 运行ExanicGtjaLv2Test(debug版) 或 ExanicGtjaLv2Test(release版)
    示例:
    ExanicGtjaLv2Test (不加参数则解析所有数据)
    ExanicGtjaLv2Test addr1 addr2 (可添加不同地址参数，解析对应地址数据)