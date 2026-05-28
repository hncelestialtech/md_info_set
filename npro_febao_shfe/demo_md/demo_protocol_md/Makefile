CPP=g++
LINK=g++

CPPFLAGS=-std=c++17 -O2 -Wall -fPIC -Wno-unused-function
LDFLAGS=-lpthread -lrt --shared

INCS=-I../../../fb_md_plugin_api/include -I.
INCS+=-Ifebas_api_v3.08.02/include

SRCS=demo_protocol_md.cc demo_protocol_adapter.cc

OBJS=${SRCS:.cc=.o}
LIBS=

TARGET=demo_protocol_md.so

${TARGET}:${OBJS}
	${LINK} -o $@ ${OBJS} ${LDFLAGS} ${LIBS}

%.o:%.cc
	${CXX} -o $@ ${CPPFLAGS} ${INCS} -c $<

.PHONY: clean
clean:
	-rm ${OBJS}
	-rm ${TARGET}