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

#ifndef TRAFFICHANDLER_H
#define TRAFFICHANDLER_H

#include "hls_stream.h"
#include "ap_int.h"
#include "ap_axi_sdata.h"

#ifndef __SYNTHESIS__
#define _DEBUG_EN 1
#endif

#if _DEBUG_EN == 1
#define DEBUG(msg)                    \
    do                                 \
    {                                  \
        std::cout << msg << std::endl; \
    } while(0);
#else
#define DEBUG(msg)
#endif

// filterEnable 
#define CAPTURE_ENABLE (1 << 0)
#define FE_DESTINATION_MAC_ADDRESS (1 << 0)
#define FE_SOURCE_MAC_ADDRESS (1 << 1)
#define FE_PROTOCOL_TYPE (1 << 2)
#define FE_SOURCE_IP_ADDRESS (1 << 3)
#define FE_DESTINATION_IP_ADDRESS (1 << 4)
#define FE_SOURCE_PORT (1 << 5)
#define FE_DESTINATION_PORT (1 << 6)

typedef ap_axiu<512, 1, 0, 0> axiStream_t;

typedef struct 
{
    ap_uint<512> data;
    ap_uint<64> keep;
    ap_uint<1> last;
    ap_uint<1> user;
} axisData_t;

typedef struct 
{
    ap_uint<32> data;
} axisMeta_t;

typedef struct trafficHandlerReg
{
    // common
    ap_uint<32> regUniqueNumber;

    // receiver
    ap_uint<32> regRxPacket;        
    ap_uint<32> regRxBytes;     
    ap_uint<32> regRxDiscard;
    ap_uint<32> regRxDiscardBytes;
    ap_uint<32> regRxCapture;
    ap_uint<32> regRxCaptureBytes;
    // forwarder
    ap_uint<32> regTxPacket;
    ap_uint<32> regTxBytes;
    ap_uint<32> regTxDropPacket;
    ap_uint<32> regTxDropBytes;     
    
    // datamover
    ap_uint<32> regFilteredPacket;
    ap_uint<32> regFilteredPacketBytes; 
    ap_uint<32> regStoredPacket;
    ap_uint<32> regStoredBytes;

} trafficHandlerReg;

// Ethernet II and IPv4 only 
typedef struct networkFilterSettingsReg
{
    ap_uint<32> captureEnable;
    ap_uint<32> filterRules;
    ap_uint<32> destinationMACAddressLower;
    ap_uint<32> destinationMACAddressUpper;
    ap_uint<32> sourceMACAddressLower;
    ap_uint<32> sourceMACAddressUpper;
    ap_uint<32> protocolType;   // 0: UDP, 1: TCP
    ap_uint<32> sourceIPAddress;
    ap_uint<32> destinationIPAddress;
    ap_uint<32> sourcePort;
    ap_uint<32> destinationPort;

} networkFilterSettingsReg;

typedef struct memoryRegister
{
    ap_uint<32> memoryMaxPointerLower;
    ap_uint<32> memoryMaxPointerUpper;
    ap_uint<32> currentMemoryIndex;
} memoryRegister;

typedef struct networkHeader 
{
    ap_uint<48> ethernetDestinationAddress;
    ap_uint<48> ethernetSourceAddress;
    ap_uint<16> ethernetType;
    ap_uint<8> ipVersion;
    ap_uint<8> ipHeaderLength;
    ap_uint<8> ipProtocolType;
    ap_uint<32> ipSourceAddress;
    ap_uint<32> ipDestinationAddress;
    ap_uint<16> SourcePort;
    ap_uint<16> DestinationPort;
} networkHeader;

class trafficMover
{
    public:
        void receiver(ap_uint<32> &regRxPacket,
                      ap_uint<32> &regRxBytes,
                      ap_uint<32> &regRxDiscard,
                      ap_uint<32> &regRxDiscardBytes,
                      ap_uint<32> &regRxCapture,
                      ap_uint<32> &regRxCaptureBytes,
                      ap_uint<32> &captureEnable,
                      hls::stream<axiStream_t> &inputStream,
                      hls::stream<axisData_t> &captureFifo,
                      hls::stream<axisMeta_t> &captureInformation,
                      hls::stream<axisData_t> &outputFifo,
                      hls::stream<axisMeta_t> &outputInformation);

        void forwarder(ap_uint<32> &regTxPacket,
                       ap_uint<32> &regTxBytes,
                       ap_uint<32> &regTxDropPacket,
                       ap_uint<32> &regTxDropBytes,
                       hls::stream<axisMeta_t> &inputInformation,
                       hls::stream<axisData_t> &inputFifo,
                       hls::stream<axiStream_t> &outputStream);
        
        void networkFilter(ap_uint<32> &regFilteredPacket,
                           ap_uint<32> &regFilteredPacketBytes,
                           ap_uint<32> &filterRules,
                           ap_uint<32> &destinationMACAddressLower,
                           ap_uint<32> &destinationMACAddressUpper,
                           ap_uint<32> &sourceMACAddressLower,
                           ap_uint<32> &sourceMACAddressUpper,
                           ap_uint<32> &protocolType,
                           ap_uint<32> &sourceIPAddress,
                           ap_uint<32> &destinationIPAddress,
                           ap_uint<32> &sourcePort,
                           ap_uint<32> &destinationPort,
                           hls::stream<axisData_t> &captureFifo,
                           hls::stream<axisMeta_t> &captureInformation,
                           hls::stream<axisData_t> &dataMoverFifo,
                           hls::stream<axisMeta_t> &dataMoverInformation);

        void dataMover(ap_uint<32> &captureEnable,
                       ap_uint<32> &regStoredPacket,
                       ap_uint<32> &regStoredBytes,
                       ap_uint<32> &memoryMaxPointerLower,
                       ap_uint<32> &memoryMaxPointerUpper,
                       ap_uint<32> &currentMemoryIndex,
                       hls::stream<axisData_t> &dataMoverFifo,
                       hls::stream<axisMeta_t> &dataMoverInformation,
                       ap_uint<512> *memoryOut);

};

#endif // TRAFFICHANDLER_H
