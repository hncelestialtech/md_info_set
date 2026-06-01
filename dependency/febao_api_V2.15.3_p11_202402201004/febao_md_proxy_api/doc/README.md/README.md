<!--
 * @Author: jinfy
 * @Date: 2023-05-09 15:42:40
 * @LastEditors: jinfy
 * @LastEditTime: 2023-05-09 15:42:40
 * @Description:  飞豹行情接入插件
 * @Copyright: Copyright © 2023 中金所数据有限公司 版权所有.
-->

## 项目简介

本项目包含飞豹所需要的全部行情接入插件。可使用飞豹项目中的容器进程 `fb_md_proxy` 加载所需要的行情接入插件 `so` 来接收行情。一个进程可同时加载多个 `so` 进行行情的择优。

行情接入通过本项目进行维护和发布，所管理分支由行情接入api版本决定，行情api变化只受接口改动影响，与数据模型无关。

## 项目结构

```bash
febao_md_plugin/
├── .deps_cpp/                          # 中间件依赖
├── config/                             # 配置文件
├── fb_md_plugin_api/                   # 行情接入api（由febao_server发布）
    └── include/
        ├── fb_md_entity.h
        ├── fb_md_helper.h
        ├── fb_md_plugin_api.h
        ├── fb_md_type.h
        └── fb_md_version.h
├── fb_md_plugins/                      # 各柜台接入独立维护
├── .gitlab-ci.yml                      # CI自动构建stable版本包的配置
├── CHANGELOG.md                        # 版本迭代的变更记录
├── deps_cpp.json                       # 中间件依赖配置
├── install_deps_cpp.py                 # 中间件依赖安装脚本
├── Makefile                            # 版本号生成
├── Makefile_so.tpl                     # 编译文件模板
└── README.md

```

## 当前分支api版本

```c++
#define FEBAO_MD_API_VERSION  "FEBAO_MD_V1.3"

inline void show_md_version() {
printf("febao_md_api_version: [%s] git_info: git_a0a70b2 2023-07-20 15:51:31 +0800 \n",FEBAO_MD_API_VERSION);
};
```

## 行情插件详情

命名规则： 柜台名_md


| 插件名     | 备注                             |
| ---------- | -------------------------------- |
| demo_md           | 接入编写示例           |
| cffex_datafeed_md | 中金所datafeed         |
| ctp_md            | ctp柜台                |
| ctp_mini_md       | ctp_mini               |
| dce_datafeed_md   | 大商所datafeed         |
| es_dstar_md       | 易盛v10                |
| es_md             | 易盛柜台               |
| es_multi_md       | 易盛组播               |
| es_speed_md       | 易盛硬件行情           |
| exsim_md          | 模拟器行情接入         |
| femas_md          | 飞马柜台               |
| femasop_md        | 飞马期权柜台           |
| gfe_datafeed_md   | 广期所datafeed         |
| qdp_md            | 量投柜台               |
| qxgj_cffex_md     | 牵星国君环境中金所行情  |
| qxgj_sse_md       | 牵星国君环境上证所行情  |
| qxgj_nproplus_md     | 牵星国君环境中金所行情  |
| qxkj_szse_fast_md    | 牵星深交所行情      |
| qxgj_szse_sip_md     | 牵星深交所sip行情   |
| qxgj_nproplus_md     | 牵星国君环境中金所行情  |
| qxnh_md           | 牵星南华环境行情  |
| requester_md      | 组播分发  |
| sfe_gmd_md        | 行情平台上期所转发接入  |
| sfe_md            | 上期所交易所二代行情直连  |
| sfe_mirror_md     | 上期所交易所二代行情直连+ctp的mdfront  |
| sfe_mirror2_md    | 上期所交易所二代行情直连+tcp镜像快照的mdfront  |
| sh_md             | 牵星上交所行情  |
| shengli_md        | 盛立柜台  |
| sipui2_md         | 中畅行情  |
| sse_vde_md        | 上交所交易所vde行情直连  |
| szse_md           | 深交所交易所binary行情直连  |
| xspeed_md         | xone柜台  |
| yd_md             | 易达柜台  |