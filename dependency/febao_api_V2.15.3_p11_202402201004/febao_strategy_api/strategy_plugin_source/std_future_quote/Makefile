VER=1.0.0

EXE=std_future_quote.so
EXE_D=std_future_quote_d.so


API_DIR=../../../febao_server/fb_i_strategy

INC_DIR=-I $(API_DIR)/include -I $(API_DIR)/include/caller -I $(API_DIR)/include/filter -I $(API_DIR)/include/stream -I $(API_DIR)/include/trialer

DIR_LIST = ../../frame/common ./src/main ./src/rule ./src/data ./src/strategy ../../frame/helper

LIBS= $(API_DIR)/libcffex_fb_i_strategy.a
LIBS_D= $(API_DIR)/libcffex_fb_i_strategy_d.a

DFLAGS += -DSTD_FUTURE_QUOTE

include ../../Makefile_tpl
