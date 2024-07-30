# /*******************************************************************************
# (c) Copyright 2019 Xilinx, Inc. All rights reserved.
# This file contains confidential and proprietary information
# of Xilinx, Inc. and is protected under U.S. and
# international copyright and other intellectual property
# laws.
#
# DISCLAIMER
# This disclaimer is not a license and does not grant any
# rights to the materials distributed herewith. Except as
# otherwise provided in a valid license issued to you by
# Xilinx, and to the maximum extent permitted by applicable
# law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
# WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
# AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
# BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
# INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
# (2) Xilinx shall not be liable (whether in contract or tort,
# including negligence, or under any other theory of
# liability) for any loss or damage of any kind or nature
# related to, arising under or in connection with these
# materials, including for any direct, or any indirect,
# special, incidental, or consequential loss or damage
# (including loss of data, profits, goodwill, or any type of
# loss or damage suffered as a result of any action brought
# by a third party) even if such damage or loss was
# reasonably foreseeable or Xilinx had been advised of the
# possibility of the same.
#
# CRITICAL APPLICATIONS
# Xilinx products are not designed or intended to be fail-
# safe, or for use in any application requiring fail-safe
# performance, such as life-support or safety devices or
# systems, Class III medical devices, nuclear facilities,
# applications related to the deployment of airbags, or any
# other applications that could lead to death, personal
# injury, or severe property or environmental damage
# (individually and collectively, "Critical
# Applications"). Customer assumes the sole risk and
# liability of any use of Xilinx products in Critical
# Applications, subject only to applicable laws and
# regulations governing limitations on product liability.
#
# THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
# PART OF THIS FILE AT ALL TIMES.
#
# *******************************************************************************/
set path_to_packaged "./packaged_kernel/${suffix}"
set path_to_tmp_project "./tmp_kernel_pack_${suffix}"

set words [split $device "_"]
set board [lindex $words 1]
set __intfName_0 gt_serial_port_0
set __intfName_1 gt_serial_port_1
set __refclkIntfName_0 gt_refclk_0
set __refclkIntfName_1 gt_refclk_1

#This design is implemented for Alveo U55C
if {([string compare -nocase $board "u55c"] != 0)} {
	puts "Unknown board $board"
	exit	
}
	
set projPart $::env(XPART)
set projName kernel_pack
create_project -force $projName $path_to_tmp_project -part $projPart
set __board [string tolower $board]
add_files -norecurse [glob ./src/eth_kernel.v]

# IP version variables
set cmac_usplus_ver [get_property VERSION [get_ipdefs xilinx.com:ip:cmac_usplus:*]]
set axis_data_fifo_ver [get_property VERSION [get_ipdefs xilinx.com:ip:axis_data_fifo:*]]
set fifo_generator_ver [get_property VERSION [get_ipdefs xilinx.com:ip:fifo_generator:*]]

# CGEMAC 
create_ip -name cmac_usplus -vendor xilinx.com -library ip -version $cmac_usplus_ver -module_name ethernet_0 
set_property -dict { 
    CONFIG.CMAC_CAUI4_MODE {1}
    CONFIG.NUM_LANES {4x25}
    CONFIG.GT_REF_CLK_FREQ {161.1328125}
    CONFIG.USER_INTERFACE {AXIS}
    CONFIG.GT_DRP_CLK {100}
    CONFIG.CMAC_CORE_SELECT {CMACE4_X0Y3}
    CONFIG.GT_GROUP_SELECT {X0Y24~X0Y27}
    CONFIG.LANE1_GT_LOC {X0Y24}
    CONFIG.LANE2_GT_LOC {X0Y25}
    CONFIG.LANE3_GT_LOC {X0Y26}
    CONFIG.LANE4_GT_LOC {X0Y27}
    CONFIG.LANE5_GT_LOC {NA}
    CONFIG.LANE6_GT_LOC {NA}
    CONFIG.LANE7_GT_LOC {NA}
    CONFIG.LANE8_GT_LOC {NA}
    CONFIG.LANE9_GT_LOC {NA}
    CONFIG.LANE10_GT_LOC {NA}
	CONFIG.TX_FLOW_CONTROL {0} 
	CONFIG.RX_FLOW_CONTROL {0}
    CONFIG.INCLUDE_RS_FEC {1}
    CONFIG.ETHERNET_BOARD_INTERFACE {custom}
    CONFIG.DIFFCLK_BOARD_INTERFACE {custom}
    CONFIG.ENABLE_PIPELINE_REG {1}
	CONFIG.Component_Name {ethernet_0}
} [get_ips ethernet_0]

create_ip -name cmac_usplus -vendor xilinx.com -library ip -version $cmac_usplus_ver -module_name ethernet_1 
set_property -dict { 
    CONFIG.CMAC_CAUI4_MODE {1}
    CONFIG.NUM_LANES {4x25}
    CONFIG.GT_REF_CLK_FREQ {161.1328125}
    CONFIG.USER_INTERFACE {AXIS}
    CONFIG.GT_DRP_CLK {100}
    CONFIG.CMAC_CORE_SELECT {CMACE4_X0Y4}
    CONFIG.GT_GROUP_SELECT {X0Y28~X0Y31}
    CONFIG.LANE1_GT_LOC {X0Y28}
    CONFIG.LANE2_GT_LOC {X0Y29}
    CONFIG.LANE3_GT_LOC {X0Y30}
    CONFIG.LANE4_GT_LOC {X0Y31}
    CONFIG.LANE5_GT_LOC {NA}
    CONFIG.LANE6_GT_LOC {NA}
    CONFIG.LANE7_GT_LOC {NA}
    CONFIG.LANE8_GT_LOC {NA}
    CONFIG.LANE9_GT_LOC {NA}
    CONFIG.LANE10_GT_LOC {NA}
	CONFIG.TX_FLOW_CONTROL {0} 
	CONFIG.RX_FLOW_CONTROL {0}
    CONFIG.INCLUDE_RS_FEC {1}
    CONFIG.ETHERNET_BOARD_INTERFACE {custom}
    CONFIG.DIFFCLK_BOARD_INTERFACE {custom}
    CONFIG.ENABLE_PIPELINE_REG {1}
	CONFIG.Component_Name {ethernet_1}
} [get_ips ethernet_1]

# FIFO 
create_ip -name fifo_generator -vendor xilinx.com -library ip -version $fifo_generator_ver -module_name fifo_generator_0
set_property -dict [list CONFIG.Fifo_Implementation {Independent_Clocks_Block_RAM} CONFIG.INTERFACE_TYPE {Native} CONFIG.Performance_Options {First_Word_Fall_Through} CONFIG.Input_Data_Width {578} CONFIG.Input_Depth {512} CONFIG.Output_Data_Width {578} CONFIG.Output_Depth {512} CONFIG.Reset_Pin {true} CONFIG.Enable_Reset_Synchronization {true} CONFIG.Reset_Type {Asynchronous_Reset} CONFIG.Full_Flags_Reset_Value {1} CONFIG.Use_Dout_Reset {true} CONFIG.Data_Count_Width {9} CONFIG.Write_Data_Count_Width {9} CONFIG.Read_Data_Count_Width {9} CONFIG.Full_Threshold_Assert_Value {511} CONFIG.Full_Threshold_Negate_Value {510} CONFIG.Empty_Threshold_Assert_Value {4} CONFIG.Empty_Threshold_Negate_Value {5} CONFIG.Enable_Safety_Circuit {false}] [get_ips fifo_generator_0]

create_ip -name fifo_generator -vendor xilinx.com -library ip -version $fifo_generator_ver -module_name fifo_generator_1 
set_property -dict [list CONFIG.Fifo_Implementation {Independent_Clocks_Distributed_RAM} CONFIG.synchronization_stages {4} CONFIG.Performance_Options {First_Word_Fall_Through} CONFIG.Input_Data_Width {1} CONFIG.Input_Depth {16} CONFIG.Output_Data_Width {1} CONFIG.Output_Depth {16} CONFIG.Use_Embedded_Registers {false} CONFIG.Reset_Type {Asynchronous_Reset} CONFIG.Full_Flags_Reset_Value {1} CONFIG.Data_Count_Width {4} CONFIG.Write_Data_Count_Width {4} CONFIG.Read_Data_Count_Width {4} CONFIG.Full_Threshold_Assert_Value {15} CONFIG.Full_Threshold_Negate_Value {14} CONFIG.Empty_Threshold_Assert_Value {4} CONFIG.Empty_Threshold_Negate_Value {5}] [get_ips fifo_generator_1]

create_ip -name axis_data_fifo -vendor xilinx.com -library ip -version $axis_data_fifo_ver -module_name axis_data_fifo_0 
set_property -dict [list CONFIG.TDATA_NUM_BYTES {64} CONFIG.TUSER_WIDTH {1} CONFIG.FIFO_DEPTH {64} CONFIG.IS_ACLK_ASYNC {1} CONFIG.SYNCHRONIZATION_STAGES {8} CONFIG.HAS_TKEEP {1} CONFIG.HAS_TLAST {1} CONFIG.FIFO_MEMORY_TYPE {block} CONFIG.Component_Name {axis_data_fifo_1}] [get_ips axis_data_fifo_0]

# Creating RTL kernel 
update_compile_order -fileset sources_1
update_compile_order -fileset sim_1
ipx::package_project -root_dir $path_to_packaged -vendor no_vendor -library rtlkernel -version 1.0 -taxonomy /KernelIP -import_files -set_current false
ipx::unload_core $path_to_packaged/component.xml
ipx::edit_ip_in_project -upgrade true -name tmp_edit_project -directory $path_to_packaged $path_to_packaged/component.xml
set_property core_revision 1 [ipx::current_core]
foreach up [ipx::get_user_parameters] {
  ipx::remove_user_parameter [get_property NAME $up] [ipx::current_core]
}
set_property sdx_kernel true [ipx::current_core]
set_property sdx_kernel_type rtl [ipx::current_core]
ipx::create_xgui_files [ipx::current_core]

ipx::add_bus_interface ap_clk [ipx::current_core]
set_property abstraction_type_vlnv xilinx.com:signal:clock_rtl:1.0 [ipx::get_bus_interfaces ap_clk -of_objects [ipx::current_core]]
set_property bus_type_vlnv xilinx.com:signal:clock:1.0 [ipx::get_bus_interfaces ap_clk -of_objects [ipx::current_core]]
ipx::add_port_map CLK [ipx::get_bus_interfaces ap_clk -of_objects [ipx::current_core]]
set_property physical_name ap_clk [ipx::get_port_maps CLK -of_objects [ipx::get_bus_interfaces ap_clk -of_objects [ipx::current_core]]]
ipx::associate_bus_interfaces -busif s_axi_ctrl -clock ap_clk [ipx::current_core]

ipx::add_bus_interface $__intfName_0 [ipx::current_core]
set_property interface_mode master [ipx::get_bus_interfaces $__intfName_0 -of_objects [ipx::current_core]]
set_property abstraction_type_vlnv xilinx.com:interface:gt_rtl:1.0 [ipx::get_bus_interfaces $__intfName_0 -of_objects [ipx::current_core]]
set_property bus_type_vlnv xilinx.com:interface:gt:1.0 [ipx::get_bus_interfaces $__intfName_0 -of_objects [ipx::current_core]]
ipx::add_port_map GRX_P [ipx::get_bus_interfaces $__intfName_0 -of_objects [ipx::current_core]]
set_property physical_name gt_rxp_in_0 [ipx::get_port_maps GRX_P -of_objects [ipx::get_bus_interfaces $__intfName_0 -of_objects [ipx::current_core]]]
ipx::add_port_map GTX_N [ipx::get_bus_interfaces $__intfName_0 -of_objects [ipx::current_core]]
set_property physical_name gt_txn_out_0 [ipx::get_port_maps GTX_N -of_objects [ipx::get_bus_interfaces $__intfName_0 -of_objects [ipx::current_core]]]
ipx::add_port_map GRX_N [ipx::get_bus_interfaces $__intfName_0 -of_objects [ipx::current_core]]
set_property physical_name gt_rxn_in_0 [ipx::get_port_maps GRX_N -of_objects [ipx::get_bus_interfaces $__intfName_0 -of_objects [ipx::current_core]]]
ipx::add_port_map GTX_P [ipx::get_bus_interfaces $__intfName_0 -of_objects [ipx::current_core]]
set_property physical_name gt_txp_out_0 [ipx::get_port_maps GTX_P -of_objects [ipx::get_bus_interfaces $__intfName_0 -of_objects [ipx::current_core]]]

ipx::add_bus_interface $__intfName_1 [ipx::current_core]
set_property interface_mode master [ipx::get_bus_interfaces $__intfName_1 -of_objects [ipx::current_core]]
set_property abstraction_type_vlnv xilinx.com:interface:gt_rtl:1.0 [ipx::get_bus_interfaces $__intfName_1 -of_objects [ipx::current_core]]
set_property bus_type_vlnv xilinx.com:interface:gt:1.0 [ipx::get_bus_interfaces $__intfName_1 -of_objects [ipx::current_core]]
ipx::add_port_map GRX_P [ipx::get_bus_interfaces $__intfName_1 -of_objects [ipx::current_core]]
set_property physical_name gt_rxp_in_1 [ipx::get_port_maps GRX_P -of_objects [ipx::get_bus_interfaces $__intfName_1 -of_objects [ipx::current_core]]]
ipx::add_port_map GTX_N [ipx::get_bus_interfaces $__intfName_1 -of_objects [ipx::current_core]]
set_property physical_name gt_txn_out_1 [ipx::get_port_maps GTX_N -of_objects [ipx::get_bus_interfaces $__intfName_1 -of_objects [ipx::current_core]]]
ipx::add_port_map GRX_N [ipx::get_bus_interfaces $__intfName_1 -of_objects [ipx::current_core]]
set_property physical_name gt_rxn_in_1 [ipx::get_port_maps GRX_N -of_objects [ipx::get_bus_interfaces $__intfName_1 -of_objects [ipx::current_core]]]
ipx::add_port_map GTX_P [ipx::get_bus_interfaces $__intfName_1 -of_objects [ipx::current_core]]
set_property physical_name gt_txp_out_1 [ipx::get_port_maps GTX_P -of_objects [ipx::get_bus_interfaces $__intfName_1 -of_objects [ipx::current_core]]]

# GT Differential Reference Clock

ipx::add_bus_interface $__refclkIntfName_0 [ipx::current_core]
set_property abstraction_type_vlnv xilinx.com:interface:diff_clock_rtl:1.0 [ipx::get_bus_interfaces $__refclkIntfName_0 -of_objects [ipx::current_core]]
set_property bus_type_vlnv xilinx.com:interface:diff_clock:1.0 [ipx::get_bus_interfaces $__refclkIntfName_0 -of_objects [ipx::current_core]]
ipx::add_port_map CLK_P [ipx::get_bus_interfaces $__refclkIntfName_0 -of_objects [ipx::current_core]]
set_property physical_name gt_refclk_p_0 [ipx::get_port_maps CLK_P -of_objects [ipx::get_bus_interfaces $__refclkIntfName_0 -of_objects [ipx::current_core]]]
ipx::add_port_map CLK_N [ipx::get_bus_interfaces $__refclkIntfName_0 -of_objects [ipx::current_core]]
set_property physical_name gt_refclk_n_0 [ipx::get_port_maps CLK_N -of_objects [ipx::get_bus_interfaces $__refclkIntfName_0 -of_objects [ipx::current_core]]]

ipx::add_bus_interface $__refclkIntfName_1 [ipx::current_core]
set_property abstraction_type_vlnv xilinx.com:interface:diff_clock_rtl:1.0 [ipx::get_bus_interfaces $__refclkIntfName_1 -of_objects [ipx::current_core]]
set_property bus_type_vlnv xilinx.com:interface:diff_clock:1.0 [ipx::get_bus_interfaces $__refclkIntfName_1 -of_objects [ipx::current_core]]
ipx::add_port_map CLK_P [ipx::get_bus_interfaces $__refclkIntfName_1 -of_objects [ipx::current_core]]
set_property physical_name gt_refclk_p_1 [ipx::get_port_maps CLK_P -of_objects [ipx::get_bus_interfaces $__refclkIntfName_1 -of_objects [ipx::current_core]]]
ipx::add_port_map CLK_N [ipx::get_bus_interfaces $__refclkIntfName_1 -of_objects [ipx::current_core]]
set_property physical_name gt_refclk_n_1 [ipx::get_port_maps CLK_N -of_objects [ipx::get_bus_interfaces $__refclkIntfName_1 -of_objects [ipx::current_core]]]

# Interface 
ipx::associate_bus_interfaces -busif s_axi_ctrl -clock ap_clk [ipx::current_core]
ipx::associate_bus_interfaces -busif rx0_axis -clock ap_clk [ipx::current_core]
ipx::associate_bus_interfaces -busif rx1_axis -clock ap_clk [ipx::current_core]
ipx::associate_bus_interfaces -busif tx0_axis -clock ap_clk [ipx::current_core]
ipx::associate_bus_interfaces -busif tx1_axis -clock ap_clk [ipx::current_core]

set_property xpm_libraries {XPM_CDC XPM_MEMORY XPM_FIFO} [ipx::current_core]
set_property supported_families { } [ipx::current_core]
set_property auto_family_support_level level_2 [ipx::current_core]
ipx::update_checksums [ipx::current_core]
ipx::save_core [ipx::current_core]
close_project -delete
