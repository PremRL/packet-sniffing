#ifndef TRAFFICHANDLER_KERNEL_H
#define TRAFFICHANDLER_KERNEL_H

#include "traffichandler.hpp"

extern "C" void trafficHandlerTop(trafficHandlerReg &trafficReg,
                                  networkFilterSettingsReg &networkReg,
                                  memoryRegister &memoryReg,
                                  hls::stream<axiStream_t> &inputData,
                                  hls::stream<axiStream_t> &outputData, 
                                  ap_uint<512> *memoryOut);

#endif // TRAFFICHANDLER_KERNEL_H
