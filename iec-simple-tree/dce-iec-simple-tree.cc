/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**  Brief A template for the usage of dce in combination with
+    smart grid protocols. Can only be used with the p2p
+    protocol.
*/

#include "ns3/core-module.h"
#include "ns3/dce-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/constant-position-mobility-model.h"
#include "ccnx/misc-tools.h"

#include <string>
#include <sstream>

using namespace ns3;
using namespace std;

// ===========================================================================
//
//           client
//             |
//             |
//             r1
//           /   \
//          /     \
//         r2     r3
//        /\      /\
//       /  \    /  \
//      s1  s2  s3  s4
//
// Tree topology : One IEC61850 client and four servers
//
// Note : All nodes are connected with routers. The first nodes are the
//        routers and the last node is the client. Tested with libIEC61850,
//        simple_iec_server and simple_iec_client. The libIEC61850
//        applications are written by David Mittelstaedt.
// ===========================================================================

NS_LOG_COMPONENT_DEFINE ("SimpleTree");

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

     NS_LOG_INFO ("Building tree topology.");

     // 1 NodeContainer, first nodes are always routers and last node is always
     // the client
     NS_LOG_INFO ("Creating Nodes.");
     NodeContainer nodes;
     nodes.Create (8);

     NS_LOG_INFO ("Creating PointToPointHelper.");
     PointToPointHelper pointToPoint_r1_r2, pointToPoint_r1_r3,
          pointToPoint_r2_s1, pointToPoint_r2_s2, pointToPoint_r3_s3,
          pointToPoint_r3_s4, pointToPoint_r1_c;
     pointToPoint_r1_r2.SetDeviceAttribute ("DataRate", StringValue (dataRate));
     pointToPoint_r1_r2.SetChannelAttribute ("Delay", StringValue (delay));
     pointToPoint_r1_r3.SetDeviceAttribute ("DataRate", StringValue (dataRate));
     pointToPoint_r1_r3.SetChannelAttribute ("Delay", StringValue (delay));
     pointToPoint_r2_s1.SetDeviceAttribute ("DataRate", StringValue (dataRate));
     pointToPoint_r2_s1.SetChannelAttribute ("Delay", StringValue (delay));
     pointToPoint_r2_s2.SetDeviceAttribute ("DataRate", StringValue (dataRate));
     pointToPoint_r2_s2.SetChannelAttribute ("Delay", StringValue (delay));
     pointToPoint_r3_s3.SetDeviceAttribute ("DataRate", StringValue (dataRate));
     pointToPoint_r3_s3.SetChannelAttribute ("Delay", StringValue (delay));
     pointToPoint_r3_s4.SetDeviceAttribute ("DataRate", StringValue (dataRate));
     pointToPoint_r3_s4.SetChannelAttribute ("Delay", StringValue (delay));
     pointToPoint_r1_c.SetDeviceAttribute ("DataRate", StringValue (dataRate));
     pointToPoint_r1_c.SetChannelAttribute ("Delay", StringValue (delay));

     NS_LOG_INFO ("Creating NetDeviceContainer.");
     NetDeviceContainer devices_r1_r2, devices_r1_r3, devices_r2_s1,
          devices_r2_s2, devices_r3_s3, devices_r3_s4, devices_r1_c;
     devices_r1_r2 = pointToPoint_r1_r2.Install(nodes.Get (0), nodes.Get (1));
     devices_r1_r3 = pointToPoint_r1_r3.Install(nodes.Get (0), nodes.Get (2));
     devices_r2_s1 = pointToPoint_r2_s1.Install(nodes.Get (1), nodes.Get (3));
     devices_r2_s2 = pointToPoint_r2_s2.Install(nodes.Get (1), nodes.Get (4));
     devices_r3_s3 = pointToPoint_r3_s3.Install(nodes.Get (2), nodes.Get (5));
     devices_r3_s4 = pointToPoint_r3_s4.Install(nodes.Get (2), nodes.Get (6));
     devices_r1_c = pointToPoint_r1_c.Install(nodes.Get (0), nodes.Get (nodes.GetN() -1));

     NS_LOG_INFO ("Installing internet stack on all nodes.");
     InternetStackHelper stack;
     stack.InstallAll ();

     NS_LOG_INFO ("Assigning IP Addresses.");
     Ipv4AddressHelper address;
     Ipv4InterfaceContainer interfaces;
     address.SetBase ("10.1.1.0", "255.255.255.252");
     interfaces = address.Assign (devices_r1_r2);
     address.SetBase ("10.1.2.0", "255.255.255.252");
     interfaces = address.Assign (devices_r1_r3);
     address.SetBase ("10.1.3.0", "255.255.255.252");
     interfaces = address.Assign (devices_r2_s1);
     address.SetBase ("10.1.4.0", "255.255.255.252");
     interfaces = address.Assign (devices_r2_s2);
     address.SetBase ("10.1.5.0", "255.255.255.252");
     interfaces = address.Assign (devices_r3_s3);
     address.SetBase ("10.1.6.0", "255.255.255.252");
     interfaces = address.Assign (devices_r3_s4);
     address.SetBase ("10.1.7.0", "255.255.255.252");
     interfaces = address.Assign (devices_r1_c);

     // // Turn on global static routing
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
     apps = dce.Install (nodes.Get (3));
     apps.Start (Seconds (2.0));
     apps.Stop (Seconds (30.0));

     // Installing the server
     dce.SetBinary (server);
     dce.ResetArguments ();
     dce.ResetEnvironment ();
     apps = dce.Install (nodes.Get (4));
     apps.Start (Seconds (4.0));
     apps.Stop (Seconds (30.0));

     // Installing the server
     dce.SetBinary (server);
     dce.ResetArguments ();
     dce.ResetEnvironment ();
     apps = dce.Install (nodes.Get (5));
     apps.Start (Seconds (6.0));
     apps.Stop (Seconds (30.0));

     // Installing the server
     dce.SetBinary (server);
     dce.ResetArguments ();
     dce.ResetEnvironment ();
     apps = dce.Install (nodes.Get (6));
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
          pointToPoint_r1_r2.EnablePcapAll ("simple_tree", false);
     }

     NS_LOG_INFO ("Running Simulation.");
     Simulator::Stop (Seconds(30.0));
     Simulator::Run ();
     Simulator::Destroy ();
     NS_LOG_INFO ("Simulation done.");

     return 0;
}
