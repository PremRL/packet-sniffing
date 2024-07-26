VIVADO := $(XILINX_VIVADO)/bin/vivado
$(XCLBIN)/$(KERNEL).$(DSA).xo: scripts/package_kernel.tcl scripts/gen_xo.tcl \
								const/kernel.xml \
								src/*.v
	mkdir -p $(XCLBIN)
	XPART=$(XPART) $(VIVADO) -mode batch -source scripts/gen_xo.tcl -tclargs $(XCLBIN)/$(KERNEL).$(DSA).xo $(KERNEL) $(TARGET) $(DSA)
