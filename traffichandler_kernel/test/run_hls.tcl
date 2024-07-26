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

source settings.tcl

set PROJ "prj"
set SOLN "sol"
set CLKP 250MHz
set CASE_ROOT [pwd]
set KERNEL_ROOT "${CASE_ROOT}/../"
set CFLAGS "-std=c++14"

open_project -reset $PROJ

add_files "${KERNEL_ROOT}/traffichandler.cpp" -cflags ${CFLAGS}
add_files "${KERNEL_ROOT}/traffichandler_top.cpp" -cflags ${CFLAGS}
add_files -tb "tb_traffichandler.cpp" -cflags "-I${KERNEL_ROOT} ${CFLAGS}"

set_top trafficHandlerTop

open_solution -reset $SOLN -flow_target vivado
set_part $XPART
create_clock -period $CLKP -name default

if {$CSIM == 1} {
  csim_design
}

if {$CSYNTH == 1} {
  csynth_design
}

if {$COSIM == 1} {
  cosim_design
}

if {$VIVADO_SYN == 1} {
  export_design -flow syn -rtl verilog
}

if {$VIVADO_IMPL == 1} {
  export_design -flow impl -rtl verilog
}

if {$QOR_CHECK == 1} {
  puts "QoR check not implemented yet"
}

exit