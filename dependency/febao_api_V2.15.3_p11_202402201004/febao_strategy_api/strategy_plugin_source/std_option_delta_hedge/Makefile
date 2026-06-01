VER=1.0.0

EXE=std_option_delta_hedge.so
EXE_D=std_option_delta_hedge_d.so

API_DIR=../../../febao_server/fb_i_strategy

INC_DIR=-I $(API_DIR)/include -I $(API_DIR)/include/caller -I $(API_DIR)/include/filter -I $(API_DIR)/include/stream -I $(API_DIR)/include/trialer

DIR_LIST = ../../frame/common ./src/main ./src/strategy ./src/rule ./src/data_manager ../../frame/helper

LIBS= $(API_DIR)/libcffex_fb_i_strategy.a
LIBS_D= $(API_DIR)/libcffex_fb_i_strategy_d.a

DFLAGS += -DSTD_OPTION_DELTA_HEDGE

include ../../Makefile_tpl
