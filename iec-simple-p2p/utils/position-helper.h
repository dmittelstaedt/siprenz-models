#ifndef POSITION_HELPER_H_
#define POSITION_HELPER_H_

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"

class PositionHelper {
public:
     static void setPosition(ns3::Ptr<ns3::Node> node, double x, double y);
};

#endif /* POSITION_HELPER_H_ */
