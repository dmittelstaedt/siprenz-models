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
#include "ns3/config-store.h"
#include <string>
#include <sstream>

using namespace ns3;
using namespace std;

// ===========================================================================
//
// Note : Tested with libIEC61850.
// ===========================================================================

NS_LOG_COMPONENT_DEFINE ("SimpleLTE");

// Function to get a value as string
template <typename T>
string toString(T const& value) {
     stringstream sstr;
     sstr << value;
     return sstr.str();
}

// Function to get the IP-Address as string from nodecontainer
string getIpFromNodeContainer(NodeContainer& nodes, uint32_t index) {
     Ptr<Node> node =  nodes.Get(index); // Get pointer to ith node in container
     Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); // Get Ipv4 instance of the node
     Ipv4Address addri = ipv4->GetAddress (1, 0).GetLocal (); // Get Ipv4InterfaceAddress of xth interface.
     ostringstream stream;
     addri.Print(stream);
     string ip = stream.str();
     return ip;
}

// Function to get the IP-Address as string from a node
string getIpFromNode(Ptr<Node> node) {
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
     string dataRate = "100Mbps";
     // string delay = "2ms";
     double delay = 0.002;
     bool tracing = false;

     //TODO: ConfigStore
     CommandLine cmd;
     cmd.AddValue ("DataRate", "Datarate of the connection", dataRate);
     cmd.AddValue ("Delay", "Delay of the connection", delay);
     cmd.AddValue ("Tracing", "Tracing with pcap files", tracing);
     cmd.Parse (argc, argv);

     NS_LOG_INFO("Reading Input.");

     NS_LOG_INFO ("Protocol: " + protocol);
     NS_LOG_INFO ("Server: " + server);
     NS_LOG_INFO ("Client: " + client);
     NS_LOG_INFO ("DataRate: " + dataRate);
     NS_LOG_INFO ("Delay: " + toString(delay));
     if (tracing) {
          NS_LOG_INFO ("Tracing: true");
     } else {
          NS_LOG_INFO ("Tracing: false");
     }

     Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
     Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();

     lteHelper->SetEpcHelper (epcHelper);

     Ptr<Node> pgw = epcHelper->GetPgwNode ();

     // Create a single RemoteHost
     NodeContainer remoteHostContainer;
     remoteHostContainer.Create (1);
     Ptr<Node> remoteHost = remoteHostContainer.Get (0);
     InternetStackHelper internet;
     internet.Install (remoteHostContainer);

     // Create the internet
     PointToPointHelper p2ph;
     p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
     p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
     p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (delay)));
     NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
     Ipv4AddressHelper ipv4h;
     ipv4h.SetBase ("10.1.1.0", "255.255.255.0");
     Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);

     NS_LOG_INFO("Internet IP interfaces: " << internetIpIfaces.GetN());

     Ipv4StaticRoutingHelper ipv4RoutingHelper;
     Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
     remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

     NodeContainer enbNodes, ueNodes;
     enbNodes.Create (1);
     ueNodes.Create (2);

     //TODO: how to set the position
     MobilityHelper mobility;
     mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
     mobility.Install (enbNodes);
     mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
     mobility.Install (ueNodes);

     NetDeviceContainer enbDevs;
     enbDevs = lteHelper->InstallEnbDevice (enbNodes);

     NetDeviceContainer ueDevs;
     ueDevs = lteHelper->InstallUeDevice (ueNodes);

     NS_LOG_INFO("NetDevices: " << ueDevs.GetN());

     internet.Install (ueNodes);

     Ipv4InterfaceContainer ueIpIface;
     // assign IP address to UEs
     for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
     {
          Ptr<Node> ue = ueNodes.Get (u);
          Ptr<NetDevice> ueLteDevice = ueDevs.Get (u);
          ueIpIface.Add(epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevice)));
          // set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
     }

     NS_LOG_INFO("Interface Container: " << ueIpIface.GetN ());

     NS_LOG_INFO("IP address of first ue node: " + getIpFromNode(ueNodes.Get(1)));

     lteHelper->Attach (ueDevs, enbDevs.Get (0));

     //TODO: DedicatedEpSBearer not neccessary, there is always a default bearer
     // Ptr<EpcTft> tft = Create<EpcTft> ();
     // EpcTft::PacketFilter pf;
     // pf.localPortStart = 1234;
     // pf.localPortEnd = 1234;
     // tft->Add (pf);
     // lteHelper->ActivateDedicatedEpsBearer (ueDevs, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), tft);

     NS_LOG_INFO ("Installing applications.");
     DceManagerHelper dceManager;
     DceApplicationHelper dce;
     ApplicationContainer apps;
     dceManager.Install (ueNodes);
     dceManager.Install (remoteHostContainer);

     dce.SetStackSize (1<<20);

     // Launch iec server on first ue node
     dce.SetBinary (server);
     dce.ResetArguments ();
     dce.ResetEnvironment ();
     apps = dce.Install (ueNodes.Get (0));
     apps.Start (Seconds (1.0));

     // Launch iec server on second ue node
     // dce.SetBinary (server);
     // dce.ResetArguments ();
     // dce.ResetEnvironment ();
     // apps = dce.Install (ueNodes.Get (1));
     // apps.Start (Seconds (3.0));

     // Launch iec client on the remoteHost
     dce.SetBinary (client);
     dce.ResetArguments ();
     dce.ResetEnvironment ();
     dce.AddArgument ("7.0.0.2");
     apps = dce.Install (remoteHostContainer.Get(0));
     apps.Start (Seconds (3.0));
     apps.Stop (Seconds (5.0));

     // dce.SetBinary (client);
     // dce.ResetArguments ();
     // dce.ResetEnvironment ();
     // dce.AddArgument ("7.0.0.2");
     // apps = dce.Install (ueNodes.Get (1));
     // apps.Start (Seconds (3.0));
     // apps.Stop (Seconds (5.0));

     // Define at the end of the simulation script
     // lteHelper->EnablePhyTraces ();
     // lteHelper->EnableMacTraces ();
     // lteHelper->EnableRlcTraces ();
     // lteHelper->EnablePdcpTraces ();

     if (tracing) {
          NS_LOG_INFO ("Enabling pcap tracing.");
          p2ph.EnablePcapAll ("simple_lte", false);
     }

     NS_LOG_INFO ("Running Simulation.");
     Simulator::Stop (Seconds(10.0));
     Simulator::Run ();
     Simulator::Destroy ();
     NS_LOG_INFO ("Simulation done.");

     return 0;
}
