#include "ip-helper.h"

using namespace ns3;
using namespace std;

// Function to get the IP-Address as string from a node
string IpHelper::getIp(Ptr<Node> node) {
     Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); // Get Ipv4 instance of the node
     Ipv4Address addri = ipv4->GetAddress (1, 0).GetLocal (); // Get Ipv4InterfaceAddress of xth interface.
     ostringstream stream;
     addri.Print(stream);
     return stream.str();
}

// Function to get the IP-Address as string from nodecontainer
string IpHelper::getIp(NodeContainer& nodes, uint32_t index) {
     Ptr<Node> node =  nodes.Get(index); // Get pointer to ith node in container
     Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); // Get Ipv4 instance of the node
     Ipv4Address addri = ipv4->GetAddress (1, 0).GetLocal (); // Get Ipv4InterfaceAddress of xth interface.
     ostringstream stream;
     addri.Print(stream);
     return stream.str();
}
