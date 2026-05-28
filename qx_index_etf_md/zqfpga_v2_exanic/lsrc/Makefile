.PHONY: debug release clean
VPATH=.
cc=g++
target_d=../bin/ExanicGtjaLv2TestD
target_r=../bin/ExanicGtjaLv2Test
objs=GtjaLv2.o
objs_d=GtjaLv2_d.o
othobjs=-lpthread -lrt -lciul1 -lexanic
flg_i=-I../include -I../common
cflag_d=-std=c++17 -g -DDEBUG
cflag_r=-std=c++17 -O3
path_com=../common
src_com=$(path_com)/cfg.cpp
objs_com=$(path_com)/cfg.o
objs_com_d=$(path_com)/cfg_d.o

$(objs_com):
	$(cc) $(cflag_r) $(flg_i) -c $(src_com) -o $(objs_com)

$(objs_com_d):
	$(cc) $(cflag_d) $(flg_i) -c $(src_com) -o $(objs_com_d)

$(objs):%.o:%.cpp
	$(cc) $(cflag_r) $(flg_i) -c $< -o $@

$(objs_d):%_d.o:%.cpp
	$(cc) $(cflag_d) $(flg_i) -c $< -o $@

debug:$(target_d)
$(target_d):$(objs_d) $(objs_com_d)
	$(cc) $(cflag_d) $(flg_L) $(objs_d) $(objs_com_d) $(othobjs) -LDebug -o $(target_d)
	-rm $(objs_d)

release:$(target_r)
$(target_r):$(objs) $(objs_com)
	$(cc) -O3 $(flg_L) $(objs) $(objs_com) $(othobjs) -LRelease -o $(target_r)
	-rm $(objs)

clean:
	-rm $(objs) $(objs_d) $(objs_com) $(objs_com_d)
	-rm -rf $(target_d) $(target_r)
