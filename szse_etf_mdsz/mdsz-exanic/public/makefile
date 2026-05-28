CC = g++
TARGET = demo
CFLAGS = -O3 -std=c++11 -I./
CDFLAGS = -g -O3 -std=c++11 -I./
LDFLAGS = -L./
LDLIBS = ./libmdszL1Recv.a -lpthread -lexanic
SRC = ./demo.cpp
OBJS=$(SRC:.cpp=.o)
DOBJS=$(SRC:.cpp=.do)
LIBOBJS=$(addprefix $(OBJS_PATH)/,$(SRC:.cpp=.o))

all: clean debug release

debug: $(TARGET)_d

release: $(TARGET)

$(TARGET)_d: $(DOBJS)
	$(CC) $(CDFLAGS) $(LDFLAGS) $(DOBJS) -o $@ $(LDLIBS)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)

$(OBJS):%.o:%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(DOBJS):%.do:%.cpp
	$(CC) $(CDFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET)
	rm -f $(TARGET)_d
	rm -f *.o
	rm -f *.do
	rm -f ./src/*.o
	rm -f ./src/*.do
