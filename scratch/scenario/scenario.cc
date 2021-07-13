#include "scenario.h"
Scenario::Scenario (/* args */)
    : port (9),
      bytesTotal (0),
      packetsReceived (0),
      m_CSVfileName ("./scratch/scenario/route.csv"),
      m_traceMobility (false),
      time_step (4),
      topo_type ("test"),
      uavHelper (NodeUAVhelper (num_uavNodes)),
      ueHelper (NodeUEhelper (num_ueNodes, time_step)),
      crHelper (NodeUEhelper (num_crNodes, time_step))

{
  // Physic Setting
  wifiPhy = YansWifiPhyHelper::Default ();
  this->wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel", "MaxRange", DoubleValue (100));
  this->wifiPhy.SetChannel (wifiChannel.Create ());
}

Scenario::~Scenario ()
{
}

void
Scenario::CheckThroughput ()
{
  double kbs = (bytesTotal * 8.0) / 1000;
  bytesTotal = 0;
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds ());
  //修正路由
  for (uint32_t i = 0; i < num_ueNodes; i++)
    {
      uint32_t block = ueHelper.getUEBlock (i);
      switch (block)
        {
        case 1:
          ueHelper.Connect_to_UAV (i, wifiPhy, uavHelper, 0);
          break;
        case 3:
          ueHelper.Connect_to_UAV (i, wifiPhy, uavHelper, 1);
          break;
        case 5:
          ueHelper.Connect_to_UAV (i, wifiPhy, uavHelper, 1);
          break;
        case 7:
          ueHelper.Connect_to_UAV (i, wifiPhy, uavHelper, 2);
          break;
        case 9:
          ueHelper.Connect_to_UAV (i, wifiPhy, uavHelper, 3);
          break;
        default:
          ueHelper.Connect_to_UAV (i, wifiPhy, uavHelper, 1);
          break;
        }
    }

  std::ofstream out (m_CSVfileName.c_str (), std::ios::app);

  out << (Simulator::Now ()).GetSeconds () << "," << kbs << "," << packetsReceived << ","
      << m_nSinks << "," << m_protocolName << "," << m_txp << "" << std::endl;

  out.close ();
  packetsReceived = 0;
  Simulator::Schedule (Seconds (time_step), &Scenario::CheckThroughput, this);
}

std::string
Scenario::CommandSetup (int argc, char **argv)
{
  CommandLine cmd;
  // Throughout
  cmd.AddValue ("CSVfileName", "The name of the CSV output file name", m_CSVfileName);
  cmd.AddValue ("traceMobility", "Enable mobility tracing", m_traceMobility);

  //Scenario Building
  cmd.AddValue ("numUAVs", "number of UAVs", num_uavNodes);
  cmd.AddValue ("numUEs", "number of UEs", num_ueNodes);
  cmd.AddValue ("topo_type", "type of Topo", topo_type);
  cmd.AddValue ("ue_mobility_type", "type of UE Mobility", ueHelper.mobility_type);

  cmd.Parse (argc, argv);
  return m_CSVfileName;
}

void
Scenario::CourseChangeCallback (std::string path, Ptr<const MobilityModel> model)
{
  Vector position = model->GetPosition ();
  std::cout << "CourseChange " << path << " x=" << position.x << ", y=" << position.y
            << ", z=" << position.z << std::endl;
}

YansWifiPhyHelper
Scenario::get_wifiPhy ()
{
  return this->wifiPhy;
}

void
Scenario::init_Topo ()
{
  InternetStackHelper internet_stack;
  Ipv4AddressHelper ipAddrs;
  uavHelper.init_UAVs (wifiPhy, internet_stack);

  if (topo_type == "test")
    init_Topo_test (internet_stack);
}

void
Scenario::init_Topo_test (InternetStackHelper &internet_stack)
{
  NS_LOG_UNCOND ("Set 4 UAV in the position of center in construction site");
  uavHelper.setUAVPosition (0, Vector (100, 100, 10));
  uavHelper.setUAVPosition (1, Vector (200, 100, 10));
  uavHelper.setUAVPosition (2, Vector (100, 200, 10));
  uavHelper.setUAVPosition (3, Vector (200, 200, 10));

  ueHelper.init_UEs (internet_stack);
  crHelper.init_UEs (internet_stack);
  crHelper.setUEPosition (0, Vector (50, 50, 0));
  crHelper.Connect_to_UAV (0, wifiPhy, uavHelper, 0);
  //修正路由,Checkthroughtout会出错
  for (uint32_t i = 0; i < num_ueNodes; i++)
    {
      uint32_t block = ueHelper.getUEBlock (i);
      switch (block)
        {
        case 1:
          ueHelper.Connect_to_UAV (i, wifiPhy, uavHelper, 0);
          break;
        case 3:
          ueHelper.Connect_to_UAV (i, wifiPhy, uavHelper, 1);
          break;
        case 7:
          ueHelper.Connect_to_UAV (i, wifiPhy, uavHelper, 2);
          break;
        case 9:
          ueHelper.Connect_to_UAV (i, wifiPhy, uavHelper, 3);
          break;
        default:
          ueHelper.Connect_to_UAV (i, wifiPhy, uavHelper, 1);
          break;
        }
    }

  //设定Application
  stringstream ss;
  uint64_t rate = 60 * 1024 * 8 * 1024; //60MBps
  ss << rate;
  string rate_str = ss.str () + "bps";
  //LogComponentEnableAll(LOG_LEVEL_INFO);
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", StringValue ("1024"));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("2048bps"));
  OnOffHelper onoff1 ("ns3::UdpSocketFactory", Address ());
  onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
  Ptr<Socket> sink =
      SetupPacketReceive (crHelper.interfaces[0].GetAddress (0), crHelper.NC_UEs.Get (0));
  for (uint32_t i = 0; i < num_ueNodes; i++)
    {
      AddressValue remoteAddress (InetSocketAddress (crHelper.interfaces[0].GetAddress (0), port));
      onoff1.SetAttribute ("Remote", remoteAddress);

      Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
      app_c.push_back (onoff1.Install (ueHelper.NC_UEs.Get (i)));
      app_c[i].Start (Seconds (var->GetValue (15, 16)));
      app_c[i].Stop (Seconds (100));
    }

  //CR 用作接受， UE用作发送
  CheckThroughput ();

  //输出参数表头
  //blank out the last output file and write the column headers
  std::ofstream out (m_CSVfileName.c_str ());
  out << "SimulationSecond,"
      << "ReceiveRate,"
      << "PacketsReceived,"
      << "NumberOfSinks,"
      << "RoutingProtocol,"
      << "TransmissionPower" << std::endl;
  out.close ();

  // Animation Setting
  AnimationInterface anim ("./scratch/scenario/static-case.xml");
  anim.EnablePacketMetadata (true);
  for (uint32_t i = 0; i < num_uavNodes; i++)
    {
      stringstream ss;
      ss << i;
      string name = "UAV" + ss.str ();
      anim.UpdateNodeDescription (i, name);
    }
  for (uint32_t i = 0; i < num_ueNodes; i++)
    {
      stringstream ss;
      ss << i;
      string name = "UE" + ss.str ();
      anim.UpdateNodeDescription (i + num_uavNodes, name);
    }


}

std::string
Scenario::PrintReceivedPacket (Ptr<Socket> socket, Ptr<Packet> packet, Address senderAddress)
{
  std::ostringstream oss;

  oss << Simulator::Now ().GetSeconds () << " " << socket->GetNode ()->GetId ();

  if (InetSocketAddress::IsMatchingType (senderAddress))
    {
      InetSocketAddress addr = InetSocketAddress::ConvertFrom (senderAddress);
      oss << " received one packet from " << addr.GetIpv4 ();
    }
  else
    {
      oss << " received one packet!";
    }
  return oss.str ();
}

void
Scenario::ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address senderAddress;
  while ((packet = socket->RecvFrom (senderAddress)))
    {
      bytesTotal += packet->GetSize ();
      packetsReceived += 1;
      NS_LOG_UNCOND (Scenario::PrintReceivedPacket (socket, packet, senderAddress));
    }
}

Ptr<Socket>
Scenario::SetupPacketReceive (Ipv4Address addr, Ptr<Node> node)
{
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback (&Scenario::ReceivePacket, this));

  return sink;
}

int
main (int argc, char **argv)
{
  NS_LOG_UNCOND ("Start!");
  Scenario scen;
  scen.CommandSetup (argc, argv);
  scen.init_Topo ();
  
  // Start simulatation
  Simulator::Stop (Seconds (100));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}