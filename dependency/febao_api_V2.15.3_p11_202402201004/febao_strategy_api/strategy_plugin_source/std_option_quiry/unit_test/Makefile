VER=1.0.0

EXE1=test
DFLAGS += -DDEBUG
DFLAGS += -DSTD_OPTION_QUOTE
DFLAGS += -DMODULE_STRATEGY

API_DIR=../../../../febao_server/mm_i_strategy/
TOOL_DIR=../../../../febao_tools/strategy_unit_test/

DIR_LIST += $(API_DIR)/include  $(API_DIR)/include/caller  $(API_DIR)/include/filter $(API_DIR)/include/stream $(API_DIR)/include/trialer
DIR_LIST += $(TOOL_DIR)/include
GCOV_TARGET_LIST += ../src/*/*.cc ../../../frame/*/*.cc
GCOV_GCDATA_DIRS += ../../../frame/common ../../../frame/helper

GCOV_GCDATA_DIRS += ../src/main ../src/rule  ../src/data_struct ../src/strategy

DIR_LIST += $(GCOV_GCDATA_DIRS)
DIR_LIST += ./src/main ./src/t_src

LIBS= -L../../../../febao_server/mm_i_strategy/lib -lcffex_mm_i_strategy -L../../../../febao_tools/strategy_unit_test -lcffex_strategy_unit_test -lgtest -lpthread


###############################################################
################# common frame ################################
###############################################################
OS=$(shell uname)$(shell uname -a | sed -n '1p' | perl -nle 'print $$1 if /\s+([0-9]\.\d+)/')
GCC=$(shell gcc --version | sed -n '1p' | perl -nle 'print $$1 if /\s+([0-9]\.\d+)/')
VER_PT=$(shell bit=`getconf LONG_BIT`;if [ $$bit -eq 64 ];  then echo 'X64'; else echo 'X32'; fi;)
CC=g++

CODE_LEN_PATH=./src
Build=build
OutPut=$(Build)/self/self/self/self/test

#SOURCE CODE
CC_SRC=$(shell find  $(DIR_LIST)    -name "*.cc" )
CC_SRC2=$(shell find  $(DIR_LIST)   -name "*.c" )
CC_SRC3=$(shell find  $(DIR_LIST)   -name "*.cpp" )

#OBJECTS
CC_OBJS=$(patsubst %.cc,./$(OutPut)/%.o,$(CC_SRC))
CC_OBJS2=$(patsubst %.c,./$(OutPut)/%.o,$(CC_SRC2))
CC_OBJS3=$(patsubst %.cpp,./$(OutPut)/%.o,$(CC_SRC3))
OBJS=$(CC_OBJS) $(CC_OBJS2) $(CC_OBJS3)

define OBJ_MKDIR
  OBJ_DIRS+=./$(OutPut)/$(1)
endef

CC_DIRS=$(shell find $(DIR_LIST) -type d|sed -e '/.svn/d')
$(foreach dir,$(CC_DIRS),$(eval $(call OBJ_MKDIR,$(dir))))

#DEPS
DEPS=$(patsubst %.o,%.d,$(OBJS))

define SAFE_MKDIR
  INC_DIR+=-I $(1)
endef
$(foreach dir,$(DIR_LIST),$(eval $(call SAFE_MKDIR,$(dir))))


ifeq "$(OS)" "Darwin"
	DFLAGS += -DMAC
	CFLAGS=-Wno-invalid-source-encoding -Wno-deprecated-declarations -Wno-undefined-var-template
else
	DFLAGS  += -DLINUX
	LDFLAGS =  -lrt -ldl
	#--export-dynamic
	CFLAGS  = -msse4
endif

DFLAGS += -DTIXML_USE_STL -DGCC
LDFLAGS+=$(LIB_DIR) $(LIBS)
#-Wl
CPPFLAGS+=$(INC_DIR) $(DFLAGS) -std=c++0x
#CFLAGS += -g -O3 -fPIC -Wall -Wno-unused-function -Wno-strict-aliasing
CFLAGS += -g -fPIC -Wall -Wno-unused-function -Wno-strict-aliasing
#CFLAGS += -g -fPIC -Wall -Wno-unused-function -Wno-strict-aliasing

# ### GCOV ###
GCOV=gcov
LCOV=lcov
GENHTML=genhtml
ifeq ($(TEST_GCOV),1)
	GCOV_FLAGS=-fprofile-arcs -ftest-coverage
	CFLAGS   += $(GCOV_FLAGS)
	CPPFLAGS += $(GCOV_FLAGS)
endif

define GCOV_OBJ_MKDIR
  GCOV_O_FLAGS+=-o ./$(OutPut)/$(1)
endef
$(foreach dir,$(GCOV_GCDATA_DIRS),$(eval $(call GCOV_OBJ_MKDIR,$(dir))))
# ### GCOV ###

$(warning "the value of OBJS is $(OBJS)")

all:$(EXE1)
$(shell mkdir -p $(sort $(OBJ_DIRS)))
-include $(DEPS)

$(EXE1):$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(EXE1_A):$(OBJS)
	ar -rc $@ $(shell find $(Build) -name "*.o")

./$(OutPut)/%.o:%.cc
	$(CC) -o $@ $(CFLAGS) -c $< $(CPPFLAGS)
./$(OutPut)/%.d:%.cc
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(patsubst %.d,%.o,$@) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

./$(OutPut)/%.o:%.cpp
	$(CC) -o $@ $(CFLAGS) -c $< $(CPPFLAGS)
./$(OutPut)/%.d:%.cpp
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(patsubst %.d,%.o,$@) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

./$(OutPut)/%.o:%.c
	$(CC) -o $@ $(CFLAGS) -c $< $(CPPFLAGS)
./$(OutPut)/%.d:%.c
	@set -e; rm -f $@; \


	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(patsubst %.d,%.o,$@) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

clean:
	rm -rf build
	rm -rf $(EXE1)
codelen:
	find $(CODE_LEN_PATH) \( -name "*.cc"  -name "*.cpp" -o -name "*.h" -o -name "*.c" \) -exec cat {} \;|sed -e 's/\"/\n\"\n/g;s/\([^\/]\)\(\/\*\)/\1\n\2\n/g;'|sed  -e '/^\"/{:a;N;/\".*\"/!ba;s/\".*\".*//;N;/\"/!d;b}' -e '/^\/\*/{s/\/\*.*\*\///;/\/\*/{:b;N;/\/\*.*\*\//!bb;s/\/\*.*\*\///}}' -e 's/\/\/.*//g' |sed -e '/^[[:space:]]*$$/d'|wc -l
srczip:
	zip -r ./$(EXE1)_src_$(VER).zip * -x *.o *.d *.svn *.zip *.a *.so $(EXE1) *.svn-work *.svn-base *.so.* *.d.* *.svn

release:
	rm -rf release
	mkdir release
	mkdir -p release/lib
	cp -r include release/
	cp -r *.a release/lib

gcov:
	#echo "$(GCOV) $(GCOV_TARGET_LIST) $(GCOV_O_FLAGS)"
	$(GCOV) $(GCOV_TARGET_LIST) $(GCOV_O_FLAGS)
	$(LCOV) -c -d . -o test.info
	$(GENHTML) -o gcov_result test.info
