OS_TYPE := $(shell uname -s)
BASE_DIR = ./
OBJ_DIR = ./build
BUILD_DIR = ./bin
X265_DIR = ../x265
CFLAGS = -O2 -fPIC -Iinclude -Iwrapper -I$(X265_DIR)/source

ifeq ($(OS_TYPE), Linux)
LDFLAGS = -shared -lpthread
else
LDFLAGS = -dynamiclib
endif

TARGET = $(BUILD_DIR)/x265_encoder.dvcp5
LDFLAGS += -L$(X265_DIR)/ -lx265_static.lib

.PHONY: all

HEADERS = plugin.h x265_encoder.h
SRCS = plugin.cpp ui_settings_controller.cpp x265_encoder.cpp 
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
