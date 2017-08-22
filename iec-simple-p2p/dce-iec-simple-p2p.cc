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
#include "utils/ip-helper.h"
#include "utils/string-helper.h"
#include "ns3/config-store.h"

#include <string>
#include <sstream>

using namespace ns3;
using namespace std;

/**
* ===========================================================================
*
*                   P2P
*           client ----- server
*
* 2 nodes : IEC61850 Client and Server
*
* Note :  Tested with libIEC61850, simple-iec61850-server and
*         simple-iec61850-client.
* ===========================================================================
*/

NS_LOG_COMPONENT_DEFINE ("SimpleP2P");

/**
* Main function.
* Details of the function.
* @param argc Number of arguments
* @param argv Content of the arguments
* @return Exit status of the application
*/
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
     string filePrefix = "simplep2p";

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

     NS_LOG_INFO ("Building P2P topology");

     // creating nodes
     NS_LOG_INFO ("Creating Nodes");
     NodeContainer nodes;
     nodes.Create (2);

     // creating point to point helper
     NS_LOG_INFO ("Creating PointToPointHelper");
     PointToPointHelper pointToPoint;
     pointToPoint.SetDeviceAttribute ("DataRate", StringValue (dataRate));
     pointToPoint.SetChannelAttribute ("Delay", StringValue (delay));

     // creating net device container
     NS_LOG_INFO ("Creating NetDeviceContainer");
     NetDeviceContainer devices;
     devices = pointToPoint.Install (nodes);

     // installing the internet stack
     NS_LOG_INFO ("Installing internet stack");
     InternetStackHelper stack;
     stack.Install (nodes);

     // assigning ip addresses
     NS_LOG_INFO ("Assigning IP Addresses");
     Ipv4AddressHelper address;
     address.SetBase ("10.1.1.0", "255.255.255.252");
     Ipv4InterfaceContainer interfaces = address.Assign (devices);

     // turning on global static routing
     Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

     // installing the applications on the nodes
     NS_LOG_INFO ("Installing applications");
     DceManagerHelper dceManager;
     DceApplicationHelper dce;
     ApplicationContainer apps;
     dceManager.Install (nodes);

     dce.SetStackSize (1<<20);

     // launching simple_iec_server on node 0
     dce.SetBinary (server);
     dce.ResetArguments ();
     dce.ResetEnvironment ();
     dce.AddArgument ("-p 10102");
     dce.AddArgument ("-w 36");
     dce.AddArgument ("-v");
     apps = dce.Install (nodes.Get (0));
     apps.Start (Seconds (1.0));

     // launching simple_iec_client on node 1
     dce.SetBinary (client);
     dce.ResetArguments ();
     dce.ResetEnvironment ();
     dce.AddArgument ("-c 4");
     dce.AddArgument ("-s 1");
     dce.AddArgument ("-p 10102");
     dce.AddArgument (IpHelper::getIp(nodes.Get(0)));
     apps = dce.Install (nodes.Get (1));
     apps.Start (Seconds (5.0));

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
     NS_LOG_INFO ("Running Simulation");
     Simulator::Run ();
     Simulator::Destroy ();
     NS_LOG_INFO ("Simulation done");

     return 0;
}
