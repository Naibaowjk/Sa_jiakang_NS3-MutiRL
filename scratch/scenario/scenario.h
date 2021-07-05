#ifndef SCENARIO_H

#define SCENARIO_H

#include "nodehelper.h"


class Scenario
{
private:
  // For Throughput & Output
  Ptr<Socket> SetupPacketReceive (Ipv4Address addr, Ptr<Node> node);
  void ReceivePacket (Ptr<Socket> socket);
  void CheckThroughput ();
  std::string PrintReceivedPacket (Ptr<Socket> socket, Ptr<Packet> packet, Address senderAddress);
  uint32_t port;
  uint32_t bytesTotal;
  uint32_t packetsReceived;


  std::string m_CSVfileName;
  int m_nSinks;
  std::string m_protocolName;
  double m_txp;
  bool m_traceMobility;

  // For Scenario Building
  uint32_t num_uavNodes = 4;
  uint32_t num_ueNodes = 10;
  uint32_t num_crNodes = 1;

  double time_step;
  string topo_type;
  NodeUAVhelper uavHelper;
  NodeUEhelper ueHelper;

  NodeContainer NC_UEs, NC_CR;
  NetDeviceContainer NDC_UAVs_adhoc, NDC_UAVs_ap, NDC_UEs, NDC_CR;
  YansWifiPhyHelper wifiPhy;

  
  void init_Topo_static(InternetStackHelper &internet_stack);
public:
  Scenario (/* args */);

  // Handle Argments in CommandLine
  std::string CommandSetup (int argc, char **argv);

  // Callback function for postion Change
  static void CourseChangeCallback (std::string path, Ptr<const MobilityModel> model);

  //get WifiPhy
  YansWifiPhyHelper get_wifiPhy ();

  // Demo for Static
  void init_Topo ();
  ~Scenario ();
};

#endif