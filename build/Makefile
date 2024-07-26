#
# Copyright 2021 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Makefile for xclbin creation
#

#+-------------------------------------------------------------------------------
# The following parameters are assigned with default values. These parameters can
# be overridden through the make command line
#+-------------------------------------------------------------------------------

REPORT := no
PROFILE := no
DEBUG := no
ETHERNET_DATA_WIDTH := 64

#'estimate' for estimate report generation
#'system' for system report generation
ifneq ($(REPORT), no)
CLFLAGS += --report estimate
LDCLFLAGS += --report system
endif

# generates profile summary report
ifeq ($(PROFILE), yes)
LDCLFLAGS += --profile_kernel data:all:all:all
endif

# generates debug summary report
ifeq ($(DEBUG), yes)
CLFLAGS += --dk protocol:all:all:all
endif

# configuration settings
VPP := $(XILINX_VITIS)/bin/v++

# extract platform name only from full path to xpfm
XILINX_PLATFORM=$(basename $(notdir $(DEVICE)))

# set default hardware target, allows override for hardware emulation (hw_emu)
TARGET ?= hw

# set default Slave Bridge mode, allows overide for DMA variant (DMA)
DM_MODE ?= SB

ifeq ($(TARGET),hw)
PROJECT=packetsniffing
else
$(error Unsupported TARGET=$(TARGET) in DGLLIP project)
endif

ETHERNET_XO := eth_kernel.$(XILINX_PLATFORM).xo
TRAFFICHANDLER_XO := trafficHandlerTop.xo

ETHERNET_XO_FULLPATH=../ethernet_kernel/xclbin/$(ETHERNET_XO)
TRAFFICHANDLER_XO_FULLPATH=../traffichandler_kernel/$(TRAFFICHANDLER_XO)

# shell specific connectivity
ifneq (,$(shell echo $(XILINX_PLATFORM) | awk '/_u55c_gen.*_xdma_/'))
VPPLINKFLAGS := --config u55c.ini
else
$(error Unsupported XILINX_PLATFORM=$(XILINX_PLATFORM))
endif

XODIR = xo
XOS = $(ETHERNET_XO) \
      $(TRAFFICHANDLER_XO) 

CLFLAGS += -t $(TARGET) --platform $(DEVICE) --save-temps --kernel_frequency 250

$(XODIR):
	mkdir $@

# gather up external kernels
xo/$(ETHERNET_XO): $(ETHERNET_XO_FULLPATH) | $(XODIR)
	cp $< $@

xo/$(TRAFFICHANDLER_XO): $(TRAFFICHANDLER_XO_FULLPATH) | $(XODIR)
	cp $< $@

BINARY_CONTAINER = $(PROJECT).xclbin
BINARY_CONTAINER_XOS = $(addprefix $(XODIR)/,$(XOS))

.PHONY: gather-xos
gather-xos: $(BINARY_CONTAINER_XOS)

.PHONY: binary-container
binary-container: $(BINARY_CONTAINER)

$(BINARY_CONTAINER): $(BINARY_CONTAINER_XOS)
	$(VPP) $(CLFLAGS) $(LDCLFLAGS) -l -o $@ $^ $(VPPLINKFLAGS)

.PHONY: clean
clean:
	-rm -f $(PROJECT).xclbin* $(PROJECT).ltx *.log
	-rm -rf .ipcache _x .Xil

realclean: clean
	-rm -rf xo/

.PHONY: help
help:
	@echo "Makefile usage:"
	@echo "  make binary-container  Generate hardware build (xclbin)"
	@echo "  make clean             Clean intermediate build folders"
	@echo "  make realclean         Clean + remove imported kernel objects (xo)"
