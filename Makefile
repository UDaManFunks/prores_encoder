OS_TYPE := $(shell uname -s)
BASE_DIR = ./
OBJ_DIR = ./build
BUILD_DIR = ./bin
X264_DIR = ../x264
CFLAGS = -O3 -Iinclude -Iwrapper -I$(X264_DIR) -Wall -Wno-unused-variable -Wno-multichar -fPIC -std=c++20

ifeq ($(OS_TYPE), Linux)
LDFLAGS = -shared -lpthread
else
LDFLAGS = -dynamiclib
endif

TARGET = $(BUILD_DIR)/x264_encoder.dvcp
LDFLAGS += -L$(X264_DIR) -lx264

.PHONY: all

HEADERS = plugin.h x264_encoder.h x264_encoder_main.h x264_encoder_high.h x264_encoder_h422.h
SRCS = plugin.cpp x264_encoder.cpp x264_encoder_main.cpp x264_encoder_high.cpp x264_encoder_h422.cpp
OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

all: prereq make-subdirs $(HEADERS) $(SRCS) $(OBJS) $(TARGET)

prereq:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR)/%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET):
	$(CC) $(OBJ_DIR)/*.o $(LDFLAGS) -o $(TARGET)

clean: clean-subdirs
	rm -rf $(OBJ_DIR)
	rm -rf $(BUILD_DIR)

make-subdirs:
	(cd wrapper; make; cd ..)

clean-subdirs:
	(cd wrapper; make clean; cd ..)
