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
#include "ns3/point-to-point-layout-module.h"

#include <string>
#include <sstream>

using namespace ns3;
using namespace std;

// ===========================================================================
//
//           client
//             |
//             |
//           router
//            /|\
//           / | \
//          s1 s2 s3
//
// Star topology : One IEC61850 client and three servers
//
// Note : All nodes are connected with a router. The first node is the
//        router and the last node is the client. Tested with libIEC61850,
//        simple_iec_server and simple_iec_client. The libIEC61850
//        applications are written by David Mittelstaedt.
// ===========================================================================

NS_LOG_COMPONENT_DEFINE ("SimpleStar");

// Function to get the IP-Address as string
string ipAddressToString(NodeContainer& nodes, uint32_t index) {
     Ptr<Node> node =  nodes.Get(index); // Get pointer to ith node in container
     Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); // Get Ipv4 instance of the node
     Ipv4Address addri = ipv4->GetAddress (1, 0).GetLocal (); // Get Ipv4InterfaceAddress of xth interface.
     ostringstream stream;
     addri.Print(stream);
     string ip = stream.str();
     return ip;
}

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

     NS_LOG_INFO ("Building star topology.");

     // 1 NodeContainer, first node is always the router and last node always
     // the client
     NS_LOG_INFO ("Creating Nodes.");
     NodeContainer nodes;
     nodes.Create (5);

     NS_LOG_INFO ("Creating PointToPointHelper.");
     PointToPointHelper pointToPoint_router_server1, pointToPoint_router_server2, pointToPoint_router_server3, pointToPoint_router_client;
     pointToPoint_router_server1.SetDeviceAttribute ("DataRate", StringValue (dataRate));
     pointToPoint_router_server1.SetChannelAttribute ("Delay", StringValue (delay));
     pointToPoint_router_server2.SetDeviceAttribute ("DataRate", StringValue (dataRate));
     pointToPoint_router_server2.SetChannelAttribute ("Delay", StringValue (delay));
     pointToPoint_router_server3.SetDeviceAttribute ("DataRate", StringValue (dataRate));
     pointToPoint_router_server3.SetChannelAttribute ("Delay", StringValue (delay));
     pointToPoint_router_client.SetDeviceAttribute ("DataRate", StringValue (dataRate));
     pointToPoint_router_client.SetChannelAttribute ("Delay", StringValue (delay));

     NS_LOG_INFO ("Creating NetDeviceContainer.");
     NetDeviceContainer devices_router_server1, devices_router_server2, devices_router_server3, devices_router_client;
     devices_router_server1 = pointToPoint_router_server1.Install(nodes.Get (0), nodes.Get (1));
     devices_router_server2 = pointToPoint_router_server2.Install(nodes.Get (0), nodes.Get (2));
     devices_router_server3 = pointToPoint_router_server3.Install(nodes.Get (0), nodes.Get (3));
     devices_router_client = pointToPoint_router_client.Install(nodes.Get (0), nodes.Get (nodes.GetN() -1));

     NS_LOG_INFO ("Installing internet stack on all nodes.");
     InternetStackHelper stack;
     stack.InstallAll ();

     NS_LOG_INFO ("Assigning IP Addresses.");
     Ipv4AddressHelper address;
     Ipv4InterfaceContainer interfaces;
     address.SetBase ("10.1.1.0", "255.255.255.252");
     interfaces.Add(address.Assign (devices_router_server1));
     address.SetBase ("10.1.2.0", "255.255.255.252");
     interfaces.Add(address.Assign (devices_router_server2));
     address.SetBase ("10.1.3.0", "255.255.255.252");
     interfaces.Add(address.Assign (devices_router_server3));
     address.SetBase ("10.1.4.0", "255.255.255.252");
     interfaces.Add(address.Assign (devices_router_client));

     // Turn on global static routing so we can actually be routed across the
     // star.
     Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

     NS_LOG_INFO ("Installing applications.");
     DceManagerHelper dceManager;
     DceApplicationHelper dce;
     ApplicationContainer apps;
     dceManager.Install (nodes);

     dce.SetStackSize (1<<20);

     // Installing the server
     dce.SetBinary (server);
     dce.ResetArguments ();
     dce.ResetEnvironment ();
     apps = dce.Install (nodes.Get (1));
     apps.Start (Seconds (2.0));
     apps.Stop (Seconds (30.0));

     // Installing the server
     dce.SetBinary (server);
     dce.ResetArguments ();
     dce.ResetEnvironment ();
     apps = dce.Install (nodes.Get (2));
     apps.Start (Seconds (4.0));
     apps.Stop (Seconds (30.0));

     // Installing the server
     dce.SetBinary (server);
     dce.ResetArguments ();
     dce.ResetEnvironment ();
     apps = dce.Install (nodes.Get (3));
     apps.Start (Seconds (8.0));
     apps.Stop (Seconds (30.0));

     // Installing the client on the last node
     dce.SetBinary (client);
     dce.ResetArguments ();
     dce.ResetEnvironment ();
     dce.AddArgument("10.1.3.2");
     apps = dce.Install (nodes.Get (nodes.GetN() -1));
     apps.Start (Seconds (10.0));
     apps.Stop (Seconds (12.0));

     if (tracing) {
          NS_LOG_INFO ("Enabling pcap tracing.");
          pointToPoint_router_server1.EnablePcapAll ("simple_star", false);
     }

     NS_LOG_INFO ("Running Simulation.");
     Simulator::Stop (Seconds(30.0));
     Simulator::Run ();
     Simulator::Destroy ();
     NS_LOG_INFO ("Simulation done.");

     return 0;
}
