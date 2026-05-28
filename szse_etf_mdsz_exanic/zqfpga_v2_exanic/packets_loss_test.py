# -*- coding: utf-8 -*-
"""
Created on Thu Sep  2 10:13:13 2021

@author: Xiaodong, Chen
@email: chenxiaodong024674@gtjas.com
@phone: 18801911881
"""

# 上交所、深交所 逐笔委托/成交 丢包测试
import pandas as pd
import sys

def get_loss_ratio(df, channel, index):
    
    """ 
    丢包测试:
        输入df格式：df 至少包含 channel 和 index 字段
        若index 按照 channel 连续则未丢包
    """
    id_diff = df.groupby(channel)[index].diff().dropna()
    # 理论上 id_diff 全为1则说明没有丢包
    cond = (id_diff > 0)
    if not cond.all():
        err_index = id_diff.loc[~cond].index
        for id_ in err_index:
            print "err loc: %d" % id_
            sys.stdout.flush()
        id_diff = id_diff.loc[cond]
    loss = (id_diff - 1).sum()
    loss_ratio = loss / (len(df) + loss)
    return loss_ratio


# 丢包测试
print "[金桥路行情丢包测试]"
print "上交所逐笔委托...",
sys.stdout.flush()
shse_order = pd.read_csv(r"output_addr1/shse_order.txt", usecols=["委托通道", "委托序号"], sep="\t", error_bad_lines=False)
r1 = get_loss_ratio(shse_order, "委托通道", "委托序号")
print "上交所逐笔委托丢包率{:.6%}".format(r1)
print "上交所逐笔成交...",
sys.stdout.flush()
shse_trade = pd.read_csv(r"output_addr1/shse_trade.txt", usecols=["成交通道", "成交序号"], sep="\t", error_bad_lines=False)
r2 = get_loss_ratio(shse_trade, "成交通道", "成交序号")
print "上交所逐笔成交丢包率{:.6%}".format(r2)
# 深交所需要将逐笔委托和逐笔成交合并后测试
print "深交所逐笔委托/成交...",
sys.stdout.flush()
szse_order = pd.read_csv(r"output_addr2/szse_order.txt", usecols=["通道号", "消息记录号"], sep="\t", error_bad_lines=False)
szse_trade = pd.read_csv(r"output_addr2/szse_trade.txt", usecols=["通道号", "消息记录号"], sep="\t", error_bad_lines=False)
szse = pd.concat([szse_order, szse_trade]).groupby("通道号", group_keys=False).apply(lambda x: x.sort_values(by="消息记录号"))
r3 = get_loss_ratio(szse, "通道号", "消息记录号")
print "深交所逐笔委托/逐笔成交丢包率{:.6%}".format(r3)
