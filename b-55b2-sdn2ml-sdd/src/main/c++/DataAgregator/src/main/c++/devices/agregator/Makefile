CXX = g++

# ONNX Runtime
ONNX_LIB_DIR_REL = ./../../../../../../onnxruntime-linux-x64-1.21.0/lib
ONNX_LIB_DIR     := $(abspath $(ONNX_LIB_DIR_REL))
ONNX_INCLUDE_DIR_REL = ./../../../../../../onnxruntime-linux-x64-1.21.0/include
ONNX_INCLUDE_DIR     := $(abspath $(ONNX_INCLUDE_DIR_REL))

# CODAC / NDS3
CODAC_INCLUDE_DIR := $(if $(CODAC_ROOT),$(CODAC_ROOT)/include,/opt/codac/include)
CODAC_LIB_DIR     := $(if $(CODAC_ROOT),$(CODAC_ROOT)/lib,/opt/codac/lib)



# --- Первый файл ---
MODEL1_FILE := ./BP20_noMS_DT25_y100_rev_vgg_ep1000_mae_r1000_r1_om_vs_vg.onnx
MODEL1_PATH := $(abspath $(MODEL1_FILE))

# --- Второй файл ---
MODEL2_FILE := ./xgb_1_fp_v0_1.onnx
MODEL2_PATH := $(abspath $(MODEL2_FILE))



CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -fPIC -pthread -DNDS3_DLL -I../../include -I$(ONNX_INCLUDE_DIR) -I$(CODAC_INCLUDE_DIR) -g 

CXXFLAGS += -DMODEL1_PATH=\"$(MODEL1_PATH)\"
CXXFLAGS += -DMODEL2_PATH=\"$(MODEL2_PATH)\"



LDFLAGS = -L../../../../lib -L$(CODAC_LIB_DIR) -L$(ONNX_LIB_DIR) \
          -lonnxruntime -pthread -ldl -Wl,-rpath,$(ONNX_LIB_DIR)

TARGETDIR = ../../../../lib
# Flags passed to gcc during linking
LINK = -shared -fPIC 
 
# Name of the nds library
TARGET = libnds3-DataAgregator.so
 
# Additional linker libraries
LIBS = -lnds3 
 
# Source code files used in this project
SRCS = DataAgregator.cpp 
 
OBJS = $(SRCS:.cpp=.o)

all: $(TARGETDIR)/$(TARGET)

# Rules for building
$(TARGETDIR)/$(TARGET): $(OBJS)
	mkdir -p $(TARGETDIR)
	$(CXX) $(LINK) $(LDFLAGS) -o $@ $^ $(LIBS)
 
.PHONY: clean
clean:
	rm -rf $(TARGET)
 
