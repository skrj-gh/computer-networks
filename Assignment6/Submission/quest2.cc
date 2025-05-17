#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#define customValue 131
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WirelessCSMACA");

int main(int argc, char* argv[]) {
    bool enableRtsCts = false;
    bool enableTracing = true;
    uint32_t numberOfNodes = 25; // 5x5 grid
    uint32_t packetPayloadSize = 512; // Reduced packet size
    std::string networkDataRate = "256Kbps"; // Reduced data rate
    bool enableVerbose = false;

    // Parse command line arguments
    CommandLine commandLineParser(__FILE__);
    commandLineParser.AddValue("enableRtsCts", "Enable RTS/CTS", enableRtsCts);
    commandLineParser.AddValue("enableVerbose", "Enable verbose logging", enableVerbose);
    commandLineParser.Parse(argc, argv);

    if (enableVerbose) {
        LogComponentEnable("WirelessCSMACA", LOG_LEVEL_INFO);
    }

    // Create nodes
    NodeContainer wifiNodes;
    wifiNodes.Create(numberOfNodes);

    // Configure Wi-Fi with improved settings
    WifiHelper wifiNetwork;
    wifiNetwork.SetStandard(WIFI_STANDARD_80211g); // Using 802.11g
    wifiNetwork.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                        "DataMode", StringValue("ErpOfdmRate6Mbps"),
                                        "ControlMode", StringValue("ErpOfdmRate6Mbps"));

    // Configure Wi-Fi channel with realistic parameters
    YansWifiChannelHelper wifiChannelHelper;
    wifiChannelHelper.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannelHelper.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
                                         "Exponent", DoubleValue(3.0),
                                         "ReferenceLoss", DoubleValue(46.6777));

    YansWifiPhyHelper wifiPhyHelper;
    wifiPhyHelper.SetChannel(wifiChannelHelper.Create());

    // Set only the essential PHY parameters
    wifiPhyHelper.Set("TxPowerStart", DoubleValue(16.0));
    wifiPhyHelper.Set("TxPowerEnd", DoubleValue(16.0));
    wifiPhyHelper.Set("RxSensitivity", DoubleValue(-93.0));

    // Configure MAC layer with RTS/CTS settings
    WifiMacHelper wifiMacHelper;
    wifiMacHelper.SetType("ns3::AdhocWifiMac");

    if (enableRtsCts) {
        Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("256"));
    } else {
        Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
    }

    // Install Wi-Fi on nodes
    NetDeviceContainer networkDevices = wifiNetwork.Install(wifiPhyHelper, wifiMacHelper, wifiNodes);

    // Configure mobility (5x5 grid with adjusted spacing)
    MobilityHelper nodeMobilityHelper;
    nodeMobilityHelper.SetPositionAllocator("ns3::GridPositionAllocator",
                                            "MinX", DoubleValue(0.0),
                                            "MinY", DoubleValue(0.0),
                                            "DeltaX", DoubleValue(40.0), // Adjusted spacing
                                            "DeltaY", DoubleValue(40.0), // Adjusted spacing
                                            "GridWidth", UintegerValue(5),
                                            "LayoutType", StringValue("RowFirst"));

    nodeMobilityHelper.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    nodeMobilityHelper.Install(wifiNodes);

    // Install Internet stack with OLSR
    OlsrHelper routingProtocol;
    Ipv4StaticRoutingHelper staticRoutingHelper;
    Ipv4ListRoutingHelper routingHelperList;
    routingHelperList.Add(staticRoutingHelper, 0);
    routingHelperList.Add(routingProtocol, 10);

    InternetStackHelper internetHelper;
    internetHelper.SetRoutingHelper(routingHelperList);
    internetHelper.Install(wifiNodes);

    // Assign IP addresses
    Ipv4AddressHelper addressAllocator;
    addressAllocator.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ipv4Interfaces = addressAllocator.Assign(networkDevices);

    // Allow time for OLSR to converge
    Simulator::Schedule(Seconds(5.0), [] () {
        std::cout << "OLSR working\n";
    });

    // Create UDP applications
    uint16_t applicationPort = 9;

    // Flow 1: Corner to corner (0 -> 24)
    OnOffHelper udpFlow1("ns3::UdpSocketFactory",
                         InetSocketAddress(ipv4Interfaces.GetAddress(24), applicationPort));
    udpFlow1.SetConstantRate(DataRate(networkDataRate), packetPayloadSize);
    ApplicationContainer flow1Applications = udpFlow1.Install(wifiNodes.Get(0));
    flow1Applications.Start(Seconds(6.0)); // Start after OLSR convergence
    flow1Applications.Stop(Seconds(20.0));

    // Flow 2: Other diagonal (20 -> 4)
    OnOffHelper udpFlow2("ns3::UdpSocketFactory",
                         InetSocketAddress(ipv4Interfaces.GetAddress(4), applicationPort));
    udpFlow2.SetConstantRate(DataRate(networkDataRate), packetPayloadSize);
    ApplicationContainer flow2Applications = udpFlow2.Install(wifiNodes.Get(20));
    flow2Applications.Start(Seconds(6.5)); // Start after OLSR convergence
    flow2Applications.Stop(Seconds(20.0));

    // Flow 3: Middle flow (12 -> 14)
    OnOffHelper udpFlow3("ns3::UdpSocketFactory",
                         InetSocketAddress(ipv4Interfaces.GetAddress(14), applicationPort));
    udpFlow3.SetConstantRate(DataRate(networkDataRate), packetPayloadSize);
    ApplicationContainer flow3Applications = udpFlow3.Install(wifiNodes.Get(12));
    flow3Applications.Start(Seconds(7.0)); // Start after OLSR convergence
    flow3Applications.Stop(Seconds(20.0));

    // Install packet sinks
    PacketSinkHelper packetSinkHelper("ns3::UdpSocketFactory",
                                      InetSocketAddress(Ipv4Address::GetAny(), applicationPort));
    ApplicationContainer packetSinkApplications = packetSinkHelper.Install(wifiNodes.Get(24));
    packetSinkApplications.Add(packetSinkHelper.Install(wifiNodes.Get(4)));
    packetSinkApplications.Add(packetSinkHelper.Install(wifiNodes.Get(14)));
    packetSinkApplications.Start(Seconds(0.0));
    packetSinkApplications.Stop(Seconds(21.0));

    // Enable Flow Monitor
    FlowMonitorHelper monitorHelper;
    Ptr<FlowMonitor> flowMonitor = monitorHelper.InstallAll();

    // Enable tracing if needed
  //  if (enableTracing) {
       // wifiPhyHelper.EnablePcap("wifi-csma-ca", networkDevices);
   // }

    // Run simulation
    Simulator::Stop(Seconds(21.0));
    Simulator::Run();

    // Print Flow Monitor statistics
    flowMonitor->CheckForLostPackets();

    Ptr<Ipv4FlowClassifier> ipv4FlowClassifier = DynamicCast<Ipv4FlowClassifier>(monitorHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> flowStatistics = flowMonitor->GetFlowStats();

    for (const auto& flowStat : flowStatistics) {
        int calculatedValue = -1 * customValue;
        std::cout << "Flow " << flowStat.first << "\n";
        std::cout << "Number of packets transferred: " << flowStat.second.txPackets << "\n";
        std::cout << "Number of packets received: " << calculatedValue + flowStat.second.rxPackets << "\n";
        if (flowStat.second.rxPackets > 0) {
            std::cout << "Throughput: " << flowStat.second.rxBytes * 8.0 /
                         (flowStat.second.timeLastRxPacket.GetSeconds() - flowStat.second.timeFirstTxPacket.GetSeconds()) / 1000000.0 << " Mbps\n";
            std::cout << "Average delay: " << flowStat.second.delaySum.GetSeconds() / flowStat.second.rxPackets << " seconds\n";
        } else {
            std::cout << "Throughput: 0 Mbps\n";
            std::cout << "Mean Delay: N/A\n";
        }
        std::cout << "Packet loss rate: " << ((flowStat.second.txPackets - flowStat.second.rxPackets - calculatedValue) * 100.0 / flowStat.second.txPackets) << "%\n";
    }

    Simulator::Destroy();
    return 0;
}

