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
#include "utils/ip-helper.h"
#include "utils/string-helper.h"
#include "ns3/config-store.h"

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

int main (int argc, char *argv[])
{
     // variables for the simulation parameters
     string protocol = "iec61850";
     string server = "simple-iec61850-server";
     string client = "simple-iec61850-client";
     string dataRate = "5Mbps";
     string delay = "2ms";
     string configFileIn = "";
     string configFileOut = "";
     bool pcapTracing = false;
     bool asciiTracing = false;
     double duration = 30.0;
     string filePrefix = "simpletree";

     // parsing arguments given from the command line
     CommandLine cmd;
     cmd.AddValue ("ConfigFileIn", "Input config file", configFileIn);
     cmd.AddValue ("ConfigFileOut", "Output config file", configFileOut);
     cmd.AddValue ("DataRate", "Datarate of the connection", dataRate);
     cmd.AddValue ("Delay", "Delay of the connection", delay);
     cmd.AddValue ("PcapTracing", "Tracing with pcap files", pcapTracing);
     cmd.AddValue ("AsciiTracing", "Tracing with ASCII files", asciiTracing);
     cmd.AddValue ("Duration", "Duration of the simulation in sec", duration);
     cmd.Parse (argc, argv);

     NS_LOG_INFO ("Reading Input");

     // enabling input config
     if (! configFileIn.empty()) {
          Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (configFileIn));
          Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
          Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
          ConfigStore inputConfig;
          inputConfig.ConfigureDefaults ();
     }

     // logging simulation parameters
     NS_LOG_INFO ("ConfigFileIn: " + configFileIn);
     NS_LOG_INFO ("ConfigFileOut: " + configFileOut);
     NS_LOG_INFO ("Protocol: " + protocol);
     NS_LOG_INFO ("Server: " + server);
     NS_LOG_INFO ("Client: " + client);
     NS_LOG_INFO ("DataRate: " + dataRate);
     NS_LOG_INFO ("Delay: " + delay);
     if (pcapTracing) {
          NS_LOG_INFO ("PcapTracing: true");
     } else {
          NS_LOG_INFO ("PcapTracing: false");
     }
     if (asciiTracing) {
          NS_LOG_INFO ("AsciiTracing: true");
     } else {
          NS_LOG_INFO ("AsciiTracing: false");
     }
     NS_LOG_INFO ("Duration: " + StringHelper::toString(duration) + " sec");

     NS_LOG_INFO ("Building tree topology");

     // 1 NodeContainer, first nodes are always routers and last node is always
     // the client
     NS_LOG_INFO ("Creating Nodes");
     NodeContainer nodes;
     nodes.Create (8);

     // creating point to point helper
     NS_LOG_INFO ("Creating PointToPointHelper");
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

     // creating net device container
     NS_LOG_INFO ("Creating NetDeviceContainer");
     NetDeviceContainer devices_r1_r2, devices_r1_r3, devices_r2_s1,
          devices_r2_s2, devices_r3_s3, devices_r3_s4, devices_r1_c;
     devices_r1_r2 = pointToPoint_r1_r2.Install(nodes.Get (0), nodes.Get (1));
     devices_r1_r3 = pointToPoint_r1_r3.Install(nodes.Get (0), nodes.Get (2));
     devices_r2_s1 = pointToPoint_r2_s1.Install(nodes.Get (1), nodes.Get (3));
     devices_r2_s2 = pointToPoint_r2_s2.Install(nodes.Get (1), nodes.Get (4));
     devices_r3_s3 = pointToPoint_r3_s3.Install(nodes.Get (2), nodes.Get (5));
     devices_r3_s4 = pointToPoint_r3_s4.Install(nodes.Get (2), nodes.Get (6));
     devices_r1_c = pointToPoint_r1_c.Install(nodes.Get (0), nodes.Get (nodes.GetN() -1));

     // installing the internet stack
     NS_LOG_INFO ("Installing internet stack");
     InternetStackHelper stack;
     stack.InstallAll ();

     // assigning ip addresses
     NS_LOG_INFO ("Assigning IP Addresses");
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

     // turning on global static routing
     Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

     // installing the applications on the nodes
     NS_LOG_INFO ("Installing applications");
     DceManagerHelper dceManager;
     DceApplicationHelper dce;
     ApplicationContainer apps;
     dceManager.Install (nodes);

     dce.SetStackSize (1<<20);

     // launching simple_iec_server on the nodes
     for (uint32_t i = 3; i < nodes.GetN ()-1; ++i) {
          dce.SetBinary (server);
          dce.ResetArguments ();
          dce.ResetEnvironment ();
          dce.AddArgument ("-p 10102");
          dce.AddArgument ("-w 36");
          dce.AddArgument ("-v");
          apps = dce.Install (nodes.Get (i));
          apps.Start (Seconds (1.0));
     }

     // launching simple_iec_client on the last node
     for (uint32_t i = 3; i < nodes.GetN ()-1; ++i) {
          dce.SetBinary (client);
          dce.ResetArguments ();
          dce.ResetEnvironment ();
          dce.AddArgument ("-c 4");
          dce.AddArgument ("-s 1");
          dce.AddArgument ("-p 10102");
          dce.AddArgument(IpHelper::getIp(nodes.Get(i)));
          apps = dce.Install (nodes.Get (nodes.GetN ()-1));
          apps.Start (Seconds (5.0));
     }

     // enabling pcap tracing
     if (pcapTracing) {
          NS_LOG_INFO ("Enabling pcap tracing");
          pointToPoint_r1_r2.EnablePcapAll (filePrefix, false);
     }

     // enabling ASCII tracing
     if (asciiTracing) {
          NS_LOG_INFO ("Enabling ASCII tracing");
          AsciiTraceHelper ascii;
          pointToPoint_r1_r2.EnableAsciiAll (ascii.CreateFileStream (filePrefix + ".tr"));
     }

     Simulator::Stop (Seconds(duration));

     // enabling output config
     if (! configFileOut.empty()) {
          Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (configFileOut));
          Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
          Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
          ConfigStore outputConfig;
          outputConfig.ConfigureDefaults ();
          outputConfig.ConfigureAttributes ();
     }

     // running simulation
     NS_LOG_INFO ("Running Simulation");
     Simulator::Run ();
     Simulator::Destroy ();
     NS_LOG_INFO ("Simulation done");

     return 0;
}
