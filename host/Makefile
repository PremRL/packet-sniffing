ifndef XILINX_XRT
	XILINX_XRT := "/opt/xilinx/xrt"
endif

XRT_LIB := -lxrt_core -lxrt_coreutil 
INCLUDES := -I$(XILINX_XRT)/include

# Set project directory one level above of Makefile directory. $(CURDIR) is a GNU make variable containing the path to the current working directory
PROJDIR := $(realpath $(CURDIR))
SOURCEDIR := $(PROJDIR)
BUILDDIR := $(PROJDIR)/build
OUTPUTDIR := $(PROJDIR)/../build

ifndef HOST_EXE
	HOST_EXE := host_exe
endif
TARGET := $(HOST_EXE)

# Name the compiler
CXX = g++
CXXFLAGS := -g -std=c++17 -fPIC -pthread -D_REENTRANT $(DEFINES) $(XRT_DEFINES) -Wall -Wextra 
LDFLAGS  := -pthread -L$(XILINX_XRT)/lib  -lstdc++ -lm -lOpenCL -lrt -luuid $(XRT_LIB)

# primary build targets
.PHONY: all clean

all: $(OUTPUTDIR)/$(TARGET)

$(OUTPUTDIR)/$(TARGET): ./host.cpp 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $+ $(LDFLAGS) 
	@echo 'Compiled Host Executable: $(HOST_EXE)'

clean:
	rm -rf $(OUTPUTDIR)/$(TARGET)
