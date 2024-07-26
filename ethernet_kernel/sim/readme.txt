I used Vivado RTL simulation for these modules to check their functionalities.

If you want to run them, please kindly create your own Vivado project, 
include these test bench files into the project, run the following TCL commands, and start the simulation.


create_ip -name axis_data_fifo -vendor xilinx.com -library ip -version 2.0 -module_name axis_data_fifo_0
set_property -dict [list CONFIG.TDATA_NUM_BYTES {64} CONFIG.TUSER_WIDTH {1} CONFIG.FIFO_MODE {2} CONFIG.IS_ACLK_ASYNC {1} CONFIG.HAS_TKEEP {1} CONFIG.HAS_TLAST {1} CONFIG.FIFO_MEMORY_TYPE {block}] [get_ips axis_data_fifo_0]

create_ip -name axis_data_fifo -vendor xilinx.com -library ip -version 2.0 -module_name axis_data_fifo_1 
set_property -dict [list CONFIG.TDATA_NUM_BYTES {64} CONFIG.TUSER_WIDTH {1} CONFIG.FIFO_DEPTH {64} CONFIG.IS_ACLK_ASYNC {1} CONFIG.HAS_TKEEP {1} CONFIG.HAS_TLAST {1} CONFIG.FIFO_MEMORY_TYPE {block} CONFIG.Component_Name {axis_data_fifo_1}] [get_ips axis_data_fifo_1]

