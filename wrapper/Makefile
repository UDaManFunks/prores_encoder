BASE_DIR = ../
OBJ_DIR = ./build
CFLAGS = -O3 -I../include -fPIC -std=c++20 -Wall -Wno-unused-variable -Wno-multichar
HEADERS = host_api.h plugin_api.h
SRCS = host_api.cpp plugin_api.cpp
OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

.PHONY: all

all: prereq $(HEADERS) $(SRCS) $(OBJS)

prereq:
	mkdir $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(OBJS)
	rm -rf $(OBJ_DIR)
