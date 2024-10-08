OS_TYPE := $(shell uname -s)
BASE_DIR = ./
OBJ_DIR = ./build
BUILD_DIR = ./bin
FFMPEG_DIR = ../ffmpeg_pkg
WRAPPER_DIR = ./wrapper
CFLAGS = -O3 -Iinclude -Iwrapper -I${FFMPEG_DIR}/include -fPIC -std=c++20 -D__STDC_CONSTANT_MACROS -Wall -Wno-unknown-pragmas -Wno-unused-variable -Wno-multichar
HEADERS = plugin.h uisettings_controller.h prores_worker.h prores_encoder.h prores422_encoder.h proreshq_encoder.h proreslt_encoder.h prorespx_encoder.h
SRCS = plugin.cpp uisettings_controller.cpp prores_worker.cpp prores_encoder.cpp prores422_encoder.cpp proreshq_encoder.cpp proreslt_encoder.cpp prorespx_encoder.cpp
OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
TARGET = $(BUILD_DIR)/prores_encoder.dvcp
CXX = g++

ifeq ($(OS_TYPE), Linux)
LDFLAGS = -fPIC -shared -lpthread -Wl,-Bsymbolic -Wl,--no-undefined -static-libstdc++ -static-libgcc -std=c++20 -lstdc++
else
LDFLAGS = -dynamiclib
endif

LDFLAGS += -L$(FFMPEG_DIR)/lib -lavcodec -lavfilter -lswscale -lswresample -lavutil

.PHONY: all

all: prereq make-subdirs $(HEADERS) $(SRCS) $(OBJS) $(TARGET)

prereq:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR)/%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS)

$(TARGET):
	$(CXX) $(WRAPPER_DIR)/build/*.o $(OBJ_DIR)/*.o $(LDFLAGS) -o $(TARGET)

clean: clean-subdirs
	rm $(OBJS)
	rmdir $(OBJ_DIR)
	rm  $(TARGET)
	rmdir $(BUILD_DIR)

make-subdirs:
	(cd $(WRAPPER_DIR); make; cd ..)

clean-subdirs:
	(cd $(WRAPPER_DIR); make clean; cd ..)
