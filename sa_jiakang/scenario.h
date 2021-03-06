#ifndef SCENARIO_H

#define SCENARIO_H

#include "nodehelper.h"

class Scenario: public OpenGymEnv
{
private:
  // For Maddpg
  vector<double> pktlossrate;


  uint32_t port;
  // For Scenario Building
  uint32_t construction_size;
  Vector charge_position;


  uint32_t num_uavNodes;
  uint32_t num_ueNodes;
  uint32_t num_crNodes;

  double time_step;
  string topo_type;

  NodeUAVhelper uavHelper;
  NodeUEhelper crHelper;
  NodeUEhelper ueHelper;

  YansWifiPhyHelper wifiPhy;
  string m_datapath;

  void checkthroughput();
  void checkip();
  void checksenderinfo();
  void init_Topo_test (InternetStackHelper &internet_stack);
  void init_Topo_static(InternetStackHelper &internet_stack);
  void timestep_handler();
  void print_header();
  void rl_test ();
  void rl_static_full_energy ();
  void rl_static_dynamic_energy ();
  

public:
  Scenario (/* args */);
  Scenario (uint32_t num_uavNodes_arg, uint32_t num_ueNodes_arg, double time_step_arg,
            string topo_type_arg, uint32_t construction_size_arg, Vector charge_posi_arg);
  // Handle Argments in CommandLine

  // Callback function for postion Change
  static void CourseChangeCallback (std::string path, Ptr<const MobilityModel> model);

  //get WifiPhy
  YansWifiPhyHelper get_wifiPhy ();

  // Demo for Static
  void init_Topo ();

  // Event Handler
  static void ue_app_datarate_handler (Scenario *scenario, uint32_t i, DataRateValue value);
  static void ue_app_state_handler (Scenario *scenario, uint32_t i, string state);
  static void uav_state_handler (Scenario *scenario, uint32_t i, string state);

  // RL env
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  Ptr<OpenGymSpace> GetActionSpace();
  Ptr<OpenGymSpace> GetObservationSpace();
  bool GetGameOver();
  Ptr<OpenGymDataContainer> GetObservation();
  float GetReward();
  std::string GetExtraInfo();
  bool ExecuteActions(Ptr<OpenGymDataContainer> action);

  ~Scenario ();
};

#endif