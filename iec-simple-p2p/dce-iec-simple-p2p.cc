/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**  Brief A simple model for the usage of IEC61850.
+    This model contains two nodes, which are connected over P2P. One node is
+    the server and the other node the client.
*/

#include "ns3/core-module.h"
#include "ns3/dce-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/constant-position-mobility-model.h"
#include "ccnx/misc-tools.h"
#include "utils/my-utils.h"

#include <string>
#include <sstream>

using namespace ns3;
using namespace std;

// ===========================================================================
//
//         node 0                 node 1
//   +----------------+    +----------------+
//   |                |    |                |
//   +----------------+    +----------------+
//   |    10.1.1.1    |    |    10.1.1.2    |
//   +----------------+    +----------------+
//   | point-to-point |    | point-to-point |
//   +----------------+    +----------------+
//           |                     |
//           +---------------------+
//                5 Mbps, 2 ms
//
// 2 nodes : IEC61850 Client and Server
//
// Note : Tested with libIEC61850, simple_iec_server and simple_iec_client.
//        The libIEC61850-applications are written by David Mittelstaedt.
// ===========================================================================

NS_LOG_COMPONENT_DEFINE ("SimpleP2P");

// Function to get the IP-Address as string
// string ipAddressToString(NodeContainer& nodes, uint32_t index) {
//      Ptr<Node> node =  nodes.Get(index); // Get pointer to ith node in container
//      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); // Get Ipv4 instance of the node
//      Ipv4Address addri = ipv4->GetAddress (1, 0).GetLocal (); // Get Ipv4InterfaceAddress of xth interface.
//      ostringstream stream;
//      addri.Print(stream);
//      string ip = stream.str();
//      return ip;
// }

int main (int argc, char *argv[])
{
     // simulation parameters
     string protocol = "iec61850";
     string server = "simple-iec-server";
     string client = "simple-iec-client";
     string dataRate = "5Mbps";
     string delay = "2ms";
     bool tracing = false;

     CommandLine cmd;
     cmd.AddValue ("DataRate", "Datarate of the connection", dataRate);
     cmd.AddValue ("Delay", "Delay of the connection", delay);
     cmd.AddValue ("Tracing", "Tracing with pcap files", tracing);
     cmd.Parse (argc, argv);

     NS_LOG_INFO ("Reading Input.");

     NS_LOG_INFO(MyUtils::sayHello());

     NS_LOG_INFO ("Protocol: " + protocol);
     NS_LOG_INFO ("Server: " + server);
     NS_LOG_INFO ("Client: " + client);
     NS_LOG_INFO ("DataRate: " + dataRate);
     NS_LOG_INFO ("Delay: " + delay);
     if (tracing) {
          NS_LOG_INFO ("Tracing: true");
     } else {
          NS_LOG_INFO ("Tracing: false");
     }

     NS_LOG_INFO ("Building P2P topology.");

     NS_LOG_INFO ("Creating Nodes.");
     NodeContainer nodes;
     nodes.Create (2);

     NS_LOG_INFO ("Creating PointToPointHelper.");
     PointToPointHelper pointToPoint;
     pointToPoint.SetDeviceAttribute ("DataRate", StringValue (dataRate));
     pointToPoint.SetChannelAttribute ("Delay", StringValue (delay));

     NS_LOG_INFO ("Creating NetDeviceContainer.");
     NetDeviceContainer devices;
     devices = pointToPoint.Install (nodes);

     NS_LOG_INFO ("Installing internet stack on all nodes.");
     InternetStackHelper stack;
     stack.Install (nodes);

     NS_LOG_INFO ("Assigning IP Addresses.");
     Ipv4AddressHelper address;
     address.SetBase ("10.1.1.0", "255.255.255.252");
     Ipv4InterfaceContainer interfaces = address.Assign (devices);

     // // Turn on global static routing
     Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

     NS_LOG_INFO ("IP-Address of the server: " + MyUtils::ipAddressToString(nodes, 0));

     NS_LOG_INFO ("Installing applications.");
     DceManagerHelper dceManager;
     DceApplicationHelper dce;
     ApplicationContainer apps;
     dceManager.Install (nodes);

     dce.SetStackSize (1<<20);

     // Launch simple_iec_server on node 0
     dce.SetBinary (server);
     dce.ResetArguments ();
     dce.ResetEnvironment ();
     apps = dce.Install (nodes.Get (0));
     apps.Start (Seconds (1.0));

     // Launch simple_iec_client on node 1
     dce.SetBinary (client);
     dce.ResetArguments ();
     dce.ResetEnvironment ();
     dce.AddArgument (MyUtils::ipAddressToString(nodes, 0));
     apps = dce.Install (nodes.Get (1));
     apps.Start (Seconds (3.0));
     apps.Stop (Seconds (5.0));

     if (tracing) {
          NS_LOG_INFO ("Enabling pcap tracing.");
          pointToPoint.EnablePcapAll ("simple_p2p", false);
     }

     NS_LOG_INFO ("Running Simulation.");
     Simulator::Stop (Seconds(30.0));
     Simulator::Run ();
     Simulator::Destroy ();
     NS_LOG_INFO ("Simulation done.");

     return 0;
}
