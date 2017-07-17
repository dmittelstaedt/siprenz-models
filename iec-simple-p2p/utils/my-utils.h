#ifndef MY_UTILS_H_
#define MY_UTILS_H_

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include <string>
#include <sstream>

class MyUtils {
public:
    static std::string sayHello();
    static std::string ipAddressToString(ns3::NodeContainer& nodes, uint32_t index);
};


#endif /* MY_UTILS_H_ */
