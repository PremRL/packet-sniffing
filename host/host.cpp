/**
 * Copyright (C) 2019-2022 Xilinx, Inc
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You may
 * not use this file except in compliance with the License. A copy of the
 * License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include <cstdint>
#include <cstring>
#include <inttypes.h>
#include <iostream>
#include <sstream>    
#include <fstream>  
#include <cstring>
#include <string>
#include <xrt/xrt_device.h>
#include <experimental/xrt_xclbin.h>
#include <xrt/xrt_bo.h>
#include <xrt/xrt_kernel.h>
#include <experimental/xrt_ip.h>

// Addresses 

/* ethernet register offset */
#define ETHERNET_KERNEL_UNIQUE_NUMBER_OFFSET                        (0x00000010)
#define ETHERNET_KERNEL_MAC_RESET_OFFSET                            (0x00000020)
#define ETHERNET_KERNEL_MAC_PORT_0_STATUS_OFFSET                    (0x00000030)
#define ETHERNET_KERNEL_MAC_PORT_1_STATUS_OFFSET                    (0x00000038)

/* trafficHandler register offset */
#define TRAFFICHANDLER_KERNEL_CONTROL_OFFSET                        (0x00000000)
#define TRAFFICHANDLER_KERNEL_GLOBAL_INTERRUPT_ENABLE_OFFSET        (0x00000004)
#define TRAFFICHANDLER_KERNEL_IP_INTERRUPT_ENABLE_OFFSET            (0x00000008)
#define TRAFFICHANDLER_KERNEL_IP_INTERRUPT_STATUS_OFFSET            (0x0000000C)

#define TRAFFICHANDLER_KERNEL_UNIQUE_NUMBER_OFFSET                  (0x00000010)
#define TRAFFICHANDLER_KERNEL_RX_PACKET_OFFSET                      (0x00000020)
#define TRAFFICHANDLER_KERNEL_RX_BYTES_OFFSET                       (0x00000030)
#define TRAFFICHANDLER_KERNEL_RX_DISCARD_OFFSET                     (0x00000040)
#define TRAFFICHANDLER_KERNEL_RX_DISCARD_BYTES_OFFSET               (0x00000050)
#define TRAFFICHANDLER_KERNEL_RX_CAPTURE_OFFSET                     (0x00000060)
#define TRAFFICHANDLER_KERNEL_RX_CAPTURE_BYTES_OFFSET               (0x00000070)
#define TRAFFICHANDLER_KERNEL_TX_PACKET_OFFSET                      (0x00000080)
#define TRAFFICHANDLER_KERNEL_TX_BYTES_OFFSET                       (0x00000090)
#define TRAFFICHANDLER_KERNEL_TX_DROP_PACKET_OFFSET                 (0x000000A0)
#define TRAFFICHANDLER_KERNEL_TX_DROP_BYTES_OFFSET                  (0x000000B0)
#define TRAFFICHANDLER_KERNEL_FILTERED_PACKET_OFFSET                (0x000000C0)
#define TRAFFICHANDLER_KERNEL_FILTERED_BYTES_OFFSET                 (0x000000D0)
#define TRAFFICHANDLER_KERNEL_STORED_PACKET_OFFSET                  (0x000000E0)
#define TRAFFICHANDLER_KERNEL_STORED_BYTES_OFFSET                   (0x000000F0)

#define TRAFFICHANDLER_KERNEL_CAPTURE_ENABLE_OFFSET                 (0x00000100)
#define TRAFFICHANDLER_KERNEL_FILTER_RULES_OFFSET                   (0x00000108)
#define TRAFFICHANDLER_KERNEL_DESTINATION_MAC_ADDRESS_LOWER_OFFSET  (0x00000110)
#define TRAFFICHANDLER_KERNEL_DESTINATION_MAC_ADDRESS_UPPER_OFFSET  (0x00000118)
#define TRAFFICHANDLER_KERNEL_SOURCE_MAC_ADDRESS_LOWER_OFFSET       (0x00000120)
#define TRAFFICHANDLER_KERNEL_SOURCE_MAC_ADDRESS_UPPER_OFFSET       (0x00000128)
#define TRAFFICHANDLER_KERNEL_PROTOCOL_OFFSET                       (0x00000130)
#define TRAFFICHANDLER_KERNEL_SOURCE_IP_ADDRESS_OFFSET              (0x00000138)
#define TRAFFICHANDLER_KERNEL_DESTINATION_IP_ADDRESS_OFFSET         (0x00000140)
#define TRAFFICHANDLER_KERNEL_SOURCE_PORT_OFFSET                    (0x00000148)
#define TRAFFICHANDLER_KERNEL_DESTINATION_PORT_OFFSET               (0x00000150)

#define TRAFFICHANDLER_KERNEL_MEMORY_MAX_POINTER_LOWER_OFFSET       (0x00000158)
#define TRAFFICHANDLER_KERNEL_MEMORY_MAX_POINTER_UPPER_OFFSET       (0x00000160)
#define TRAFFICHANDLER_KERNEL_CURRENT_MEMORY_INDEX_OFFSET           (0x00000168)
#define TRAFFICHANDLER_KERNEL_MEMORY_POINTER_LOWER_OFFSET           (0x00000178)
#define TRAFFICHANDLER_KERNEL_MEMORY_POINTER_UPPER_OFFSET           (0x0000017C)

// filterEnable 
#define CAPTURE_ENABLE              (1 << 0)
#define FE_DESTINATION_MAC_ADDRESS  (1 << 0)
#define FE_SOURCE_MAC_ADDRESS       (1 << 1)
#define FE_PROTOCOL_TYPE            (1 << 2)
#define FE_SOURCE_IP_ADDRESS        (1 << 3)
#define FE_DESTINATION_IP_ADDRESS   (1 << 4)
#define FE_SOURCE_PORT              (1 << 5)
#define FE_DESTINATION_PORT         (1 << 6)

struct optionArguments {
    char * cuname;
    char * command;
    uint64_t value_option;
    char * string_option;
} optionArguments_struct;
struct optionArguments * myOptions = &optionArguments_struct;

constexpr uint32_t TOTAL_MEMORY_SIZE   = (1U << 30);                // 1 GB
constexpr uint32_t MEMORY_SIZE         = TOTAL_MEMORY_SIZE / 2;	    // 512 MB

// list of compute unit instances 
xrt::ip ip_eth;
xrt::ip ip_traf0to1;
xrt::ip ip_traf1to0;
xrt::ip ip_instance;

/* ------------------------------------------------------------------- */
// usage and status functions 

static void usage()
{
    std::cout << "usage: \n";
    std::cout << "cu-name [command] [-option]\n";
    std::cout <<
        "  cu-name      indicate one of the following kernels: ethernet, traffic_0to1, traffic_1to0\n" 
        "  [command]    ethernet    - 'getstatus' to query the current ethernet status\n"
        "               traffic_xxx - 'getstatus' to query the current statistics\n"
        "                           - 'setdestinationmac' setting the destination MAC address as a rule for capturing\n"
        "                           - 'setsourcemac' setting the source MAC address as a rule for capturing\n"
        "                           - 'setprotocol' setting the protocol as a rule for capturing, 0 for udp and 1 for tcp\n"
        "                           - 'setsourceipaddress' setting the source IP address as a rule for capturing\n"
        "                           - 'setdestinationipaddress' setting the destination IP address as a rule for capturing\n"
        "                           - 'setsourceport' setting the source port as a rule for capturing\n"
        "                           - 'setdestinationport' setting the destination port as a rule for capturing\n"
        "                           - 'startcapture' to apply the filter rules and start capturing\n"
        "                           - 'stopcapture' to stop capturing\n"
        "  [-option]   'setdestinationmac'          - hexadecimal value as a MAC address starting with '0x'\n"
        "              'setsourcemac'               - hexadecimal value as a MAC address starting with '0x'\n"
        "              'setprotocol'                - decimal value in the range of 0 and 1\n"
        "              'setsourceipaddress'         - decimal value with a specific format of x.x.x.x\n"
        "              'setdestinationipaddress'    - decimal value with a specific format of x.x.x.x\n"
        "              'setsourceport'              - decimal value in the range of 0 and 65535\n"
        "              'setdestinationport'         - decimal value in the range of 0 and 65535\n"
        "              'stopcapture'                - file directory to write the captured data\n";
    std::cout << "\n\n";
} 

#define SHOW_TRUE_FALSE(CONDITION)      ((CONDITION) ? "true" : "false")

static void displayEthernetStatus()
{
    uint32_t value; 
    std::cout << "ethernet kernel status" << std::endl;

    value = ip_eth.read_register( ETHERNET_KERNEL_MAC_RESET_OFFSET);
    std::cout << "ethernet#0 reset: " << SHOW_TRUE_FALSE(value >> 1) << std::endl;
    
    value = ip_eth.read_register(ETHERNET_KERNEL_MAC_PORT_0_STATUS_OFFSET);
    std::cout << "ethernet#0 locol fault: " << SHOW_TRUE_FALSE(value & 1) << std::endl;
    std::cout << "ethernet#0 remote false: " << SHOW_TRUE_FALSE((value >> 1) & 1) << std::endl;

    value = ip_eth.read_register( ETHERNET_KERNEL_MAC_RESET_OFFSET);
    std::cout << "ethernet#1 reset: " << SHOW_TRUE_FALSE((value >> 1) & 1) << std::endl;

    value = ip_eth.read_register(ETHERNET_KERNEL_MAC_PORT_1_STATUS_OFFSET);
    std::cout << "ethernet#1 locol fault: " << SHOW_TRUE_FALSE(value & 1) << std::endl;
    std::cout << "ethernet#1 remote false: " << SHOW_TRUE_FALSE((value >> 1) & 1) << std::endl;
}

static void displayTrafficStatus(xrt::ip instance, char * name)
{
    std::cout << "traffic" << name << " kernel status" << std::endl;
    std::cout << "number of rx packets = " << instance.read_register(TRAFFICHANDLER_KERNEL_RX_PACKET_OFFSET) << std::endl;
    std::cout << "number of rx bytes = " << instance.read_register(TRAFFICHANDLER_KERNEL_RX_BYTES_OFFSET) << std::endl;
    std::cout << "number of rx discarded packets = " << instance.read_register(TRAFFICHANDLER_KERNEL_RX_DISCARD_OFFSET) << std::endl;
    std::cout << "number of rx discarded bytes = " << instance.read_register(TRAFFICHANDLER_KERNEL_RX_DISCARD_BYTES_OFFSET) << std::endl;
    std::cout << "number of rx captured packets = " << instance.read_register(TRAFFICHANDLER_KERNEL_RX_CAPTURE_OFFSET) << std::endl;
    std::cout << "number of rx captured bytes = " << instance.read_register(TRAFFICHANDLER_KERNEL_RX_CAPTURE_BYTES_OFFSET) << std::endl;
    std::cout << "number of tx packets = " << instance.read_register(TRAFFICHANDLER_KERNEL_TX_PACKET_OFFSET) << std::endl;
    std::cout << "number of tx bytes = " << instance.read_register(TRAFFICHANDLER_KERNEL_TX_BYTES_OFFSET) << std::endl;
    std::cout << "number of tx dropped packets = " << instance.read_register(TRAFFICHANDLER_KERNEL_TX_DROP_PACKET_OFFSET) << std::endl;
    std::cout << "number of tx dropped bytes = " << instance.read_register(TRAFFICHANDLER_KERNEL_TX_DROP_BYTES_OFFSET) << std::endl;
    std::cout << "number of filtered packets = " << instance.read_register(TRAFFICHANDLER_KERNEL_FILTERED_PACKET_OFFSET) << std::endl;
    std::cout << "number of filtered bytes = " << instance.read_register(TRAFFICHANDLER_KERNEL_FILTERED_BYTES_OFFSET) << std::endl;
    std::cout << "number of stored packets = " << instance.read_register(TRAFFICHANDLER_KERNEL_STORED_PACKET_OFFSET) << std::endl;
    std::cout << "number of stored bytes = " << instance.read_register(TRAFFICHANDLER_KERNEL_STORED_BYTES_OFFSET) << std::endl;
}

/* ------------------------------------------------------------------- */
/* Argument parsing */

static bool parseMACAddress(char * str, uint64_t * value)
{
    bool ret_val = true; 

    if (*str++ != '0') {
        ret_val = false;
    }
    
    if (*str++ != 'x') {
        ret_val = false;
    }

    if (strlen((const char *)str) != 12) {
        ret_val = false;
    }

    if (ret_val) {
        *value = strtoumax(str, NULL, 16);
    }
    return ret_val;
}

static bool parseIPAddress(char * str, uint64_t * value)
{
    bool ret_val = true; 
    uint64_t ipaddr = 0;
    uint64_t tmp;

    for (uint32_t i = 0; i < 4; i++) {
        tmp = strtoumax(str, &str, 10);
        if (tmp > 255) {
            ret_val = false;
        }
        if ((i != 3) && (*str != '.')) {
            ret_val = false;
        }

        if (!ret_val) {
            break;
        } else {
            if (i != 3) {
                ++str;
            }
            ipaddr = (ipaddr << 8) + (tmp & 0xFF);
        }
    }

    if (ret_val) {
        if (*str != '\0') {
            ret_val = false;
        }
    }
    
    if (ret_val) {
        *value = ipaddr;
    }
    return ret_val;
}


static bool parseDecimalValue(char * str, uint64_t * value)
{
    bool ret_val = true; 
    
    *value = strtoumax(str, &str, 10);
      
    return ret_val;
}


static bool parseArguments(int argc, std::vector<char *> argv) 
{
    bool ret_val = true;

    switch (argc) 
    {
        case 2:
            myOptions->cuname = argv[0];
            myOptions->command = argv[1];
            break;
        case 3:
            myOptions->cuname = argv[0];
            myOptions->command = argv[1];
            myOptions->string_option = argv[2];
            break;
        default:
            ret_val = false;
            break;
    }

    if (ret_val) {
        if (strcmp("ethernet", myOptions->cuname) == 0) {
            if (strcmp("getstatus", myOptions->command) == 0) {
                if (argc != 2) {
                    ret_val = false;
                }
            } else {
                ret_val = false;
            }
        } else if ((strcmp("traffic_0to1", myOptions->cuname) == 0) || (strcmp("traffic_1to0", myOptions->cuname) == 0)) {
            if (strcmp("getstatus", myOptions->command) == 0) {
                if (argc != 2) {
                    ret_val = false;
                }
            } else if ((strcmp("setdestinationmac", myOptions->command) == 0) || (strcmp("setsourcemac", myOptions->command) == 0)) {
                if (argc != 3) {
                    ret_val = false;
                }
                if (ret_val) {
                    ret_val = parseMACAddress(myOptions->string_option, &myOptions->value_option);
                }
            } else if (strcmp("setprotocol", myOptions->command) == 0) {
                if (argc != 3) {
                    ret_val = false;
                }
                if (ret_val) {
                    ret_val = parseDecimalValue(myOptions->string_option, &myOptions->value_option);
                }
                if (ret_val && (myOptions->value_option > 1)) {
                    ret_val = false;
                }
            } else if ((strcmp("setsourceipaddress", myOptions->command) == 0) || (strcmp("setdestinationipaddress", myOptions->command) == 0)) {
                if (argc != 3) {
                    ret_val = false;
                }
                if (ret_val) {
                    ret_val = parseIPAddress(myOptions->string_option, &myOptions->value_option);
                }
            } else if ((strcmp("setsourceport", myOptions->command) == 0) || (strcmp("setdestinationport", myOptions->command) == 0)) {
                if (argc != 3) {
                    ret_val = false;
                }
                if (ret_val) {
                    ret_val = parseDecimalValue(myOptions->string_option, &myOptions->value_option);
                }
                if (myOptions->value_option > 65535) {
                    ret_val = false;
                }
            } else if (strcmp("startcapture", myOptions->command) == 0) {
                if (argc != 2) {
                    ret_val = false;
                }
            } else if (strcmp("stopcapture", myOptions->command) == 0) {
                if (argc != 3) {
                    ret_val = false;
                }
            } else {
                ret_val = false;
            }
        } else {
            ret_val = false;
        }
    }
    return ret_val;
}

void ws(std::istream &is) 
{
    while (std::isspace(is.peek())) {
        char ch;
        is.get(ch);
        if ((int)ch == 0x0A) {
            std::cout << ">> ";
        }
    }
}

/* ------------------------------------------------------------------- */

int main(int argc, char * argv[]) 
{   
    bool bContinue = true;
    bool bCommandOK;
    uint32_t value;
    
	void * host_ptr = NULL;
	uint64_t hw_ptr = 0;
	uint64_t min_hw_ptr = 0;
	uint64_t max_hw_ptr = 0;

    // connecting to the device and finding the memory index 
    auto device = xrt::device(0);
    auto xclbin = xrt::xclbin("packetsniffing.xclbin");
    auto xclbin_uuid = device.load_xclbin(xclbin);
    auto memory = xclbin.get_mems();
    int memory_index = 0;
    while (1) 
    {
        if (memory[memory_index].get_tag() == "HOST[0]") {
            break;
        }
        memory_index++;
    }

    // connecting to each IP 
    ip_eth = xrt::ip(device, xclbin_uuid, "eth_kernel:eth_kernel");
    ip_traf0to1 = xrt::ip(device, xclbin_uuid, "trafficHandlerTop:traffic_0to1");
    ip_traf1to0 = xrt::ip(device, xclbin_uuid, "trafficHandlerTop:traffic_1to0");
    
    // allocating memory 
    auto bufferHandler = xrt::bo(device, TOTAL_MEMORY_SIZE, xrt::bo::flags::host_only, memory_index);
    hw_ptr = bufferHandler.address();
    host_ptr = bufferHandler.map();

    // verifying unique numbers 
    value = ip_eth.read_register(ETHERNET_KERNEL_UNIQUE_NUMBER_OFFSET);
    if (value != 0x30314744) {
        bContinue = false;
    }

    value = ip_traf0to1.read_register(TRAFFICHANDLER_KERNEL_UNIQUE_NUMBER_OFFSET);
    if (value != 0xAABBCCDD) {
        bContinue = false;
    }
    
    value = ip_traf1to0.read_register(TRAFFICHANDLER_KERNEL_UNIQUE_NUMBER_OFFSET);
    if (value != 0xAABBCCDD) {
        bContinue = false;
    }
    
    //
    // Main programme
    //
    std::string line;      
    int args_count; 
    char nameString[20];
    std::cout << "\n packetsniffing on the alveo system example.\n\n";
    usage();
    while (bContinue) {  
        std::vector<char *> args;
        std::cout << ">> ";
        ws(std::cin); //skips Whitespaces
        std::getline(std::cin, line);
        std::istringstream iss(line);
        std::string token;
        args_count = 0;
        while(iss >> token) {
            char *arg = new char[token.size() + 1];
            copy(token.begin(), token.end(), arg);
            arg[token.size()] = '\0';
            args.push_back(arg);
            ++args_count;
        }
        args.push_back(0);
        
        // argument handler 
        memset(myOptions, 0, sizeof(*myOptions));
        bCommandOK = true;
        if (bCommandOK) {
            if (!parseArguments(args_count, args)) {
                bCommandOK = false;
            }
        }

        /* main functionality */
        if (!bCommandOK && 
            (strcmp("usage", args[0]) == 0 || strcmp("help", args[0]) == 0)) {
            usage();
        } 
        else if (!bCommandOK) {
            std::cout << "error: command invalid" << std::endl;
        }

        if (bCommandOK) {
            if (strcmp("ethernet", myOptions->cuname) == 0) 
            {
                if (strcmp("getstatus", myOptions->command) == 0) 
                {     
                    displayEthernetStatus();
                    std::cout << std::endl;
                } 
            } 
            else 
            {   
                // selecting cuname...
                if (strcmp("traffic_0to1", myOptions->cuname) == 0) {
                    ip_instance = ip_traf0to1;
                    strcpy(nameString, (const char *)"0to1");
                } else {
                    ip_instance = ip_traf1to0;
                    strcpy(nameString, (const char *)"1to0");
                }


                if (strcmp("getstatus", myOptions->command) == 0) 
                {     
                    displayTrafficStatus(ip_instance, &nameString[0]);
                    std::cout << std::endl;
                } 
                else if (strcmp("setdestinationmac", myOptions->command) == 0) 
                {
                    std::cout << "set value = 0x" << std::hex << myOptions->value_option << std::dec << std::endl;
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_DESTINATION_MAC_ADDRESS_LOWER_OFFSET, 
                        (uint32_t)myOptions->value_option
                    );
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_DESTINATION_MAC_ADDRESS_UPPER_OFFSET, 
                        (uint32_t)(myOptions->value_option >> 32)
                    );
                    value = ip_instance.read_register(
                        TRAFFICHANDLER_KERNEL_FILTER_RULES_OFFSET
                    );
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_FILTER_RULES_OFFSET, 
                        (value | FE_DESTINATION_MAC_ADDRESS)
                    );
                }
                else if (strcmp("setsourcemac", myOptions->command) == 0) 
                {
                    std::cout << "set value = 0x" << std::hex << myOptions->value_option << std::dec << std::endl;
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_SOURCE_MAC_ADDRESS_LOWER_OFFSET, 
                        (uint32_t)myOptions->value_option
                    );
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_SOURCE_MAC_ADDRESS_UPPER_OFFSET, 
                        (uint32_t)(myOptions->value_option >> 32)
                    );
                    value = ip_instance.read_register(
                        TRAFFICHANDLER_KERNEL_FILTER_RULES_OFFSET
                    );
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_FILTER_RULES_OFFSET, 
                        (value | FE_SOURCE_MAC_ADDRESS)
                    );
                }
                else if (strcmp("setprotocol", myOptions->command) == 0) 
                {
                    std::cout << "set value = 0x" << std::hex << myOptions->value_option << std::dec << std::endl;
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_PROTOCOL_OFFSET, 
                        (uint32_t)myOptions->value_option
                    );
                    value = ip_instance.read_register(
                        TRAFFICHANDLER_KERNEL_FILTER_RULES_OFFSET
                    );
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_FILTER_RULES_OFFSET, 
                        (value | FE_PROTOCOL_TYPE)
                    );
                }
                else if (strcmp("setsourceipaddress", myOptions->command) == 0) 
                {
                    std::cout << "set value = 0x" << std::hex << myOptions->value_option << std::dec << std::endl;
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_SOURCE_IP_ADDRESS_OFFSET, 
                        (uint32_t)myOptions->value_option
                    );
                    value = ip_instance.read_register(
                        TRAFFICHANDLER_KERNEL_FILTER_RULES_OFFSET
                    );
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_FILTER_RULES_OFFSET, 
                        (value | FE_SOURCE_IP_ADDRESS)
                    );
                }
                else if (strcmp("setdestinationipaddress", myOptions->command) == 0) 
                {
                    std::cout << "set value = 0x" << std::hex << myOptions->value_option << std::dec << std::endl;
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_DESTINATION_IP_ADDRESS_OFFSET, 
                        (uint32_t)myOptions->value_option
                    );
                    value = ip_instance.read_register(
                        TRAFFICHANDLER_KERNEL_FILTER_RULES_OFFSET
                    );
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_FILTER_RULES_OFFSET, 
                        (value | FE_DESTINATION_IP_ADDRESS)
                    );
                }
                else if (strcmp("setsourceport", myOptions->command) == 0) 
                {
                    std::cout << "set value = " << myOptions->value_option << std::endl;
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_SOURCE_PORT_OFFSET, 
                        (uint32_t)myOptions->value_option
                    );
                    value = ip_instance.read_register(
                        TRAFFICHANDLER_KERNEL_FILTER_RULES_OFFSET
                    );
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_FILTER_RULES_OFFSET, 
                        (value | FE_SOURCE_PORT)
                    );
                }
                else if (strcmp("setdestinationport", myOptions->command) == 0) 
                {
                    std::cout << "set value = " << myOptions->value_option << std::endl;
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_DESTINATION_PORT_OFFSET, 
                        (uint32_t)myOptions->value_option
                    );
                    value = ip_instance.read_register(
                        TRAFFICHANDLER_KERNEL_FILTER_RULES_OFFSET
                    );
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_FILTER_RULES_OFFSET, 
                        (value | FE_DESTINATION_PORT)
                    );
                }
            }
        }

        /* packet capturing */
        if (bCommandOK && ((strcmp("startcapture", myOptions->command) == 0) || (strcmp("stopcapture", myOptions->command) == 0))) {
            if (strcmp("traffic_0to1", myOptions->cuname) == 0) {
                ip_instance = ip_traf0to1;
            } 
            else if (strcmp("traffic_1to0", myOptions->cuname) == 0) 
            {
                ip_instance = ip_traf1to0;
            }
            else 
            {
                std::cout << "invalid cuname" << std::endl << std::endl;
                bCommandOK = false;
            }

            // 0to1 uses lower 512KB and 1to0 uses upper 512KB
            if (bCommandOK) {
                if (strcmp("traffic_0to1", myOptions->cuname) == 0) 
                {
                    host_ptr = host_ptr;
                    hw_ptr = hw_ptr;
                    min_hw_ptr = hw_ptr;
                    max_hw_ptr = hw_ptr + MEMORY_SIZE;
                } 
                else if (strcmp("traffic_1to0", myOptions->cuname) == 0) 
                {
                    host_ptr = host_ptr + MEMORY_SIZE;
                    hw_ptr = hw_ptr + MEMORY_SIZE;
                    min_hw_ptr = hw_ptr;
                    max_hw_ptr = hw_ptr + MEMORY_SIZE;
                }
            }

            if (bCommandOK && strcmp("startcapture", myOptions->command) == 0) 
            {
                value = ip_instance.read_register(TRAFFICHANDLER_KERNEL_CAPTURE_ENABLE_OFFSET);
                if (value != 0) {
                    std::cout << "error: capture is already running" << std::endl;
                    bCommandOK = false;
                }

                if (bCommandOK) {
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_MEMORY_POINTER_LOWER_OFFSET, 
                        (uint32_t)hw_ptr
                    );
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_MEMORY_POINTER_UPPER_OFFSET, 
                        (uint32_t)(hw_ptr >> 32)
                    );
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_MEMORY_MAX_POINTER_LOWER_OFFSET, 
                        (uint32_t)max_hw_ptr
                    );
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_MEMORY_MAX_POINTER_UPPER_OFFSET, 
                        (uint32_t)(max_hw_ptr >> 32)
                    );
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_CAPTURE_ENABLE_OFFSET, 
                        CAPTURE_ENABLE
                    );
                    std::cout << "capturing is now started" << std::endl;
                }
                std::cout << std::endl;
            }
            else if (bCommandOK && strcmp("stopcapture", myOptions->command) == 0) 
            {
                value = ip_instance.read_register(TRAFFICHANDLER_KERNEL_CAPTURE_ENABLE_OFFSET);
                if (value != 1) {
                    std::cout << "error: capture is not running" << std::endl;
                    bCommandOK = false;
                }

                if (bCommandOK) {
                    value = ip_instance.read_register(TRAFFICHANDLER_KERNEL_CURRENT_MEMORY_INDEX_OFFSET);

                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_CAPTURE_ENABLE_OFFSET, 
                        0
                    );
                    ip_instance.write_register(
                        TRAFFICHANDLER_KERNEL_FILTER_RULES_OFFSET, 
                        0
                    );
                    
                    hw_ptr += value * 64;

                    if ((hw_ptr < min_hw_ptr) || (hw_ptr >= max_hw_ptr)) {
                        std::cout << "error: physical address is not in the valid range" << std::endl;
                        bCommandOK = false;
                    }
                }

                if (bCommandOK) {
                    std::cout << "number of captured bytes = " << hw_ptr - min_hw_ptr << std::endl;

                    // Create and open a text file
                    std::ofstream MyFile(myOptions->string_option);
                    MyFile.write(reinterpret_cast<const char*>(host_ptr), value * 64);
                    MyFile.close();
                    std::cout << "File has been created and written" << std::endl;
                }
                std::cout << std::endl;
            }
        }

        // freeing arguments
        if (bCommandOK) {
            for(size_t i = 0; i < args.size(); i++)
                delete[] args[i];
        }
    }
    std::cout << "closing: system failure" << std::endl; 

    return 0;
}