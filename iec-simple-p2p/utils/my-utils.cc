#include "my-utils.h"

using namespace ns3;
using namespace std;

string MyUtils::sayHello() {
     string test = "Hello World from outside of ns-3. I was called without a parameter.";
     return test;
}

string MyUtils::ipAddressToString(NodeContainer& nodes, uint32_t index) {
     Ptr<Node> node =  nodes.Get(index); // Get pointer to ith node in container
     Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); // Get Ipv4 instance of the node
     Ipv4Address addri = ipv4->GetAddress (1, 0).GetLocal (); // Get Ipv4InterfaceAddress of xth interface.
     ostringstream stream;
     addri.Print(stream);
     string ip = stream.str();
     return ip;
}
