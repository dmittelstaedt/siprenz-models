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
#include "utils/ip-helper.h"
#include "utils/string-helper.h"
#include "ns3/config-store.h"

#include <string>
#include <sstream>

using namespace ns3;
using namespace std;

// ===========================================================================
//
//        n2 n3 n4
//         \ | /
//          \|/
//     n1--- n0---n5
//          /|\
//         / | \
//        n8 n7 n6
//
// Star topology : One IEC61850 client and multiple servers
//
// Note : All nodes are connected with a hub. Tested with libIEC61850,
//        simple_iec_server and simple_iec_client. The libIEC61850
//        applications are written by David Mittelstaedt.
// ===========================================================================

NS_LOG_COMPONENT_DEFINE ("GenericStar");

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
     string filePrefix = "genericstar";
     uint32_t nSpokes = 4; // Default number of nodes in the star (without router)

     CommandLine cmd;
     cmd.AddValue ("ConfigFileIn", "Input config file", configFileIn);
     cmd.AddValue ("ConfigFileOut", "Output config file", configFileOut);
     cmd.AddValue ("DataRate", "Datarate of the connection", dataRate);
     cmd.AddValue ("Delay", "Delay of the connection", delay);
     cmd.AddValue ("PcapTracing", "Tracing with pcap files", pcapTracing);
     cmd.AddValue ("AsciiTracing", "Tracing with ASCII files", asciiTracing);
     cmd.AddValue ("Duration", "Duration of the simulation in sec", duration);
     cmd.AddValue ("nSpokes", "Number of nodes to place in the star", nSpokes);
     cmd.Parse (argc, argv);

     NS_LOG_INFO ("Reading Input.");

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
     NS_LOG_INFO ("nSpokes: " + StringHelper::toString(nSpokes));

     NS_LOG_INFO ("Building generic star topology.");

     // creating point to point helper
     NS_LOG_INFO ("Creating PointToPointHelper.");
     PointToPointHelper pointToPoint;
     pointToPoint.SetDeviceAttribute ("DataRate", StringValue (dataRate));
     pointToPoint.SetChannelAttribute ("Delay", StringValue (delay));
     PointToPointStarHelper star (nSpokes, pointToPoint);

     // installing the internet stack on all nodes
     NS_LOG_INFO ("Installing internet stack.");
     InternetStackHelper internet;
     star.InstallStack (internet);

     // assigning ip addresses
     NS_LOG_INFO ("Assigning IP addresses.");
     star.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"));

     // turning on global static routing
     Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

     // installing the applications on the nodes
     NS_LOG_INFO ("Installing applications.");
     DceManagerHelper dceManager;
     DceApplicationHelper dce;
     ApplicationContainer apps;

     dce.SetStackSize (1<<20);

     // launching simple_iec_server on all server nodes
     for (uint32_t i = 0; i < star.SpokeCount ()-1; ++i) {
          dceManager.Install (star.GetSpokeNode (i));
          dce.SetBinary (server);
          dce.ResetArguments ();
          dce.ResetEnvironment ();
          dce.AddArgument ("-p 10102");
          dce.AddArgument ("-w 36");
          dce.AddArgument ("-v");
          apps = dce.Install (star.GetSpokeNode (i));
          apps.Start (Seconds (1.0));
     }

     // launching simple_iec_client on the last node
     dceManager.Install (star.GetSpokeNode(star.SpokeCount()-1));
     for (uint32_t i = 0; i < star.SpokeCount ()-1; ++i) {
          dce.SetBinary (client);
          dce.ResetArguments ();
          dce.ResetEnvironment ();
          // dce.AddArgument ("-c 4");
          dce.AddArgument ("-s 1");
          dce.AddArgument ("-p 10102");
          dce.AddArgument(IpHelper::getIp(star.GetSpokeNode (i)));
          apps = dce.Install (star.GetSpokeNode(star.SpokeCount()-1));
          apps.Start (Seconds (5.0));
     }

     // enabling pcap tracing
     if (pcapTracing) {
          NS_LOG_INFO ("Enabling pcap tracing");
          pointToPoint.EnablePcapAll (filePrefix, false);
     }

     // enabling ASCII tracing
     if (asciiTracing) {
          NS_LOG_INFO ("Enabling ASCII tracing");
          AsciiTraceHelper ascii;
          pointToPoint.EnableAsciiAll (ascii.CreateFileStream (filePrefix + ".tr"));
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
     NS_LOG_INFO ("Running Simulation.");
     Simulator::Run ();
     Simulator::Destroy ();
     NS_LOG_INFO ("Simulation done.");

     return 0;
}
