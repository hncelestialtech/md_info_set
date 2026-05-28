###源文件匹配
csrc = $(wildcard *.c) # 匹配所有 C 文件
cxx_src = $(wildcard *.cpp) # 匹配所有 C++ 文件
cobj = $(patsubst %.c, %.o, $(csrc)) # C 源文件转目标文件
cxx_obj = $(patsubst %.cpp, %.o, $(cxx_src)) # C++ 源文件转目标文件

###编译器配置
cc = gcc # C 编译器
cxx = g++ # C++ 编译器

### 预处理器与编译标志
INCLUDES = ./libs/ # 自定义头文件路径
cppflags = -I./.  -I/usr/include/netinet -I$(INCLUDES) # 强制 C++11 标准
cflags = -c -m64 -mtune=native -O0 -fpermissive -g -mcmodel=medium
# 关键参数说明：
# -O3: 最高优化级别
# -fpermissive: 降低代码严格性（兼容旧代码）
# -mcmodel=medium: 支持大型代码模型


## 连接器配置
ldadd = -lciul -lm -pthread -lrt -lstdc++ -L. -lexanic -lpthread  ## -L指定当前目录为库搜索路径，需确认动态库（如 libexanic.so）存在。
ldflags = -Wl,-E -Wl,-rpath  # 导出符号 + 运行时库路径
## mineldflags = -Wl,-rpath=./ -lcffex_lvl2_lvl2 # 本地运行时路径 ##  设置运行时库路径为当前目录，部署时可能需要调整。





### 设置可执行文件名称
target := cffex_udp_l2_sw

$(target):$(cobj) $(cxx_obj) 
	echo "$(csrc) $(cxx_src)"
	echo "$(cobj) $(cxx_obj)" 
	echo "cc = $(cc)" "cxx = $(cxx)"
	$(cc)  $^  -o $@ $(ldflags) $(ldadd) $(mineldflags)
	

### 编译子规则
### c文件编译
%.o : %.c
	$(cc)  $(cppflags) $(cflags) $^ -o $@

### c++文件编译
cxxflags = -std=c++11 ## 强制使用 C++11 标准（通过 $(cxxflags)）
%.o : %.cpp
	$(cxx) $(cppflags) $(cflags) $(cxxflags) $^ -o $@



#### 清理规则
.PHONY:clean

clean:
	-rm -f *.o
	-rm -f $(target)

