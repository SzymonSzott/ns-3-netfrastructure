/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 SEBASTIEN DERONNE
 * Copyright (c) 2020 AGH University of Science and Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Szymon Szott <szott@kt.agh.edu.pl>
 * Based on he-wifi-network.cc by S. Deronne <sebastien.deronne@gmail.com>
 * Last update: 2020-06-13 08:29:59
 */

#include "ns3/core-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/flow-monitor-module.h"
#include <iomanip>

// This scenario allows to measure the data link performance of an IEEE 802.11ax Wi-Fi network.
// 
//   AP  STA ... STA
//    *  *       *
//    |  |       |
//   n0  n1      nWifi
//
// The figure above illustrates the number of Wi-Fi stations and not their positions: all stations 
// are placed at point (0, 0, 0) as we are interested in performance under perfect radio conditions.
// 
// The stations generate constant traffic so as to saturate the channel.
// The user can specify the number of stations and MCS value used.
// The simulation output is the aggregate network throughput.
//
// This scenario serves as an example simulation model for nETFRAStructure:
// https://github.com/SzymonSzott/ns-3-netfrastructure

bool fileExists(const std::string& filename);

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("he-wifi-performance");

int main (int argc, char *argv[])
{
  
  // Initialize default simulation parameters
  uint32_t nWifi = 1;   //Number of transmitting stations
  int mcs = 11; // Default MCS is set to highest value
  int channelWidth = 20; //Default channel width [MHz]
  int gi = 800; //Default guard interval [ns]
  double simulationTime = 10; // Simulation time [s]
  
  // Parse command line arguments
  CommandLine cmd;
  cmd.AddValue ("nWifi", "Number of stations", nWifi);  
  cmd.Parse (argc,argv);

  // Print simulation settings to screen
  std::cout << std::endl << "Simulating an IEEE 802.11ax network with the following settings:" << std::endl;
  std::cout << "- number of transmitting stations: " << nWifi << std::endl;  
  std::cout << "- frequency band: 5 GHz" << std::endl;  
  std::cout << "- modulation and coding scheme (MCS): " << mcs << std::endl;  
  std::cout << "- channel width: " << channelWidth << " MHz" << std::endl;  
  std::cout << "- guard interval: " << gi << " ms" << std::endl;    
  std::cout << "- run number: " << RngSeedManager::GetRun() << std::endl;    

  // Create AP and stations
  NodeContainer wifiApNode;
  wifiApNode.Create (1);
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);

  // Configure wireless channel
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  Ptr<YansWifiChannel> channel;
  YansWifiChannelHelper channelHelper = YansWifiChannelHelper::Default ();
  phy.SetChannel (channelHelper.Create ());  
 
  // Create and configure Wi-Fi network
  WifiMacHelper mac;
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211ax_5GHZ);

  std::ostringstream oss;
  oss << "HeMcs" << mcs;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue (oss.str ()),
                                "ControlMode", StringValue (oss.str ())); //Set MCS

  Ssid ssid = Ssid ("ns3-80211ax"); //Set SSID

  mac.SetType ("ns3::StaWifiMac",
                "Ssid", SsidValue (ssid));

  // Create and configure Wi-Fi interfaces
  NetDeviceContainer staDevice;
  staDevice = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac",
                "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevice;
  apDevice = wifi.Install (phy, mac, wifiApNode);

  // Set channel width and guard interval on all interfaces of all nodes
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/ChannelWidth", UintegerValue (channelWidth));
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/HeConfiguration/GuardInterval", TimeValue (NanoSeconds (gi)));  

  // Configure mobility
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);
  mobility.Install (wifiStaNodes);

  // Install an Internet stack
  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  // Configure IP addressing
  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer staNodeInterface;
  Ipv4InterfaceContainer apNodeInterface;

  staNodeInterface = address.Assign (staDevice);
  apNodeInterface = address.Assign (apDevice);

  // Install applications (traffic generators)
  ApplicationContainer sourceApplications, sinkApplications;
  uint32_t portNumber = 9;
  for (uint32_t index = 0; index < nWifi; ++index) //Loop over all stations (which transmit to the AP)
    {
      auto ipv4 = wifiApNode.Get (0)->GetObject<Ipv4> (); //Get destination's IP interface
      const auto address = ipv4->GetAddress (1, 0).GetLocal (); //Get destination's IP address
      InetSocketAddress sinkSocket (address, portNumber++); //Configure destination socket
      OnOffHelper onOffHelper ("ns3::UdpSocketFactory", sinkSocket); //Configure traffic generator: UDP, destination socket
      onOffHelper.SetConstantRate (DataRate (150e6 / nWifi), 1000);  //Set data rate (150 Mb/s divided by no. of transmitting stations) and packet size [B]
      sourceApplications.Add (onOffHelper.Install (wifiStaNodes.Get (index))); //Install traffic generator on station
      PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", sinkSocket); //Configure traffic sink
      sinkApplications.Add (packetSinkHelper.Install (wifiApNode.Get (0))); //Install traffic sink on AP
    }

  // Configure application start/stop times
  // Note: 
  // - source starts transmission at 1.0 s
  // - source stops at simulationTime+1
  // - simulationTime reflects the time when data is sent
  sinkApplications.Start (Seconds (0.0));
  sinkApplications.Stop (Seconds (simulationTime + 1));
  sourceApplications.Start (Seconds (1.0));
  sourceApplications.Stop (Seconds (simulationTime + 1));

  //Install FlowMonitor
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  // Define simulation stop time
  Simulator::Stop (Seconds (simulationTime + 1));
  
  // Print information that the simulation will be executed
  std::clog << std::endl << "Starting simulation... ";

  // Run the simulation!
  Simulator::Run ();

  // Print information that the simulation has finished
  std::clog << ("done!") << std::endl;  

  // === Begin main part of nETFRAStructure code ===
  // Create (if necessary) and open output CSV file
  std::ofstream myfile;
  std::string outputCsv = "he-wifi-performance.csv";
  if (fileExists(outputCsv)) {
    // If the file exists, append to it
    myfile.open (outputCsv, std::ios::app); 
  }
  else {
    // If the file does not exist, create it and set the header line
    myfile.open (outputCsv, std::ios::app);  
    myfile << "Timestamp,nWifi,RngRun,FlowSrc,Throughput" << std::endl;
  }

  //Get timestamp
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);

  // Calculate per-flow throughput and print results to file
  double flowThr=0;
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i) {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
    flowThr=i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ()) / 1e6; //Throughput in Mb/s
    myfile << std::put_time(&tm, "%Y-%m-%d %H:%M") << "," << nWifi << "," << RngSeedManager::GetRun() << "," << t.sourceAddress << "," << flowThr << std::endl;
  }
  myfile.close();
  // === End main part of nETFRAStructure code ===  

  // Calculate network throughput
  double throughput = 0;
  for (uint32_t index = 0; index < sinkApplications.GetN (); ++index) //Loop over all traffic sinks
  {
    uint64_t totalBytesThrough = DynamicCast<PacketSink> (sinkApplications.Get (index))->GetTotalRx (); //Get amount of bytes received
    // std::cout << "Bytes received: " << totalBytesThrough << std::endl;
    throughput += ((totalBytesThrough * 8) / (simulationTime * 1000000.0)); //Mbit/s 
  }

  //Print results
  std::cout << "Results: " << std::endl;
  std::cout << "- network throughput: " << throughput << " Mbit/s" << std::endl;

  //Clean-up
  Simulator::Destroy ();

  return 0;
}

bool fileExists(const std::string& filename)
{
    std::ifstream f(filename.c_str());
    return f.good();   
}