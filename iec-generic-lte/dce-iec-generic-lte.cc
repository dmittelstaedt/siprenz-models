/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**  Brief A generic model for the usage of IEC61850 in an lte based network.
+    This model contains two UEs, which are connected over eNB and PGW to
+    the client.
+
+    @author David Mittelstädt
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
// TODO: graphic with modell
//
// Note :  Tested with libIEC61850, simple_iec_server and simple_iec_client.
//         The libIEC61850 applications are written by David Mittelstaedt.
// ===========================================================================

NS_LOG_COMPONENT_DEFINE ("genericlte");

/**
* Main function.
* Starts the simulation.
* @param argc Number of arguments
* @param argv Content of the arguments
* @return Exit status of the application
*/
int main (int argc, char *argv[])
{
     // simulation parameters
     string protocol = "iec61850";
     string server = "simple-iec61850-server";
     string client = "simple-iec61850-client";
     string dataRate = "5Mbps";
     string delay = "2ms";
     string configFileIn = "";
     string configFileOut = "";
     bool pcapTracing = false;
     bool asciiTracing = false;
     bool lteTracing = false;
     double duration = 15.0;
     uint32_t nUes = 2;
     string filePrefix = "genericlte";

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
     cmd.AddValue ("nUEs", "Number of UEs", nUes);
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
     NS_LOG_INFO ("nUEs: " + StringHelper::toString(nUes));

     NS_LOG_INFO ("Building Simple LTE topology.");

     NS_LOG_INFO ("Creating EPC.");
     Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
     Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
     lteHelper->SetEpcHelper (epcHelper);

     // NS_LOG_INFO ("Creating PGW node.");
     // Ptr<Node> pgw = epcHelper->GetPgwNode ();

     // Create a single RemoteHost
     NS_LOG_INFO ("Creating single remote node.");
     NodeContainer remoteHostContainer;
     remoteHostContainer.Create (1);
     // Ptr<Node> remoteHost = remoteHostContainer.Get (0);

     // installing the internet stack on the remote node
     NS_LOG_INFO ("Installing internet stack on remote node.");
     InternetStackHelper internet;
     internet.Install (remoteHostContainer);

     // creating point to point helper
     NS_LOG_INFO ("Creating PointToPointHelper.");
     PointToPointHelper pointToPoint;
     pointToPoint.SetDeviceAttribute ("DataRate", StringValue (dataRate));
     pointToPoint.SetChannelAttribute ("Delay", StringValue (delay));

     // creating net device container
     NS_LOG_INFO ("Creating NetDeviceContainer.");
     NetDeviceContainer internetDevices = pointToPoint.Install (epcHelper->GetPgwNode (), remoteHostContainer.Get (0));

     // assigning ip addresses
     NS_LOG_INFO ("Assigning IP Addresses.");
     Ipv4AddressHelper ipv4h;
     ipv4h.SetBase ("10.1.1.0", "255.255.255.0");
     Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);

     // turning on static routing for the remote node
     Ipv4StaticRoutingHelper ipv4RoutingHelper;
     Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHostContainer.Get (0)->GetObject<Ipv4> ());
     remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

     // creating the LTE nodes
     NS_LOG_INFO ("Creating LTE nodes.");
     NodeContainer enbNodes, ueNodes;
     enbNodes.Create (1);
     ueNodes.Create (nUes);

     MobilityHelper mobility;
     mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
     mobility.Install (enbNodes);
     mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
     mobility.Install (ueNodes);

     // creating net device container
     NS_LOG_INFO ("Creating NetDeviceContainer for the LTE nodes.");
     NetDeviceContainer enbDevs;
     enbDevs = lteHelper->InstallEnbDevice (enbNodes);
     NetDeviceContainer ueDevs;
     ueDevs = lteHelper->InstallUeDevice (ueNodes);

     NS_LOG_INFO("NetDevices: " << ueDevs.GetN());

     // installing internet stack on the ue nodes
     NS_LOG_INFO ("Installing internet stack on the ue nodes.");
     internet.Install (ueNodes);

     // assign IP address to UEs
     NS_LOG_INFO ("Assigning IP addresses to the ue nodes.");
     Ipv4InterfaceContainer ueIpIface;
     for (uint32_t i = 0; i < ueNodes.GetN (); ++i)
     {
          Ptr<Node> ue = ueNodes.Get (i);
          Ptr<NetDevice> ueLteDevice = ueDevs.Get (i);
          ueIpIface.Add(epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevice)));
          // set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
     }

     lteHelper->Attach (ueDevs, enbDevs.Get (0));

     //TODO: DedicatedEpSBearer not neccessary, there is always a default bearer
     // Ptr<EpcTft> tft = Create<EpcTft> ();
     // EpcTft::PacketFilter pf;
     // pf.localPortStart = 1234;
     // pf.localPortEnd = 1234;
     // tft->Add (pf);
     // lteHelper->ActivateDedicatedEpsBearer (ueDevs, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), tft);

     // installing applications
     NS_LOG_INFO ("Installing applications.");
     DceManagerHelper dceManager;
     DceApplicationHelper dce;
     ApplicationContainer apps;
     dceManager.Install (ueNodes);
     dceManager.Install (remoteHostContainer);

     dce.SetStackSize (1<<20);

     // Launch iec server on the ue nodes
     for (uint32_t i = 0; i < ueNodes.GetN (); ++i) {
          dce.SetBinary (server);
          dce.ResetArguments ();
          dce.ResetEnvironment ();
          dce.AddArgument ("-p 10102");
          dce.AddArgument ("-w 36");
          dce.AddArgument ("-v");
          apps = dce.Install (ueNodes.Get (i));
          apps.Start (Seconds (1.0));
     }

     // Launch iec client on the remote node
     for (uint32_t i = 0; i < ueNodes.GetN (); ++i) {
          dce.SetBinary (client);
          dce.ResetArguments ();
          dce.ResetEnvironment ();
          // dce.AddArgument ("-c 4");
          dce.AddArgument ("-s 1");
          dce.AddArgument ("-p 10102");
          dce.AddArgument (IpHelper::getIp(ueNodes.Get(i)));
          apps = dce.Install (remoteHostContainer.Get(0));
          apps.Start (Seconds (5.0));
     }

     // enabling pcap tracing
     if (pcapTracing) {
          NS_LOG_INFO ("Enabling pcap tracing.");
          pointToPoint.EnablePcapAll (filePrefix, false);
     }

     // enabling ASCII tracing
     if (asciiTracing) {
          NS_LOG_INFO ("Enabling ASCII tracing.");
          AsciiTraceHelper ascii;
          pointToPoint.EnableAsciiAll (ascii.CreateFileStream (filePrefix + ".tr"));
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
