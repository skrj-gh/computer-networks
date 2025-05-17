#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/random-variable-stream.h"
#include <fstream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CsmaSimulation");

class MetricsCollector {
private:
    std::ofstream throughputFile;
    std::ofstream delayFile;
    std::ofstream lossFile;
    Time lastCollectionTime;
    std::map<FlowId, uint32_t> lastPacketsReceived;
    std::map<FlowId, uint32_t> lastPacketsLost;
    std::map<FlowId, uint64_t> lastBytesReceived;
    std::map<FlowId, Time> lastDelaySum;
    
public:
    MetricsCollector() {
        throughputFile.open("throughput.dat");
        delayFile.open("delay.dat");
        lossFile.open("loss.dat");
        lastCollectionTime = Seconds(0);
    }
    
    ~MetricsCollector() {
        throughputFile.close();
        delayFile.close();
        lossFile.close();
    }
    
    void CollectMetrics(Ptr<FlowMonitor> flowMonitor, FlowMonitorHelper& flowHelper) {
        std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats();
        Time now = Simulator::Now();
        double timeInSeconds = now.GetSeconds();
        double interval = (now - lastCollectionTime).GetSeconds();
        
        if (interval > 0) {
            uint64_t intervalBytes = 0;
            uint32_t intervalLostPackets = 0;
            Time intervalDelay = Time(0);
            uint32_t intervalReceivedPackets = 0;

            for (auto &pair : stats) {
                FlowId flowId = pair.first;
                FlowMonitor::FlowStats &stat = pair.second;

                // Calculate interval metrics for this flow
                uint64_t flowIntervalBytes = stat.rxBytes;
                uint32_t flowIntervalLostPackets = stat.lostPackets;
                Time flowIntervalDelay = stat.delaySum;
                uint32_t flowReceivedPackets = stat.rxPackets;
                
                if (lastBytesReceived.find(flowId) != lastBytesReceived.end()) {
                    flowIntervalBytes -= lastBytesReceived[flowId];
                    flowIntervalLostPackets -= lastPacketsLost[flowId];
                    flowIntervalDelay -= lastDelaySum[flowId];
                    flowReceivedPackets -= lastPacketsReceived[flowId];
                }

                // Update running totals for this interval
                intervalBytes += flowIntervalBytes;
                intervalLostPackets += flowIntervalLostPackets;
                intervalDelay += flowIntervalDelay;
                intervalReceivedPackets += flowReceivedPackets;
                
                // Store current values for next interval
                lastBytesReceived[flowId] = stat.rxBytes;
                lastPacketsReceived[flowId] = stat.rxPackets;
                lastPacketsLost[flowId] = stat.lostPackets;
                lastDelaySum[flowId] = stat.delaySum;
            }

            // Calculate metrics for this interval
            double throughput = (intervalBytes * 8.0) / interval; // bits per second
            double packetLossRate = static_cast<double>(intervalLostPackets) / interval;
            double avgDelay = 0.0;
            
            if (intervalReceivedPackets > 0) {
                avgDelay = intervalDelay.GetSeconds() / intervalReceivedPackets;
            }

            // Store data
            throughputFile << timeInSeconds << " " << throughput << std::endl;
            lossFile << timeInSeconds << " " << packetLossRate << std::endl;
            delayFile << timeInSeconds << " " << avgDelay << std::endl;
            
            // Debug output
            std::cout << "Time: " << timeInSeconds << "s\n";
            std::cout << "  Interval Throughput: " << throughput/1e6 << " Mbps\n";
            std::cout << "  Interval Packet Loss: " << packetLossRate << " packets/s\n";
            std::cout << "  Average Delay: " << avgDelay*1000 << " ms\n";
            std::cout << "-------------------\n";
        }
        
        lastCollectionTime = now;
    }
};

// Collision detection callback function
void CollisionDetectedCallback(Ptr<const Packet> packet) {
    std::cout << "Collision detected at time " << Simulator::Now().GetSeconds() << " seconds\n";
}

int main(int argc, char *argv[]) {
    LogComponentEnable("CsmaSimulation", LOG_LEVEL_INFO);
    
    // Create nodes
    NodeContainer nodes;
    nodes.Create(5);
    
    // Create CSMA channel
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("200Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(MicroSeconds(100)));
    
    // Small queue to increase collision probability
    csma.SetQueue("ns3::DropTailQueue<Packet>",
                  "MaxSize", StringValue("5p")); 
    
    NetDeviceContainer devices = csma.Install(nodes);
    
    // Connect collision callback to each CSMA device
    for (uint32_t i = 0; i < devices.GetN(); ++i) {
        Ptr<CsmaNetDevice> csmaDevice = DynamicCast<CsmaNetDevice>(devices.Get(i));
        if (csmaDevice) {
            csmaDevice->TraceConnectWithoutContext("MacTxDrop", MakeCallback(&CollisionDetectedCallback));
        }
    }
    
    // Install Internet stack
    InternetStackHelper internet;
    internet.Install(nodes);
    
    // Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);
    
    // Create random variables for traffic pattern
    Ptr<UniformRandomVariable> startTimeRand = CreateObject<UniformRandomVariable>();
    startTimeRand->SetAttribute("Min", DoubleValue(0.0));
    startTimeRand->SetAttribute("Max", DoubleValue(1.0));
    
    Ptr<UniformRandomVariable> onTimeRand = CreateObject<UniformRandomVariable>();
    onTimeRand->SetAttribute("Min", DoubleValue(0.1));
    onTimeRand->SetAttribute("Max", DoubleValue(0.5));
    
    Ptr<UniformRandomVariable> offTimeRand = CreateObject<UniformRandomVariable>();
    offTimeRand->SetAttribute("Min", DoubleValue(0.1));
    offTimeRand->SetAttribute("Max", DoubleValue(0.3));
    
    // Create applications with varying patterns
    uint16_t basePort = 9;
    
    for (uint32_t i = 0; i < nodes.GetN(); i++) {
        for (uint32_t j = 0; j < nodes.GetN(); j++) {
            if (i != j) {
                uint16_t port = basePort + i * nodes.GetN() + j;
                
                // Create variable rate OnOff application
                OnOffHelper onoff("ns3::UdpSocketFactory",
                                InetSocketAddress(interfaces.GetAddress(j), port));
                
                // Set random on/off times for each flow
                std::stringstream onTime;
                onTime << "ns3::UniformRandomVariable[Min=0.1|Max=" << onTimeRand->GetValue() << "]";
                std::stringstream offTime;
                offTime << "ns3::UniformRandomVariable[Min=0.1|Max=" << offTimeRand->GetValue() << "]";
                
                onoff.SetAttribute("OnTime", StringValue(onTime.str()));
                onoff.SetAttribute("OffTime", StringValue(offTime.str()));
                onoff.SetAttribute("DataRate", DataRateValue(DataRate("20Mbps")));
                onoff.SetAttribute("PacketSize", UintegerValue(1460));
                
                ApplicationContainer sourceApps = onoff.Install(nodes.Get(i));
                sourceApps.Start(Seconds(startTimeRand->GetValue()));
                sourceApps.Stop(Seconds(10.0));
                
                // Create sink application
                PacketSinkHelper sink("ns3::UdpSocketFactory",
                                    InetSocketAddress(Ipv4Address::GetAny(), port));
                ApplicationContainer sinkApps = sink.Install(nodes.Get(j));
                sinkApps.Start(Seconds(0.0));
                sinkApps.Stop(Seconds(10.0));
            }
        }
    }
    
    // Setup Flow Monitor
    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> flowMonitor = flowHelper.InstallAll();
    
    // Create metrics collector
    MetricsCollector metricsCollector;
    
    // Schedule frequent metrics collection
    for (double t = 0.1; t <= 10.0; t += 0.1) {
        Simulator::Schedule(Seconds(t), &MetricsCollector::CollectMetrics, 
                          &metricsCollector, flowMonitor, std::ref(flowHelper));
    }
    
    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    Simulator::Destroy();
    
    return 0;
}

