BASE_DIR = ../
OBJ_DIR = ./build
CFLAGS = -fPIC -O2 -I../include

.PHONY: all

HEADERS = host_api.h plugin_api.h
SRCS = host_api.cpp plugin_api.cpp
OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

all: $(HEADERS) $(SRCS) $(OBJS)

$(OBJ_DIR)/%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(OBJS)
