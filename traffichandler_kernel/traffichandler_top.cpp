/*
 * Copyright 2021 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "traffichandler_kernel.hpp"

extern "C" void trafficHandlerTop(trafficHandlerReg &trafficReg,
                                  networkFilterSettingsReg &networkReg,
                                  memoryRegister &memoryReg,
                                  hls::stream<axiStream_t> &inputData,
                                  hls::stream<axiStream_t> &outputData, 
                                  ap_uint<512> *memoryOut)
{
#pragma HLS INTERFACE s_axilite port = trafficReg //offset = 0x0100
#pragma HLS INTERFACE s_axilite port = networkReg //offset = 0x0300
#pragma HLS INTERFACE s_axilite port = memoryReg //offset = 0x0400
#pragma HLS INTERFACE s_axilite port = memoryOut //offset = 0x0500
#pragma HLS INTERFACE m_axi max_write_burst_length = 256 bundle = gmem0 port = memoryOut //depth = 1024
#pragma HLS INTERFACE axis port = inputData
#pragma HLS INTERFACE axis port = outputData
#pragma HLS INTERFACE ap_ctrl_none port = return

#pragma HLS DISAGGREGATE variable = trafficReg
#pragma HLS DISAGGREGATE variable = networkReg
#pragma HLS DISAGGREGATE variable = memoryReg
#pragma HLS DATAFLOW disable_start_propagation

    static trafficMover trafficmover;
    static hls::stream<axisData_t> feedData ("feed_data");
    static hls::stream<axisMeta_t> feedMeta ("feed_meta");

    static hls::stream<axisData_t> captureData ("capture_data");
    static hls::stream<axisMeta_t> captureMeta ("capture_meta");

    static hls::stream<axisData_t> filterData ("filter_data");
    static hls::stream<axisMeta_t> filterMeta ("filter_meta");

#pragma HLS STREAM variable=feedData depth=1024
#pragma HLS STREAM variable=feedMeta depth=32
#pragma HLS STREAM variable=captureData depth=1024
#pragma HLS STREAM variable=captureMeta depth=32
#pragma HLS STREAM variable=filterData depth=64
#pragma HLS STREAM variable=filterMeta depth=8

    trafficReg.regUniqueNumber = 0xAABBCCDD;         

    trafficmover.receiver(trafficReg.regRxPacket,
                          trafficReg.regRxBytes,
                          trafficReg.regRxDiscard,
                          trafficReg.regRxDiscardBytes,
                          trafficReg.regRxCapture,
                          trafficReg.regRxCaptureBytes,
                          networkReg.captureEnable,
                          inputData,
                          captureData,
                          captureMeta,
                          feedData,
                          feedMeta);

    trafficmover.forwarder(trafficReg.regTxPacket,
                           trafficReg.regTxBytes,
                           trafficReg.regTxDropPacket,
                           trafficReg.regTxDropBytes,
                           feedMeta,
                           feedData,
                           outputData);
                           
    trafficmover.networkFilter(trafficReg.regFilteredPacket,
                               trafficReg.regFilteredPacketBytes,
                               networkReg.filterRules,
                               networkReg.destinationMACAddressLower,
                               networkReg.destinationMACAddressUpper,
                               networkReg.sourceMACAddressLower,
                               networkReg.sourceMACAddressUpper,
                               networkReg.protocolType,
                               networkReg.sourceIPAddress,
                               networkReg.destinationIPAddress,
                               networkReg.sourcePort,
                               networkReg.destinationPort,
                               captureData,
                               captureMeta,
                               filterData,
                               filterMeta);
                           
    trafficmover.dataMover(networkReg.captureEnable,
                           trafficReg.regStoredPacket,
                           trafficReg.regStoredBytes,
                           memoryReg.memoryMaxPointerLower,
                           memoryReg.memoryMaxPointerUpper,
                           memoryReg.currentMemoryIndex,
                           filterData,
                           filterMeta,
                           memoryOut);

}
