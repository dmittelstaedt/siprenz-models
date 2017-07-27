#ifndef IP_HELPER_H_
#define IP_HELPER_H_

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include <string>
#include <sstream>

class IpHelper {
public:
     static std::string getIp(ns3::Ptr<ns3::Node> node);
     static std::string getIp(ns3::NodeContainer& nodes, uint32_t index);
};

#endif /* IP_HELPER_H_ */
