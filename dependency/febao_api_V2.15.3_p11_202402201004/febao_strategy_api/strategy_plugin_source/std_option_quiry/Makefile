VER=1.0.0

EXE=std_option_quiry.so
EXE_D=std_option_quiry_d.so

API_DIR=../../../febao_server/fb_i_strategy

INC_DIR=-I $(API_DIR)/include -I $(API_DIR)/include/caller -I $(API_DIR)/include/filter -I $(API_DIR)/include/stream -I $(API_DIR)/include/trialer -I ../

DIR_LIST = ../../frame/common ../../frame/helper ./src/main ./src/strategy ./src/rule ./src/data_struct

LIBS= $(API_DIR)/libcffex_fb_i_strategy.a
LIBS_D= $(API_DIR)/libcffex_fb_i_strategy_d.a

DFLAGS += -D STD_OPTION_QUIRY

include ../../Makefile_tpl
