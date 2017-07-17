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
     uint32_t nSpokes = 8; // Default number of nodes in the star (without router)

     CommandLine cmd;
     cmd.AddValue ("DataRate", "Datarate of the connection", dataRate);
     cmd.AddValue ("Delay", "Delay of the connection", delay);
     cmd.AddValue ("Tracing", "Tracing with pcap files", tracing);
     cmd.AddValue ("nSpokes", "Number of nodes to place in the star", nSpokes);
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
     NS_LOG_INFO ("nSpokes: " << nSpokes);

     NS_LOG_INFO ("Building star topology.");

     NS_LOG_INFO ("Creating PointToPointHelper.");
     PointToPointHelper pointToPoint;
     pointToPoint.SetDeviceAttribute ("DataRate", StringValue (dataRate));
     pointToPoint.SetChannelAttribute ("Delay", StringValue (delay));
     PointToPointStarHelper star (nSpokes, pointToPoint);

     NS_LOG_INFO ("Installing internet stack on all nodes.");
     InternetStackHelper internet;
     star.InstallStack (internet);

     NS_LOG_INFO ("Assigning IP Addresses.");
     star.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"));

     // Turn on global static routing so we can actually be routed across the
     // star.
     Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

     NS_LOG_INFO ("Installing applications.");

     DceManagerHelper dceManager;
     DceApplicationHelper dce;
     ApplicationContainer apps;

     dce.SetStackSize (1<<20);

     for (uint32_t i = 0; i < star.SpokeCount ()-1; ++i) {
          dceManager.Install (star.GetSpokeNode (i));
          dce.SetBinary (server);
          dce.ResetArguments ();
          dce.ResetEnvironment ();
          // to check whether different server instances are running --> servers are starting with different timestamps
          apps = dce.Install (star.GetSpokeNode (i));
          apps.Start (Seconds (double(i+2)));
          apps.Stop (Seconds (30.0));
     }

     // Install the client on the last node
     dceManager.Install (star.GetSpokeNode(star.SpokeCount()-1));
     dce.SetBinary (client);
     dce.ResetArguments ();
     dce.ResetEnvironment ();
     dce.AddArgument("10.1.1.2");
     apps = dce.Install (star.GetSpokeNode(star.SpokeCount()-1));
     apps.Start (Seconds (10.0));
     apps.Stop (Seconds (12.0));

     // Install the client on the last node to ask another Server
     // dce.SetBinary ("simple_iec_client");
     // dce.ResetArguments ();
     // dce.ResetEnvironment ();
     // dce.AddArgument("10.1.4.2");
     // apps = dce.Install (star.GetSpokeNode(star.SpokeCount()-1));
     // apps.Start (Seconds (20.0));
     // apps.Stop (Seconds (22.0));

     if (tracing) {
          NS_LOG_INFO ("Enabling pcap tracing.");
          pointToPoint.EnablePcapAll ("generic-star", false);
     }

     NS_LOG_INFO ("Running Simulation.");
     Simulator::Stop (Seconds(30.0));
     Simulator::Run ();
     Simulator::Destroy ();
     NS_LOG_INFO ("Simulation done.");

     return 0;
}
