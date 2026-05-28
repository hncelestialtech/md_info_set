CC=g++
target=level1_Demo

.PHONY: all clean

all:$(target)
	
$(target):level1_demo.cpp my_net_interface.cpp
	$(CC) -g -o $@ $^ -lpthread
	
clean:
	rm -rf *.o $(target)
