
#include <fstream>
#include <iostream>

#include "traffichandler_kernel.hpp"

trafficHandlerReg trafficreg;
networkFilterSettingsReg networkreg;
memoryRegister memoryreg;
hls::stream<axiStream_t> inputData ("inputData");
hls::stream<axiStream_t> outputData ("outputData");
hls::stream<axiStream_t> outputFeed ("outputFeed");
hls::stream<axiStream_t> testData ("testData");

// test pcap packet#1 - 251 bytes
// Ethernet II, Src: (08:00:27:38:db:ed), Dst: (08:00:27:97:3f:45)
// Internet Protocol Version 4, Src: 1.1.1.1, Dst: 1.1.1.2
// Transmission Control Protocol, Src Port: 53, Dst Port: 1042, Seq: 1, Ack: 31, Len: 197
uint8_t test_packet0[] = {
    0x08, 0x00, 0x27, 0x97, 0x3f, 0x45, 0x08, 0x00, 0x27, 0x38, 0xdb, 0xed, 0x08, 0x00, 0x45, 0x00,
    0x00, 0xed, 0x0d, 0x55, 0x00, 0x00, 0x80, 0x06, 0x28, 0xb2, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x02, 0x00, 0x35, 0x04, 0x12, 0x5f, 0xf5, 0xa8, 0xbd, 0xd1, 0xf8, 0xc1, 0x35, 0x50, 0x18,
    0xff, 0xe1, 0x9d, 0xd0, 0x00, 0x00, 0x00, 0xc3, 0x00, 0x00, 0x80, 0x80, 0x00, 0x01, 0x00, 0x04,
    0x00, 0x00, 0x00, 0x00, 0x04, 0x65, 0x74, 0x61, 0x73, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x00, 0xfc,
    0x00, 0x01, 0xc0, 0x0c, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x0e, 0x10, 0x00, 0x2f, 0x0d, 0x74,
    0x72, 0x61, 0x69, 0x6e, 0x69, 0x6e, 0x67, 0x32, 0x30, 0x30, 0x33, 0x70, 0x00, 0x0a, 0x68, 0x6f,
    0x73, 0x74, 0x6d, 0x61, 0x73, 0x74, 0x65, 0x72, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x3c, 0x00, 0x00, 0x02, 0x58, 0x00, 0x01, 0x51, 0x80, 0x00, 0x00, 0x0e, 0x10, 0xc0, 0x0c, 0x00,
    0x02, 0x00, 0x01, 0x00, 0x00, 0x0e, 0x10, 0x00, 0x0f, 0x0d, 0x74, 0x72, 0x61, 0x69, 0x6e, 0x69,
    0x6e, 0x67, 0x32, 0x30, 0x30, 0x33, 0x70, 0x00, 0x07, 0x77, 0x65, 0x6c, 0x63, 0x6f, 0x6d, 0x65,
    0xc0, 0x0c, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x0e, 0x10, 0x00, 0x04, 0x01, 0x01, 0x01, 0x01,
    0xc0, 0x0c, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x0e, 0x10, 0x00, 0x2f, 0x0d, 0x74, 0x72, 0x61,
    0x69, 0x6e, 0x69, 0x6e, 0x67, 0x32, 0x30, 0x30, 0x33, 0x70, 0x00, 0x0a, 0x68, 0x6f, 0x73, 0x74,
    0x6d, 0x61, 0x73, 0x74, 0x65, 0x72, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x3c, 0x00,
    0x00, 0x02, 0x58, 0x00, 0x01, 0x51, 0x80, 0x00, 0x00, 0x0e, 0x10
};


// test pcap packet#2 - 84 bytes
// Ethernet II, Src: (08:00:27:97:3f:45), Dst: (08:00:27:38:db:ed)
// Internet Protocol Version 4, Src: 1.1.1.2, Dst: 1.1.1.1
// Transmission Control Protocol, Src Port: 1042, Dst Port: 53, Seq: 1, Ack: 1, Len: 30
uint8_t test_packet1[] = {
    0x08, 0x00, 0x27, 0x38, 0xdb, 0xed, 0x08, 0x00, 0x27, 0x97, 0x3f, 0x45, 0x08, 0x00, 0x45, 0x00,
    0x00, 0x46, 0x00, 0xec, 0x40, 0x00, 0x80, 0x06, 0xf5, 0xc1, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01,
    0x01, 0x01, 0x04, 0x12, 0x00, 0x35, 0xd1, 0xf8, 0xc1, 0x17, 0x5f, 0xf5, 0xa8, 0xbd, 0x50, 0x18,
    0xfb, 0x90, 0x05, 0x68, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x04, 0x65, 0x74, 0x61, 0x73, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x00, 0xfc,
    0x00, 0x01, 0x4d, 0x53
};

// test pcap packet#3 - 466 bytes
// Ethernet II, Src: (00:24:dc:6d:d7:0b), Dst: (00:07:3b:d1:18:58)
// Internet Protocol Version 4, Src: 172.19.1.44, Dst: 172.23.7.10
// User Datagram Protocol, Src Port: 1719, Dst Port: 49301
uint8_t test_packet2[] = {
    0x00, 0x07, 0x3b, 0xd1, 0x18, 0x58, 0x00, 0x24, 0xdc, 0x6d, 0xd7, 0x0b, 0x08, 0x00, 0x45, 0x88,
    0x01, 0xc4, 0x00, 0x00, 0x40, 0x00, 0x3b, 0x11, 0xdd, 0x40, 0xac, 0x13, 0x01, 0x2c, 0xac, 0x17,
    0x07, 0x0a, 0x06, 0xb7, 0xc0, 0x95, 0x01, 0xb0, 0xf8, 0x36, 0x13, 0x00, 0x00, 0x02, 0x06, 0x00,
    0x08, 0x91, 0x4a, 0x00, 0x05, 0x00, 0x0a, 0x60, 0x86, 0x48, 0x01, 0x86, 0xf8, 0x72, 0x04, 0x02,
    0x01, 0x81, 0x4b, 0x2c, 0x40, 0x4d, 0x61, 0x69, 0x6e, 0x25, 0xb3, 0x7c, 0x00, 0x01, 0x40, 0x27,
    0x03, 0xf4, 0x60, 0x00, 0x00, 0x2e, 0x00, 0x08, 0x00, 0xff, 0xff, 0x03, 0x80, 0x09, 0x78, 0x00,
    0x00, 0x00, 0x00, 0x13, 0x8d, 0x00, 0x04, 0x04, 0x78, 0x00, 0x0e, 0x80, 0x60, 0x60, 0x00, 0x00,
    0x22, 0x28, 0x00, 0x01, 0x00, 0x00, 0x2b, 0x01, 0x00, 0x0a, 0x62, 0x00, 0x13, 0x04, 0x00, 0x04,
    0x00, 0x4b, 0x04, 0xb0, 0x01, 0x00, 0x01, 0x00, 0x80, 0xe8, 0x0a, 0xc0, 0x70, 0x00, 0x06, 0x58,
    0x31, 0x37, 0x38, 0x35, 0x32, 0x37, 0x30, 0x37, 0x39, 0x35, 0x36, 0x03, 0x80, 0x0e, 0x0d, 0x31,
    0x37, 0x38, 0x35, 0x2d, 0x32, 0x37, 0x30, 0x2d, 0x37, 0x39, 0x35, 0x36, 0x01, 0xac, 0xc0, 0x80,
    0x00, 0x06, 0x58, 0x31, 0x37, 0x38, 0x35, 0x32, 0x37, 0x30, 0x37, 0x39, 0x35, 0x36, 0x03, 0x80,
    0x0e, 0x0d, 0x31, 0x37, 0x38, 0x35, 0x2d, 0x32, 0x37, 0x30, 0x2d, 0x37, 0x39, 0x35, 0x36, 0x01,
    0xac, 0xc0, 0x90, 0x00, 0x06, 0x58, 0x31, 0x37, 0x38, 0x35, 0x32, 0x37, 0x30, 0x37, 0x39, 0x35,
    0x36, 0x03, 0x80, 0x0e, 0x0d, 0x31, 0x37, 0x38, 0x35, 0x2d, 0x32, 0x37, 0x30, 0x2d, 0x37, 0x39,
    0x35, 0x36, 0x01, 0xac, 0x80, 0xa0, 0x00, 0x5c, 0x03, 0x80, 0x08, 0x07, 0x41, 0x75, 0x74, 0x6f,
    0x20, 0x49, 0x6e, 0x01, 0xa4, 0x80, 0xb0, 0x00, 0x34, 0x03, 0x80, 0x08, 0x07, 0x41, 0x75, 0x78,
    0x57, 0x6f, 0x72, 0x6b, 0x01, 0xa4, 0x80, 0xc0, 0x00, 0x5b, 0x03, 0x80, 0x0a, 0x09, 0x41, 0x66,
    0x74, 0x65, 0x72, 0x43, 0x61, 0x6c, 0x6c, 0x01, 0xa4, 0xc0, 0xd0, 0x00, 0x41, 0x20, 0x2a, 0x34,
    0x35, 0x34, 0x03, 0x80, 0x0c, 0x0b, 0x41, 0x67, 0x65, 0x6e, 0x74, 0x20, 0x4c, 0x6f, 0x67, 0x69,
    0x6e, 0x01, 0xa0, 0xc0, 0xe0, 0x00, 0x41, 0x20, 0x2a, 0x34, 0x35, 0x35, 0x03, 0x80, 0x0d, 0x0c,
    0x41, 0x67, 0x65, 0x6e, 0x74, 0x20, 0x4c, 0x6f, 0x67, 0x6f, 0x75, 0x74, 0x01, 0xa0, 0x0c, 0x20,
    0x00, 0x0f, 0x4c, 0x30, 0x01, 0x46, 0x58, 0x31, 0x37, 0x38, 0x35, 0x32, 0x39, 0x35, 0x32, 0x30,
    0x32, 0x32, 0x01, 0x50, 0x03, 0x00, 0x00, 0xef, 0x11, 0x80, 0x52, 0x30, 0x31, 0x35, 0x78, 0x2e,
    0x30, 0x32, 0x2e, 0x31, 0x2e, 0x30, 0x31, 0x36, 0x2e, 0x34, 0x03, 0x00, 0xea, 0x60, 0x01, 0x00,
    0xac, 0x13, 0x01, 0x2c, 0x06, 0xb8, 0x02, 0x7f, 0x00, 0x07, 0x11, 0x21, 0x0e, 0x00, 0x2d, 0x04,
    0x40, 0xac, 0x13, 0x01, 0x2c, 0x06, 0xb7, 0x00, 0x00, 0x00, 0x80, 0x40, 0xac, 0x13, 0x01, 0x12,
    0x06, 0xb7, 0x00, 0x00, 0x00, 0x80, 0x40, 0xac, 0x13, 0x01, 0x23, 0x06, 0xb7, 0x00, 0x00, 0x00,
    0x80, 0x40, 0xac, 0x13, 0x01, 0x11, 0x06, 0xb7, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x01, 0x78,
    0x01, 0x80
};

// test pcap packet#4
// Ethernet II, Src: (08:00:27:97:3f:45), Dst: (08:00:27:38:db:ed)
// Internet Protocol Version 4, Src: 1.1.1.2, Dst: 1.1.1.1 with 40-byte option 
// Transmission Control Protocol, Src Port: 1042, Dst Port: 53, Seq: 1, Ack: 1, Len: 30
uint8_t test_packet3[] = {
    0x08, 0x00, 0x27, 0x38, 0xdb, 0xed, 0x08, 0x00, 0x27, 0x97, 0x3f, 0x45, 0x08, 0x00, 
    0x4F, 0x00, 0x00, 0x46, 0x00, 0xec, 0x40, 0x00, 0x80, 0x06, 0xf5, 0xc1, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x12, 0x00, 0x35, 0xd1, 0xf8, 0xc1, 0x17, 0x5f, 0xf5, 0xa8, 0xbd, 0x50, 0x18, 
    0xfb, 0x90, 0x05, 0x68, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x65, 0x74, 0x61, 0x73, 0x03, 0x63, 0x6f, 
    0x6d, 0x00, 0x00, 0xfc, 0x00, 0x01, 0x4d, 0x53
};


void prepareInputData(uint8_t * mem, uint32_t mem_len, uint32_t last, uint32_t user)
{
    axiStream_t tmp;

    while (mem_len != 0) {
        memset(&tmp.data, 0, sizeof(tmp.data));
        memset(&tmp.keep, 0, sizeof(tmp.keep));
        for (uint32_t i = 0; i < 64; i++) {
            if (mem_len != 0) {
                tmp.data.range(8*i+7, 8*i) = *mem++;
                tmp.keep.range(i, i) = 1;
                --mem_len;
            }
        }
        tmp.last = (mem_len == 0) ? last : 0;
        tmp.user = (mem_len == 0) ? user : 0;
        inputData.write(tmp);
        testData.write(tmp);
    }
}

void displayStatus()
{
    std::cout << "number of rx packets = " << trafficreg.regRxPacket << std::endl;
    std::cout << "number of rx bytes = " << trafficreg.regRxBytes << std::endl;
    std::cout << "number of rx discarded packets = " << trafficreg.regRxDiscard << std::endl;
    std::cout << "number of rx discarded bytes = " << trafficreg.regRxDiscardBytes << std::endl;
    std::cout << "number of rx captured packets = " << trafficreg.regRxCapture << std::endl;
    std::cout << "number of rx captured bytes = " << trafficreg.regRxCaptureBytes << std::endl;
    std::cout << "number of tx packets = " << trafficreg.regTxPacket << std::endl;
    std::cout << "number of tx bytes = " << trafficreg.regTxBytes << std::endl;
    std::cout << "number of tx dropped packets = " << trafficreg.regTxDropPacket << std::endl;
    std::cout << "number of tx dropped bytes = " << trafficreg.regTxDropBytes << std::endl;
    std::cout << "number of filtered packets = " << trafficreg.regFilteredPacket << std::endl;
    std::cout << "number of filtered bytes = " << trafficreg.regFilteredPacketBytes << std::endl;
    std::cout << "number of stored packets = " << trafficreg.regStoredPacket << std::endl;
    std::cout << "number of stored bytes = " << trafficreg.regStoredBytes << std::endl;
}

int comparetrafficregisterStatus(uint32_t expRxPacket,
                                 uint32_t expRxBytes,
                                 uint32_t expRxDiscard,
                                 uint32_t expRxDiscardBytes,
                                 uint32_t expRxCapture,
                                 uint32_t expRxCaptureBytes,
                                 uint32_t expTxPacket,
                                 uint32_t expTxBytes,
                                 uint32_t expTxDropPacket,
                                 uint32_t expTxDropBytes,
                                 uint32_t expFilteredPacket,
                                 uint32_t expFilteredPacketBytes,
                                 uint32_t expStoredPacket,
                                 uint32_t expStoredBytes)
{
    int ret_val = 0;

    if (expRxPacket != trafficreg.regRxPacket) {
        ret_val = -3;
    }

    if (expRxBytes != trafficreg.regRxBytes) {
        ret_val = -3;
    }

    if (expRxDiscard != trafficreg.regRxDiscard) {
        ret_val = -3;
    }

    if (expRxDiscardBytes != trafficreg.regRxDiscardBytes) {
        ret_val = -3;
    }
    
    if (expRxCapture != trafficreg.regRxCapture) {
        ret_val = -3;
    }

    if (expRxCaptureBytes != trafficreg.regRxCaptureBytes) {
        ret_val = -3;
    }
    
    if (expTxPacket != trafficreg.regTxPacket) {
        ret_val = -3;
    }
    
    if (expTxBytes != trafficreg.regTxBytes) {
        ret_val = -3;
    }
    
    if (expTxDropPacket != trafficreg.regTxDropPacket) {
        ret_val = -3;
    }
    
    if (expTxDropBytes != trafficreg.regTxDropBytes) {
        ret_val = -3;
    }

    if (expFilteredPacket != trafficreg.regFilteredPacket) {
        ret_val = -3;
    }
    
    if (expFilteredPacketBytes != trafficreg.regFilteredPacketBytes) {
        ret_val = -3;
    }
    
    if (expStoredPacket != trafficreg.regStoredPacket) {
        ret_val = -3;
    }
    
    if (expStoredBytes != trafficreg.regStoredBytes) {
        ret_val = -3;
    }
    return ret_val;
}

int compareDataStream(uint32_t length)
{
    int error = 0;
    axiStream_t tmp_0, tmp_1;

    for (uint32_t i = 0; i < (length + 63) / 64; i++) { 
        if (error == 0) {
            tmp_0 = outputFeed.read();
            tmp_1 = testData.read();
        }

        if (error == 0) {
            if (tmp_0.data != tmp_1.data) {
                std::cout << "Incorrect data @" << i << std::endl;
                std::cout << std::hex << "actual value = " << tmp_0.data << ", expect value =" << tmp_1.data << std::endl;
                error = -1;
            }
        }
        
        if (error == 0) {
            if (tmp_0.keep != tmp_1.keep) {
                std::cout << "Incorrect keep @" << i << std::endl;
                std::cout << std::hex << "actual value = " << tmp_0.keep << ", expect value =" << tmp_1.keep << std::endl;
                error = -1;
            }
        }

        if (error == 0) {
            if (tmp_0.last != tmp_1.last) {
                std::cout << "Incorrect last @" << i << std::endl;
                std::cout << std::hex << "actual value = " << tmp_0.last << ", expect value =" << tmp_1.last << std::endl;
                error = -1;
            }
        }
    }
    return error;
}

int compareMemories(uint8_t * mem0, uint8_t * mem1, uint32_t len) 
{
    int error = 0;
    
    for (uint32_t i = 0; i < len; i++) {
        if (*mem0++ != *mem1++) {
            error = -6;
        }
    }

    return error;
}

int main()
{
    int error = 0;
    bool waitOutputStream;
    axiStream_t tmp;
    uint32_t length;
    uint32_t errorSetting;

    uint32_t expRxPacket = 0;
    uint32_t expRxBytes = 0;
    uint32_t expRxDiscard = 0;
    uint32_t expRxDiscardBytes = 0;
    uint32_t expRxCapture = 0;
    uint32_t expRxCaptureBytes = 0;
    uint32_t expTxPacket = 0;
    uint32_t expTxBytes = 0;
    uint32_t expTxDropPacket = 0;
    uint32_t expTxDropBytes = 0;
    uint32_t expFilteredPacket = 0;
    uint32_t expFilteredPacketBytes = 0;
    uint32_t expStoredPacket = 0;
    uint32_t expStoredBytes = 0;
    uint64_t expMemoryIndex = 0;

    uint8_t * exp_ptr;
    uint8_t * test_ptr;
    uint8_t * ptr;
    ptr = (uint8_t*)malloc(16384 * sizeof(uint8_t));
    if (ptr == nullptr) {
        std::cout << "Failed to allocate memory for the test!" << std::endl;
        return -10;
    }

    ap_uint<512> * memory_ptr;
    memory_ptr = (ap_uint<512>*)malloc(131072 * sizeof(uint8_t));
    ap_uint<512> * ini_memory_ptr = memory_ptr;
    if (memory_ptr == nullptr) {
        std::cout << "Failed to allocate memory for the test!" << std::endl;
        return -10;
    }

    memset(&trafficreg, 0, sizeof(trafficreg));
    memset(&networkreg, 0, sizeof(networkreg));
    memset(&memoryreg, 0, sizeof(memoryreg));

    std::cout << "TrafficHandler Test" << std::endl;
    std::cout << "-------------------" << std::endl;

    // test number#0 - x
    // normal case 

    for (uint32_t itt = 0; itt < 10; itt++) {
        if (error != 0) {
            break;
        }
        std::cout << std::endl << "++ Test case#" << itt << " ++" << std::endl;
        // preparing data input
        switch (itt) {
            case 0: length = 640; break;
            case 1: length = 1514; break;
            case 2: length = 64; break;
            case 3: length = 511; break;
            case 4: length = 9014; break;
            case 5: length = 4800; break;
            case 6: length = 1037; break;
            case 7: length = 1; break;
            case 8: length = 100; break;
            default: length = 131; break;
        }
        for (uint32_t i = 0; i < length; i++) {
            *(ptr + i) = rand() & 0xFF;
        }
        prepareInputData(ptr, length, 1, 0);
        
        std::cout << "Invoking kernel..." << std::endl;
        waitOutputStream = false; 
        while (1) {
            trafficHandlerTop(trafficreg,
                              networkreg,
                              memoryreg,
                              inputData,
                              outputData,
                              memory_ptr);
            // checking data continuity 
            tmp.last = 0;
            if (waitOutputStream || !outputData.empty()) {
                if (outputData.empty()) {
                    std::cout << "discontinuous stream" << std::endl;
                    error = -2;
                }
                waitOutputStream = true;

                if (error == 0) {
                    tmp = outputData.read();
                    outputFeed.write(tmp);
                }
            }

            // terminate
            if ((error != 0) || (tmp.last == 1)) {
                break;
            }
        }

        std::cout << "comparing the results..." << std::endl;
        if (error == 0) {
            displayStatus();
            expRxPacket = expRxPacket + 1;
            expRxBytes = expRxBytes + length;
            expRxDiscard = expRxDiscard;
            expRxDiscardBytes = expRxDiscardBytes;
            expTxPacket = expTxPacket + 1;
            expTxBytes = expTxBytes + length;
            expTxDropPacket = expTxDropPacket;
            expTxDropBytes = expTxDropBytes;
            error = comparetrafficregisterStatus(
                expRxPacket, 
                expRxBytes, 
                expRxDiscard, 
                expRxDiscardBytes, 
                expRxCapture,
                expRxCaptureBytes,
                expTxPacket, 
                expTxBytes, 
                expTxDropPacket, 
                expTxDropBytes,
                expFilteredPacket,
                expFilteredPacketBytes,
                expStoredPacket,
                expStoredBytes
            );
        }
        if (error == 0) {
            error = compareDataStream(length);
        }
    }

    // test number#20 - x
    // receiving invalid packets 
    for (uint32_t itt = 20; itt < 23; itt++) {
        if (error != 0) {
            break;
        }
        std::cout << std::endl << "++ Test case#" << itt << " ++" << std::endl;
        // preparing data input
        switch (itt) {
            case 20: 
                length = 1024;
                errorSetting = 1;
                break;
            
            case 21: 
                length = 9015;
                errorSetting = 0;
                break;
                
            case 22: 
                length = 12000;
                errorSetting = 1;
                break;

            default:
                std::cout << "incorrect test bench sequence" << std::endl;
                error = -100;
                break;
        }
        for (uint32_t i = 0; i < length; i++) {
            *(ptr + i) = rand() & 0xFF;
        }
        prepareInputData(ptr, length, 1, errorSetting);

        std::cout << "Invoking kernel..." << std::endl;
        for (uint32_t i = 0; i < 500; i++) {
            trafficHandlerTop(trafficreg,
                              networkreg,
                              memoryreg,
                              inputData,
                              outputData,
                              memory_ptr);
        }

        std::cout << "comparing the results..." << std::endl;
        if (error == 0) {
            displayStatus();
            expRxPacket = expRxPacket;
            expRxBytes = expRxBytes;
            expRxDiscard = expRxDiscard + 1;
            expRxDiscardBytes = expRxDiscardBytes + length;
            expTxPacket = expTxPacket;
            expTxBytes = expTxBytes;
            expTxDropPacket = expTxDropPacket + 1;
            expTxDropBytes = expTxDropBytes + length;
            error = comparetrafficregisterStatus(
                expRxPacket, 
                expRxBytes, 
                expRxDiscard, 
                expRxDiscardBytes, 
                expRxCapture,
                expRxCaptureBytes,
                expTxPacket, 
                expTxBytes, 
                expTxDropPacket, 
                expTxDropBytes,
                expFilteredPacket,
                expFilteredPacketBytes,
                expStoredPacket,
                expStoredBytes
            );
        }
        if (error == 0) {
            if (!outputData.empty()) {
                std::cout << "unexpected data stream" << std::endl;
                error = -4;
            }

            while (!testData.empty()) {
                testData.read();
            }
        }

    }

    // test number#40 - x
    // streaming tests
    for (uint32_t itt = 40; itt < 41; itt++) {
        if (error != 0) {
            break;
        }
        std::cout << std::endl << "++ Test case#" << itt << " ++" << std::endl;
        // preparing data input
        length = 1460;
        errorSetting = 0;
        for (uint32_t i = 0; i < length; i++) {
            *(ptr + i) = rand() & 0xFF;
        }

        for (uint32_t num = 0; num < 4; num++) {
            switch (num) {
                case 0: prepareInputData(ptr+0, 192, 0, 0); break;
                case 1: prepareInputData(ptr+192, 378, 0, 0); break;
                case 2: prepareInputData(ptr+570, 640, 0, 0); break;
                default: prepareInputData(ptr+1210, 250, 1, 0); break;
            }
            for (uint32_t i = 0; i < 200; i++) {
                trafficHandlerTop(trafficreg,
                                  networkreg,
                                  memoryreg,
                                  inputData,
                                  outputData,
                                  memory_ptr);
            }
            
            if (error == 0) {
                if (!inputData.empty()) {
                    error = -2;
                }
                
                if (num == 3) {
                    if (outputData.empty()) {
                        error = -2;
                    }
                } else {
                    if (!outputData.empty()) {
                        error = -2;
                    }
                }
            }

            if ((error == 0) && (num != 3)) {
                error = comparetrafficregisterStatus(
                    expRxPacket, 
                    expRxBytes, 
                    expRxDiscard, 
                    expRxDiscardBytes, 
                    expRxCapture,
                    expRxCaptureBytes,
                    expTxPacket, 
                    expTxBytes, 
                    expTxDropPacket, 
                    expTxDropBytes,
                    expFilteredPacket,
                    expFilteredPacketBytes,
                    expStoredPacket,
                    expStoredBytes
                );
            }
        }

        if (error == 0) {
            while (!outputData.empty()) {
                tmp = outputData.read();
                outputFeed.write(tmp);
            }
        }

        std::cout << "comparing the results..." << std::endl;
        if (error == 0) {
            displayStatus();
            expRxPacket = expRxPacket + 1;
            expRxBytes = expRxBytes + length;
            expRxDiscard = expRxDiscard;
            expRxDiscardBytes = expRxDiscardBytes;
            expTxPacket = expTxPacket + 1;
            expTxBytes = expTxBytes + length;
            expTxDropPacket = expTxDropPacket;
            expTxDropBytes = expTxDropBytes;
            error = comparetrafficregisterStatus(
                expRxPacket, 
                expRxBytes, 
                expRxDiscard, 
                expRxDiscardBytes, 
                expRxCapture,
                expRxCaptureBytes,
                expTxPacket, 
                expTxBytes, 
                expTxDropPacket, 
                expTxDropBytes,
                expFilteredPacket,
                expFilteredPacketBytes,
                expStoredPacket,
                expStoredBytes
            );
        }
        if (error == 0) {
            error = compareDataStream(length);
        }
    }
    
    // test number#50 - x 
    // filtering and data moving
    memoryreg.memoryMaxPointerLower = (uint32_t)((uint64_t)(ini_memory_ptr + 131072));
    memoryreg.memoryMaxPointerUpper = (uint32_t)((uint64_t)(ini_memory_ptr + 131072) >> 32);
    expMemoryIndex = 0;
    exp_ptr = (uint8_t *) memory_ptr;
    for (uint32_t itt = 50; itt < 55; itt++) {
        if (error != 0) {
            break;
        }
        std::cout << std::endl << "++ Test case#" << itt << " ++" << std::endl;
        switch (itt) {
            case 50: 
                length = sizeof(test_packet0) / sizeof(uint8_t);
                test_ptr = &test_packet0[0];
                break;
            case 51: 
                length = sizeof(test_packet1) / sizeof(uint8_t);
                test_ptr = &test_packet1[0];
                break;
            case 52: 
                length = sizeof(test_packet2) / sizeof(uint8_t);
                test_ptr = &test_packet2[0];
                break;
            case 53: 
                length = sizeof(test_packet3) / sizeof(uint8_t);
                test_ptr = &test_packet3[0];
                break;
            case 54: 
                length = sizeof(test_packet0) / sizeof(uint8_t);
                test_ptr = &test_packet0[0];
                break;
            default: 
                break;
        }
        prepareInputData(test_ptr, length, 1, 0);

        // network filter settings
        switch (itt) {
            case 50:
                networkreg.destinationMACAddressUpper = 0x0800;
                networkreg.destinationMACAddressLower = 0x27973f45;
                networkreg.sourceMACAddressUpper = 0x0800;
                networkreg.sourceMACAddressLower = 0x2738dbed;
                networkreg.protocolType = 1;
                networkreg.sourceIPAddress = 0x01010101;
                networkreg.destinationIPAddress = 0x01010102;
                networkreg.sourcePort = 53;
                networkreg.destinationPort = 1042;
                networkreg.filterRules = 0b1111111;
                networkreg.captureEnable = 1;
                break;
            case 51:
                networkreg.protocolType = 1;
                networkreg.sourceIPAddress = 0x01010102;
                networkreg.destinationIPAddress = 0x01010101;
                networkreg.sourcePort = 1042;
                networkreg.filterRules = 0b0111100;
                networkreg.captureEnable = 1;
                break;
            case 52:
                memory_ptr = ini_memory_ptr; // renew the offset
                networkreg.destinationMACAddressUpper = 0x0007;
                networkreg.destinationMACAddressLower = 0x3bd11858;
                networkreg.protocolType = 0;
                networkreg.sourceIPAddress = 0xAC13012C;
                networkreg.destinationIPAddress = 0xAC17070A;
                networkreg.destinationPort = 49301;
                networkreg.filterRules = 0b1011101;
                networkreg.captureEnable = 1;
                break;
            case 53:
                networkreg.destinationMACAddressUpper = 0x0800;
                networkreg.destinationMACAddressLower = 0x2738dbed;
                networkreg.protocolType = 1;
                networkreg.destinationIPAddress = 0x01010101;
                networkreg.destinationPort = 53;
                networkreg.filterRules = 0b1010101;
                networkreg.captureEnable = 1;
                break;
            case 54:
                networkreg.destinationMACAddressUpper = 0x0800;
                networkreg.destinationMACAddressLower = 0x2738dbed;
                networkreg.protocolType = 1;
                networkreg.destinationIPAddress = 0x01010101;
                networkreg.destinationPort = 53;
                networkreg.filterRules = 0b1010101;
                networkreg.captureEnable = 1;
                break;
            default:
                break;
        }
        
        std::cout << "Invoking kernel..." << std::endl;
        for (uint32_t i = 0; i < 100; i++) {
            trafficHandlerTop(trafficreg,
                              networkreg,
                              memoryreg,
                              inputData,
                              outputData,
                              memory_ptr);
            if (!outputData.empty()) {
                tmp = outputData.read();
                outputFeed.write(tmp);
            }
        }

        std::cout << "comparing the results..." << std::endl;
        if (error == 0) {
            displayStatus();
            expRxPacket = expRxPacket + 1;
            expRxBytes = expRxBytes + length;
            expRxDiscard = expRxDiscard;
            expRxDiscardBytes = expRxDiscardBytes;
            expRxCapture = expRxCapture + 1;
            expRxCaptureBytes = expRxCaptureBytes + length;
            expTxPacket = expTxPacket + 1;
            expTxBytes = expTxBytes + length;
            expTxDropPacket = expTxDropPacket;
            expTxDropBytes = expTxDropBytes;
            if (itt != 54) {
                expFilteredPacket = expFilteredPacket + 1;
                expFilteredPacketBytes = expFilteredPacketBytes + length;
                expStoredPacket = expStoredPacket + 1;
                expStoredBytes = expStoredBytes + ((length + 63) & ~63);
            }
            error = comparetrafficregisterStatus(
                expRxPacket, 
                expRxBytes, 
                expRxDiscard, 
                expRxDiscardBytes, 
                expRxCapture,
                expRxCaptureBytes,
                expTxPacket, 
                expTxBytes, 
                expTxDropPacket, 
                expTxDropBytes,
                expFilteredPacket,
                expFilteredPacketBytes,
                expStoredPacket,
                expStoredBytes
            );
            if ((error == 0) && (itt != 54)) {
                expMemoryIndex = expMemoryIndex + ((length + 63) / 64);
                if (expMemoryIndex != memoryreg.currentMemoryIndex) {
                    error = -3;
                }
            }
        }
        if (error == 0) {
            error = compareDataStream(length);
        }
        if ((error == 0) && (itt != 54)) {
            error = compareMemories(exp_ptr, test_ptr, length);
            exp_ptr = ((uint8_t *)memory_ptr) + (expMemoryIndex * 64);
        }
    }


    // final error checking to set return code
    std::cout << std::endl << "End of test: ";
    if (error) {
        std::cout << "FAILURE! with error=" << error <<std::endl;
    } else {
        std::cout << "SUCCESS!" << std::endl;
    }

    free(ptr);
    return error;
};
