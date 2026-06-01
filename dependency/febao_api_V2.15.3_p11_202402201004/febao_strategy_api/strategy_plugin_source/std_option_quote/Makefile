VER=1.0.0

EXE=std_option_quote.so
EXE_D=std_option_quote_d.so

API_DIR=../../../febao_server/fb_i_strategy

INC_DIR=-I $(API_DIR)/include -I $(API_DIR)/include/caller -I $(API_DIR)/include/filter -I $(API_DIR)/include/stream -I $(API_DIR)/include/trialer


DIR_LIST = ../../frame/common ../../frame/helper ./src/main ./src/rule ./src/data ./src/strategy

LIBS= $(API_DIR)/libcffex_fb_i_strategy.a
LIBS_D= $(API_DIR)/libcffex_fb_i_strategy_d.a

DFLAGS += -DSTD_OPTION_QUOTE

include ../../Makefile_tpl
