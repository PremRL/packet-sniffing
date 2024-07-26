#include <iostream>
#include "traffichandler.hpp"


void trafficMover::receiver(ap_uint<32> &regRxPacket,
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
                            hls::stream<axisMeta_t> &outputInformation)
{
#pragma HLS pipeline II = 1 style = flp

    axiStream_t dataIn;
    axisData_t writeData;
    axisMeta_t informationOut;

    enum stateType {
        WAIT_NEW_FRAME,
        STORE_FRAME,
        DISCARD_FRAME,
        WRITE_INFORMATION
    };
    static stateType state = WAIT_NEW_FRAME;

    enum captureType {
        NEW_PACKET_ALLOWED,
        STORING_DATA,
        DISCARDING_PACKET,
        WRITING_CONTROL
    };
    static captureType captureStatus = NEW_PACKET_ALLOWED;

    static ap_uint<16> length;
    static ap_uint<16> numberOfKeeps;
    static bool isInsufficientMemory;
    static bool isInsufficientMeta;
    static bool isFrameTooLarge;
    static bool isFrameCRCError;
    static bool isProcessLast;

    static ap_uint<32> countRxPacket = 0;
    static ap_uint<32> countRxBytes = 0;
    static ap_uint<32> countRxDiscard = 0;
    static ap_uint<32> countRxDiscardBytes = 0;
    static ap_uint<32> countRxCapture = 0;
    static ap_uint<32> countRxCaptureBytes = 0;

    switch(state)
    {   
        case WAIT_NEW_FRAME:
            // clearing the captured frame before processing the new one 
            if (captureStatus == DISCARDING_PACKET) {
                if (!captureFifo.full()) {
                    writeData.data = 0;
                    writeData.keep = 0;
                    writeData.last = 1;
                    captureFifo.write(writeData);
                    captureStatus = WRITING_CONTROL;
                }
            }

            if (captureStatus == WRITING_CONTROL) {
                if (!captureInformation.full()) {
                    informationOut.data = 0;
                    captureInformation.write(informationOut);
                    captureStatus = NEW_PACKET_ALLOWED;
                    DEBUG("Discard the capturing frame due to memory problems");
                }
            }
            
            // processing the incoming frame no matter what happens!
            if (!inputStream.empty()) {
                isInsufficientMemory = false;  
                isInsufficientMeta = false;
                isFrameTooLarge = false;
                isFrameCRCError = false;
                isProcessLast = false;

                // Should be able to store at least 150 words (9600 bytes) 
                if (outputFifo.full()) {
                    isInsufficientMemory = true; 
                }

                // Should be able to store at least 1 meta control chunk
                if (outputInformation.full()) {
                    isInsufficientMeta = true; 
                }

                length = 0;

                if (!isInsufficientMemory && !isInsufficientMeta) {
                    state = STORE_FRAME;
                    DEBUG("received the start and going to STORE_FRAME");
                    if (captureStatus == NEW_PACKET_ALLOWED && ((captureEnable & CAPTURE_ENABLE) != 0)) {
                        DEBUG("started packet capturing of this frame");
                        captureStatus = STORING_DATA;
                    }
                } else {
                    state = DISCARD_FRAME;
                    DEBUG("received the start and going to DISCARD_FRAME");
                } 
            }
            break;

        case STORE_FRAME:
            if (!inputStream.empty()) {
                inputStream.read(dataIn);

                // need to check whether it's slow or not
                numberOfKeeps = 0;
                for (int i = 0; i < 64; i++) {
                    if (dataIn.keep.range(i, i) == 1) {
                        ++numberOfKeeps;
                    }
                }
                length += numberOfKeeps;

                if (dataIn.last) {
                    isProcessLast = true;
                    state = WRITE_INFORMATION;
                    if (dataIn.user) {
                        isFrameCRCError = true;
                    }
                }

                if (outputFifo.full()) {
                    isInsufficientMemory = true;
                    if (!isProcessLast) {
                        state = DISCARD_FRAME;
                    }
                } else if (length > 9014) {
                    isFrameTooLarge = true;
                    if (!isProcessLast) {
                        state = DISCARD_FRAME;
                    }
                } 
                writeData.data = dataIn.data;
                writeData.keep = dataIn.keep;
                writeData.last = dataIn.last;
                outputFifo.write(writeData);

                // sequence to store the frame to be captured
                if (captureStatus == STORING_DATA) {
                    if (!captureFifo.full()) {
                        captureFifo.write(writeData);
                    } else {
                        if (dataIn.last) {
                            captureStatus = WRITING_CONTROL;
                        } else {
                            captureStatus = DISCARDING_PACKET;
                        }
                    }
                }
            }
            break;

        case DISCARD_FRAME:
            if (!inputStream.empty()) {
                // if the frame is already discarded
                if (captureStatus == STORING_DATA) {
                    captureStatus = DISCARDING_PACKET;
                }

                inputStream.read(dataIn);

                numberOfKeeps = 0;
                for (int i = 0; i < 64; i++) {
                    if (dataIn.keep.range(i, i) == 1) {
                        ++numberOfKeeps;
                    }
                }
                length += numberOfKeeps;

                if (dataIn.last) {
                    DEBUG("received an END frame of the discarded one");
                    // always write the last data 
                    writeData.data = dataIn.data;
                    writeData.keep = dataIn.keep;
                    writeData.last = dataIn.last;
                    outputFifo.write(writeData);
                    state = WRITE_INFORMATION;
                }
            }
            break;
        
        case WRITE_INFORMATION:
            if (!outputInformation.full()) {
                if (isInsufficientMemory || isInsufficientMeta || isFrameTooLarge || isFrameCRCError) {
                    DEBUG("write the control memory to not forwarding the frame");
                    informationOut.data.range(16, 16) = 0;
                    informationOut.data.range(15, 0) = length;
                    ++countRxDiscard;
                    countRxDiscardBytes += length;
                } else {
                    DEBUG("write to outputInformation");
                    informationOut.data.range(16, 16) = 1;
                    informationOut.data.range(15, 0) = length;
                    ++countRxPacket;
                    countRxBytes += length;
                }
                outputInformation.write(informationOut);
                state = WAIT_NEW_FRAME;

                // sequence to store the frame to be captured
                if (captureStatus == STORING_DATA) {
                    if ((!captureInformation.full()) && (informationOut.data.range(16, 16) == 1)) {
                        DEBUG("Captured the frame to be stored in the memory");
                        captureInformation.write(informationOut);
                        ++countRxCapture;
                        countRxCaptureBytes += length;
                        captureStatus = NEW_PACKET_ALLOWED;
                    } else {
                        captureStatus = WRITING_CONTROL;
                    }
                }
            }
            break;
    }
    
    regRxPacket = countRxPacket;
    regRxBytes = countRxBytes;
    regRxDiscard = countRxDiscard;
    regRxDiscardBytes = countRxDiscardBytes; 
    regRxCapture = countRxCapture;
    regRxCaptureBytes = countRxCaptureBytes;
}

void trafficMover::forwarder(ap_uint<32> &regTxPacket,
                             ap_uint<32> &regTxBytes,
                             ap_uint<32> &regTxDropPacket,
                             ap_uint<32> &regTxDropBytes,
                             hls::stream<axisMeta_t> &inputInformation,
                             hls::stream<axisData_t> &inputFifo,
                             hls::stream<axiStream_t> &outputStream)
{  
#pragma HLS pipeline II = 1 style = flp

    axisData_t currentData;
    axiStream_t outputData;
    axisMeta_t informationIn;

    enum stateType {
        READ_INFORMATION,
        ECHO_FRAME,
        DROP_FRAME
    };
    static stateType state = READ_INFORMATION;

    static ap_uint<1> forward;
    static ap_uint<16> length;

    static ap_uint<32> countTxPacket = 0;
    static ap_uint<32> countTxBytes = 0;
    static ap_uint<32> countTxDropPacket = 0;
    static ap_uint<32> countTxDropPacketBytes = 0;

    switch(state)
    {   
        case READ_INFORMATION:
            if (!inputInformation.empty()) {
                inputInformation.read(informationIn);
                forward = informationIn.data.range(16, 16);
                length = informationIn.data.range(15, 0);
                if (forward == 1) {
                    DEBUG("transmit the received frame");
                    state = ECHO_FRAME;
                } else {
                    DEBUG("not transmit the received frame");
                    state = DROP_FRAME;
                } 
            }
            break;

        case ECHO_FRAME:
            // inputFifo is designed to be always ready in this state
            if (!inputFifo.empty() && !outputStream.full())
            {
                inputFifo.read(currentData);
                outputData.data = currentData.data;
                outputData.keep = currentData.keep;
                outputData.last = currentData.last;
                outputStream.write(outputData);
                if (currentData.last) {
                    DEBUG("complete the frame transmitting");
                    state = READ_INFORMATION;
                    ++countTxPacket;
                    countTxBytes += length;
                }
            }
            break;
            
        case DROP_FRAME:
            if (!inputFifo.empty())
            {
                inputFifo.read(currentData);
                if (currentData.last) {
                    DEBUG("complete the frame discarding");
                    state = READ_INFORMATION;
                    ++countTxDropPacket;
                    countTxDropPacketBytes += length; 
                }
            }
            break;
    }

    regTxPacket = countTxPacket;
    regTxBytes = countTxBytes;
    regTxDropPacket = countTxDropPacket;
    regTxDropBytes = countTxDropPacketBytes;

}

void trafficMover::networkFilter(ap_uint<32> &regFilteredPacket,
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
                                 hls::stream<axisMeta_t> &dataMoverInformation)
{  
#pragma HLS pipeline II = 1 style = flp

    static axisData_t currentData0, currentData1;
    static axisMeta_t informationIn, informationOut;

    enum stateType {
        WAIT_CONTROL,
        FILTERING0,
        FILTERING1,
        STORE_DATA_HEADERS0,
        STORE_DATA_HEADERS1,
        STORE_DATA,
        STORE_COMPLETE,
        DISCARD_DATA
    };
    static stateType state = WAIT_CONTROL;

    static ap_uint<1> continueToProcess;
    static ap_uint<1> readSecondEnable;
    static ap_uint<1> last;
    static networkHeader header;
    static ap_uint<1024> dataHeader;
    static ap_uint<32> headerLength;

    static ap_uint<32> filterRulesStable;
    static ap_uint<64> destinationMACAddressStable;
    static ap_uint<64> sourceMACAddressStable;
    static ap_uint<32> protocolTypeStable;
    static ap_uint<32> sourceIPAddressStable;
    static ap_uint<32> destinationIPAddressStable;
    static ap_uint<32> sourcePortStable;
    static ap_uint<32> destinationPortStable;
    
    static ap_uint<32> countFilteredPacket = 0;
    static ap_uint<32> countFilteredPacketBytes = 0;

    switch (state) 
    {
        case WAIT_CONTROL:
            // before the operation, load all parameters 
            filterRulesStable = filterRules;
            destinationMACAddressStable.range(63, 32) = destinationMACAddressUpper;
            destinationMACAddressStable.range(31, 0) = destinationMACAddressLower;
            sourceMACAddressStable.range(63, 32) = sourceMACAddressUpper;
            sourceMACAddressStable.range(31, 0) = sourceMACAddressLower;
            protocolTypeStable = protocolType;
            sourceIPAddressStable = sourceIPAddress;
            destinationIPAddressStable = destinationIPAddress;
            sourcePortStable = sourcePort;
            destinationPortStable = destinationPort;
            if (!captureInformation.empty() && !captureFifo.empty()) {
                captureInformation.read(informationIn); // control
                captureFifo.read(currentData0); // data 
                state = FILTERING0;
            }
            break;

        case FILTERING0:
            DEBUG("start network filtering of the captured frame");

            // continue to process the frame 
            continueToProcess = ((informationIn.data & 0x10000) != 0);

            // ethernet header
            if (continueToProcess) {
                dataHeader.range(111, 0) = currentData0.data.range(111, 0);
                for (int i = 0; i < 6; i++) {
                    header.ethernetDestinationAddress.range(8*i+7, 8*i) = dataHeader.range(47-8*i, 40-8*i);
                    header.ethernetSourceAddress.range(8*i+7, 8*i) = dataHeader.range(95-8*i, 88-8*i);
                }
                for (int i = 0; i < 2; i++) {
                    header.ethernetType.range(8*i+7, 8*i) = dataHeader.range(111-8*i, 104-8*i);
                }

                // process only IPv4 packets 
                if (header.ethernetType != 0x0800) {
                    continueToProcess = false;
                }
                headerLength = 14;
            }

            // ip header
            if (continueToProcess) {
                dataHeader.range(159, 0) = currentData0.data.range(271, 112); // 20 bytes
                header.ipVersion = dataHeader.range(7, 4); // must be 4  
                header.ipHeaderLength = dataHeader.range(3, 0) << 2; // range must be in 20 and 60 bytes    

                if (header.ipVersion != 4) {
                    continueToProcess = false;
                }

                if ((header.ipHeaderLength < 20) || (header.ipHeaderLength > 60)) {
                    continueToProcess = false;
                }

                header.ipProtocolType = dataHeader.range(79, 72);

                for (int i = 0; i < 4; i++) {
                    header.ipSourceAddress.range(8*i+7, 8*i) = dataHeader.range(127-8*i, 120-8*i);
                    header.ipDestinationAddress.range(8*i+7, 8*i) = dataHeader.range(159-8*i, 152-8*i);
                }
                headerLength += header.ipHeaderLength;
            }

            // load more data since the ip header is too long
            readSecondEnable = false;
            if (continueToProcess) {
                if (header.ipProtocolType == 6) { // tcp packet 
                    if ((headerLength + 20) > 64) {
                        readSecondEnable = true;
                    }
                }

                if (header.ipProtocolType == 17) { // tcp packet 
                    if ((headerLength + 8) > 64) {
                        readSecondEnable = true;
                    }
                }
            }  

            state = FILTERING1;
            break;

        case FILTERING1:
            if (!readSecondEnable || !captureFifo.empty()) {
                if (continueToProcess && readSecondEnable) {
                    if (currentData0.last) {
                        continueToProcess = false;
                    } else {
                        captureFifo.read(currentData1); // data 
                    }
                }

                // transport header (ignoring tcp option)
                if (continueToProcess && ((header.ipProtocolType == 6) || (header.ipProtocolType == 17))) {
                    dataHeader.range(1023, 512) = currentData1.data;
                    dataHeader.range(511, 0) = currentData0.data;
                    switch (header.ipHeaderLength) {
                        case 20: dataHeader = dataHeader >> (112 + 160 + (32 * 0)); break;
                        case 24: dataHeader = dataHeader >> (112 + 160 + (32 * 1)); break; 
                        case 28: dataHeader = dataHeader >> (112 + 160 + (32 * 2)); break;
                        case 32: dataHeader = dataHeader >> (112 + 160 + (32 * 3)); break;
                        case 36: dataHeader = dataHeader >> (112 + 160 + (32 * 4)); break;
                        case 40: dataHeader = dataHeader >> (112 + 160 + (32 * 5)); break;
                        case 44: dataHeader = dataHeader >> (112 + 160 + (32 * 6)); break;
                        case 48: dataHeader = dataHeader >> (112 + 160 + (32 * 7)); break;
                        case 52: dataHeader = dataHeader >> (112 + 160 + (32 * 8)); break;
                        case 56: dataHeader = dataHeader >> (112 + 160 + (32 * 9)); break;
                        default: dataHeader = dataHeader >> (112 + 160 + (32 * 10)); break;
                    }

                    // either udp or tcp
                    for (int i = 0; i < 2; i++) {
                        header.SourcePort.range(8*i+7, 8*i) = dataHeader.range(15-8*i, 8-8*i);
                        header.DestinationPort.range(8*i+7, 8*i) = dataHeader.range(31-8*i, 24-8*i);
                    }
                }    


                // network filtering //


                if (continueToProcess && ((filterRulesStable & FE_DESTINATION_MAC_ADDRESS) != 0)) {
                    if (destinationMACAddressStable != header.ethernetDestinationAddress) {
                        continueToProcess = false;
                        DEBUG("network filtered at destination mac part");
                    }
                }
                
                if (continueToProcess && ((filterRulesStable & FE_SOURCE_MAC_ADDRESS) != 0)) {
                    if (sourceMACAddressStable != header.ethernetSourceAddress) {
                        continueToProcess = false;
                        DEBUG("network filtered at source mac part");
                    }
                }
            
                if (continueToProcess && ((filterRulesStable & FE_PROTOCOL_TYPE) != 0)) {
                    if ((protocolTypeStable == 0) && (header.ipProtocolType != 17)) {
                        continueToProcess = false;
                        DEBUG("network filtered at protocol type part");
                    }
                    if ((protocolTypeStable == 1) && (header.ipProtocolType != 6)) {
                        continueToProcess = false;
                        DEBUG("network filtered at protocol type part");
                    }
                }
                
                if (continueToProcess && ((filterRulesStable & FE_SOURCE_IP_ADDRESS) != 0)) {
                    if (sourceIPAddressStable != header.ipSourceAddress) {
                        continueToProcess = false;
                        DEBUG("network filtered at source ip part");
                    }
                }
                
                if (continueToProcess && ((filterRulesStable & FE_DESTINATION_IP_ADDRESS) != 0)) {
                    if (destinationIPAddressStable != header.ipDestinationAddress) {
                        continueToProcess = false;
                        DEBUG("network filtered at destination ip part");
                    }
                }
                
                if (continueToProcess && ((filterRulesStable & FE_SOURCE_PORT) != 0) && ((filterRulesStable & FE_PROTOCOL_TYPE) != 0)) {
                    if (sourcePortStable != header.SourcePort) {
                        continueToProcess = false;
                        DEBUG("network filtered at source port part");
                    }
                }
                
                if (continueToProcess && ((filterRulesStable & FE_DESTINATION_PORT) != 0) && ((filterRulesStable & FE_PROTOCOL_TYPE) != 0)) {
                    if (destinationPortStable != header.DestinationPort) {
                        continueToProcess = false;
                        DEBUG("network filtered at destination port part");
                    }
                }
                
                if (continueToProcess) {
                    DEBUG("the frame is now verified and to be stored in the memory");
                    state = STORE_DATA_HEADERS0;
                } else {
                    DEBUG("the frame is not valid and being discarded");
                    state = DISCARD_DATA;
                }
                last = (readSecondEnable) ? currentData1.last : currentData0.last;
            }
            break;

        case STORE_DATA_HEADERS0:
            if (!dataMoverFifo.full()) {
                dataMoverFifo.write(currentData0);
                if (readSecondEnable) {
                    state = STORE_DATA_HEADERS1;
                } else {
                    if (last) {
                        state = STORE_COMPLETE;
                    } else {
                        state = STORE_DATA;
                    }
                }
            }
            break;
            
        case STORE_DATA_HEADERS1:
            if (!dataMoverFifo.full()) {
                dataMoverFifo.write(currentData1);
                if (last) {
                    state = STORE_COMPLETE;
                } else {
                    state = STORE_DATA;
                }
            }
            break;

        case STORE_DATA:
            if (!dataMoverFifo.full() && !captureFifo.empty()) {
                captureFifo.read(currentData0);
                dataMoverFifo.write(currentData0);
                if (currentData0.last) {
                    state = STORE_COMPLETE;
                } 
            }
            break;
        
        case STORE_COMPLETE:
            if (!dataMoverInformation.full()) {
                DEBUG("completing the network filtering");
                informationOut.data = 1;
                dataMoverInformation.write(informationOut);
                ++countFilteredPacket;
                countFilteredPacketBytes += (informationIn.data & 0xffff);
                state = WAIT_CONTROL;
            }
            break;

        case DISCARD_DATA:
            if (last) {
                state = WAIT_CONTROL;
            } else {
                if (!captureFifo.empty()) {
                    captureFifo.read(currentData0);
                    last = currentData0.last;
                }
            }
            break;
    }

    regFilteredPacket = countFilteredPacket;
    regFilteredPacketBytes = countFilteredPacketBytes;
    
}



/// Reads from in_stream and in_counts, Write to out_memory
void trafficMover::dataMover(ap_uint<32> &captureEnable,
                             ap_uint<32> &regStoredPacket,
                             ap_uint<32> &regStoredBytes,
                             ap_uint<32> &memoryMaxPointerLower,
                             ap_uint<32> &memoryMaxPointerUpper,
                             ap_uint<32> &currentMemoryIndex,
                             hls::stream<axisData_t> &dataMoverFifo,
                             hls::stream<axisMeta_t> &dataMoverInformation,
                             ap_uint<512> *memoryOut)
{  
#pragma HLS pipeline II = 1 style = flp

    enum stateType {
        IDLE,
        OPERATING,
        DISCARDING
    };
    static stateType state = IDLE;

    axisData_t currentData;
    static ap_uint<32> index = 0;
    static ap_uint<32> numberOfBytes = 0;
    static ap_uint<64> memoryMaxPointerStable;
    
    static ap_uint<32> countStoredPacket = 0;
    static ap_uint<32> countStoredBytes = 0;

    switch (state) 
    {
        case IDLE: 
            numberOfBytes = 0;
            if (captureEnable == 0) {
                index = 0;
            }
            if (!dataMoverInformation.empty()) {
                dataMoverInformation.read();
                memoryMaxPointerStable.range(63, 32) = memoryMaxPointerUpper;
                memoryMaxPointerStable.range(31, 0) = memoryMaxPointerLower;
                if ((memoryMaxPointerStable - (ap_uint<64>)memoryOut) >= 16384) {
                    DEBUG("start transferring the data into the memory");
                    state = OPERATING;
                } else {
                    DEBUG("cannot store the data due to the buffer size");
                    state = DISCARDING;
                }
            }
            break;

        case OPERATING:
            if (!dataMoverFifo.empty()) {
#pragma HLS PIPELINE
                dataMoverFifo.read(currentData);
                memoryOut[index++] = currentData.data;
                numberOfBytes++;
                if (currentData.last) {
                    DEBUG("finish data moving");
                    state = IDLE;
                    countStoredPacket += 1;
                    countStoredBytes += (numberOfBytes << 6);
                }
            }
            break;
        
        case DISCARDING:
            if (!dataMoverFifo.empty()) {
                dataMoverFifo.read(currentData);
                if (currentData.last) {
                    state = IDLE;
                }
            }
            break;
    }

    currentMemoryIndex = index;
    regStoredPacket = countStoredPacket;
    regStoredBytes = countStoredBytes;
}