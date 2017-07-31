/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**  Brief A template for the usage of dce in combination with
+    smart grid protocols. Can only be used with the p2p
+    protocol.
*/

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/dce-module.h"
#include "utils/ip-helper.h"
#include "utils/string-helper.h"
#include "ns3/config-store.h"
#include <string>
#include <sstream>

using namespace ns3;
using namespace std;

// ===========================================================================
//
//   UE 7.0.0.0
//
//  *    *    *
//  |    |    |       10.1.1.0
//  ue1 enb  PGW ----------------- RH
//                point-to-point   |
//                                 |
//                                 |
//                                 s1
//
//
// Note : Tested with libIEC61850.
// ===========================================================================

NS_LOG_COMPONENT_DEFINE ("MixedLTE");

int main (int argc, char *argv[])
{
     // simulation parameters
     string protocol = "iec61850";
     string server = "simple-iec-server";
     string client = "simple-iec-client";
     string dataRate = "100Mbps";
     string delay = "2ms";
     string configFileIn = "";
     string configFileOut = "";
     bool pcapTracing = false;
     bool asciiTracing = false;
     bool lteTracing = false;
     double duration = 10.0;
     string filePrefix = "mixedlte";

     // parsing arguments given from the command line
     CommandLine cmd;
     cmd.AddValue ("ConfigFileIn", "Input config file", configFileIn);
     cmd.AddValue ("ConfigFileOut", "Output config file", configFileOut);
     cmd.AddValue ("DataRate", "Datarate of the connection", dataRate);
     cmd.AddValue ("Delay", "Delay of the connection", delay);
     cmd.AddValue ("PcapTracing", "Tracing with pcap files", pcapTracing);
     cmd.AddValue ("AsciiTracing", "Tracing with ASCII files", asciiTracing);
     cmd.AddValue ("LteTracing", "Tracing with LTE files", lteTracing);
     cmd.AddValue ("Duration", "Duration of the simulation in sec", duration);
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
     if (lteTracing) {
          NS_LOG_INFO ("LteTracing: true");
     } else {
          NS_LOG_INFO ("LteTracing: false");
     }
     NS_LOG_INFO ("Duration: " + StringHelper::toString(duration) + " sec");

     NS_LOG_INFO ("Building Mixed LTE topology.");

     NS_LOG_INFO ("Creating EPC.");
     Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
     Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
     lteHelper->SetEpcHelper (epcHelper);

     // Create the client as single remote node
     NS_LOG_INFO ("Creating the client as remote node.");
     NodeContainer remoteNode;
     remoteNode.Create (1);

     // Create a server as p2p node
     NS_LOG_INFO ("Creating a server as P2P node.");
     NodeContainer p2pNode;
     p2pNode.Create (1);

     // installing the internet stack on the p2p nodes
     NS_LOG_INFO ("Installing internet stack on the p2p nodes.");
     InternetStackHelper internet;
     internet.Install (remoteNode);
     internet.Install (p2pNode);

     // creating point to point helper
     NS_LOG_INFO ("Creating PointToPointHelper.");
     PointToPointHelper pointToPoint_pgw_rn, pointToPoint_rn_pn;
     pointToPoint_pgw_rn.SetDeviceAttribute ("DataRate", StringValue (dataRate));
     pointToPoint_pgw_rn.SetChannelAttribute ("Delay", StringValue (delay));
     pointToPoint_rn_pn.SetDeviceAttribute ("DataRate", StringValue (dataRate));
     pointToPoint_rn_pn.SetChannelAttribute ("Delay", StringValue (delay));

     // creating net device container
     NS_LOG_INFO ("Creating NetDeviceContainer.");
     NetDeviceContainer devices_pgw_rn = pointToPoint_pgw_rn.Install (epcHelper->GetPgwNode (), remoteNode.Get (0));
     NetDeviceContainer devices_rn_pn = pointToPoint_rn_pn.Install (remoteNode.Get (0), p2pNode.Get (0));

     // assigning ip addresses
     NS_LOG_INFO ("Assigning IP Addresses.");
     Ipv4AddressHelper address;
     Ipv4InterfaceContainer interfaces;
     address.SetBase ("10.1.1.0", "255.255.255.0");
     interfaces.Add (address.Assign (devices_pgw_rn));
     address.SetBase ("10.1.2.0", "255.255.255.252");
     interfaces.Add (address.Assign (devices_rn_pn));

     // turning on static routing for the remote node
     Ipv4StaticRoutingHelper ipv4RoutingHelper;
     Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteNode.Get (0)->GetObject<Ipv4> ());
     remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

     // creating the LTE nodes
     NS_LOG_INFO ("Creating LTE nodes.");
     NodeContainer enbNodes, ueNodes;
     enbNodes.Create (1);
     ueNodes.Create (1);

     //TODO: how to set the position
     MobilityHelper mobility;
     mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
     mobility.Install (enbNodes);
     mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
     mobility.Install (ueNodes);

     // creating net device container
     NS_LOG_INFO ("Creating NetDeviceContainer for the LTE nodes.");
     NetDeviceContainer enbDevs, ueDevs;
     enbDevs = lteHelper->InstallEnbDevice (enbNodes);
     ueDevs = lteHelper->InstallUeDevice (ueNodes);

     // installing internet stack on the ue nodes
     NS_LOG_INFO ("Installing internet stack on the ue nodes.");
     internet.Install (ueNodes);

     // assign IP address to UEs
     NS_LOG_INFO ("Assigning IP addresses to the ue nodes.");
     Ipv4InterfaceContainer ueIpIface;
     Ptr<Node> ue = ueNodes.Get (0);
     Ptr<NetDevice> ueLteDevice = ueDevs.Get (0);
     ueIpIface.Add(epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevice)));
     // set the default gateway for the UE
     Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
     ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

     lteHelper->Attach (ueDevs, enbDevs.Get (0));

     //TODO: DedicatedEpSBearer not neccessary, there is always a default bearer
     // Ptr<EpcTft> tft = Create<EpcTft> ();
     // EpcTft::PacketFilter pf;
     // pf.localPortStart = 1234;
     // pf.localPortEnd = 1234;
     // tft->Add (pf);
     // lteHelper->ActivateDedicatedEpsBearer (ueDevs, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), tft);

     // Install applications on the nodes.
     NS_LOG_INFO ("Installing applications.");
     DceManagerHelper dceManager;
     DceApplicationHelper dce;
     ApplicationContainer apps;
     dceManager.Install (ueNodes);
     dceManager.Install (remoteNode);
     dceManager.Install (p2pNode);

     dce.SetStackSize (1<<20);

     // Launch iec server on the ue node
     dce.SetBinary (server);
     dce.ResetArguments ();
     dce.ResetEnvironment ();
     apps = dce.Install (ueNodes.Get (0));
     apps.Start (Seconds (1.0));

     // Launch iec server on p2p node
     dce.SetBinary (server);
     dce.ResetArguments ();
     dce.ResetEnvironment ();
     apps = dce.Install (p2pNode.Get (0));
     apps.Start (Seconds (3.0));

     // Launch iec client on the remote node
     dce.SetBinary (client);
     dce.ResetArguments ();
     dce.ResetEnvironment ();
     dce.AddArgument (IpHelper::getIp(ueNodes.Get (0)));
     apps = dce.Install (remoteNode.Get(0));
     apps.Start (Seconds (4.0));
     apps.Stop (Seconds (duration));

     // Launch iec client on the remote node
     dce.SetBinary (client);
     dce.ResetArguments ();
     dce.ResetEnvironment ();
     dce.AddArgument (IpHelper::getIp(p2pNode.Get (0)));
     apps = dce.Install (remoteNode.Get(0));
     apps.Start (Seconds (8.0));
     apps.Stop (Seconds (duration));

     // enabling pcap tracing
     if (pcapTracing) {
          NS_LOG_INFO ("Enabling pcap tracing.");
          pointToPoint_pgw_rn.EnablePcapAll (filePrefix, false);
     }

     // enabling ASCII tracing
     if (asciiTracing) {
          NS_LOG_INFO ("Enabling ASCII tracing.");
          AsciiTraceHelper ascii;
          pointToPoint_pgw_rn.EnableAsciiAll (ascii.CreateFileStream (filePrefix + ".tr"));
     }

     // enabling LTE tracing
     // define at the end of the simulation script
     if (lteTracing) {
          NS_LOG_INFO ("Enabling LTE tracing.");
          lteHelper->EnablePhyTraces ();
          lteHelper->EnableMacTraces ();
          lteHelper->EnableRlcTraces ();
          lteHelper->EnablePdcpTraces ();
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
