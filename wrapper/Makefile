BASE_DIR = ../
OBJ_DIR = ./build
CFLAGS = -O3 -fPIC -I../include
HEADERS = host_api.h plugin_api.h
SRCS = host_api.cpp plugin_api.cpp
OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

.PHONY: all

all: prereq $(HEADERS) $(SRCS) $(OBJS)

prereq:
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm $(OBJS)
	rmdir $(OBJ_DIR)
