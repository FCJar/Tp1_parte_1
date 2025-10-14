/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
  uint32_t clientes = 1;
  uint32_t pacotes = 1;
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);


  CommandLine cmd;
  cmd.AddValue("clientes", "Digite o numero de clientes", clientes);
  cmd.AddValue("pacotes", "Digite o numero de pacotes", pacotes);
  cmd.Parse(argc, argv);
  if(clientes >5) clientes = 5;
  if(clientes <1) clientes = 1;
  if(pacotes >5) pacotes = 5;
  if(pacotes <1) pacotes = 1;

  NodeContainer nodeServer;
  nodeServer.Create(1);
  NodeContainer nodeClient;
  nodeClient.Create(clientes);



  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  InternetStackHelper stack;
  stack.Install(nodeServer);
  stack.Install (nodeClient);
  UdpEchoServerHelper echoServer (15);
  ApplicationContainer serverApps = echoServer.Install (nodeServer.Get (0));
  serverApps.Start (Seconds (0.0));
  serverApps.Stop (Seconds (20.0));
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();



  Ipv4AddressHelper address;
  std::vector(Ipv4InterfaceContainer) interCliente;
  for(uint32_t i = 0; i < clientes; ++i)
  {
   NodeContainer pair(nodeServer.Get(0), nodeClient.Get(i));
   NetDeviceContainer devices = pointToPoint.Install(pair);
   std::ostringstream subnet;
   subnet << "10.1." << i+1 << ".0";
   address.SetBase(subnet.str().c_str(), "255.255.255.0");
   Ipv4InterfaceContainer interfaces = address.Assign(devices);
   interCliente.push_back(interfaces);
  }
  
  for(uint32_t i = 0; i < clientes; ++i)
  {
   UdpEchoClientHelper echoClient(interCliente[i].GetAddress(0), 15);
   echoClient.SetAttribute("MaxPackets", UintegerValue(pacotes));
   echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
   echoClient.SetAttribute("PacketSize", UintegerValue(1024));
   ApplicationContainer clientApps = echoClient.Install(nodeClient.Get(i));
   double tempoAleatorio = rand->GetValue(2.0, 7.0);
   clientApps.Start(Seconds(tempoAleatorio));
   clientApps.Stop(Seconds(20.0));
  }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
