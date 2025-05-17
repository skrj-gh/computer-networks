#include <bits/stdc++.h>
#include <queue>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <windows.h> // For Sleep and GetTickCount functions
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

using namespace std;
const int processing_rate = 100;
const int NUM_PORTS = 8;           // Number of input/output ports
const int BUFFER_SIZE = 64;        // Buffer size for each input/output
const int MAX_PACKETS = 1000;      // Total number of packets to simulate
//say port 2 has more priority data and others have lower priority .Lowest ones are for best effort
map<string, int> forwarding_table;
int WFQ_weights[8]= {3, 1 , 1 , 2 , 3 , 1 , 1 , 1 };
string poss_ip[NUM_PORTS] = {
    "120.120.1.1",
    "120.120.1.2",
    "120.120.1.3",
    "120.120.1.4",
    "120.120.1.5",
    "120.120.1.6",
    "120.120.1.7",
    "120.120.1.8"
};

struct Packet {
    int id;
    int size;
    string dest_ip;
    int priority;   // Higher number = higher priority
    DWORD arrivalTime;      // Time when packet was generated
    DWORD processingTime;   // Time taken to process the packet
};

// Initialize forwarding table
void initialize_ftable() {
    for(int i = 0; i < NUM_PORTS; i++) {
        forwarding_table[poss_ip[i]] = i; // Map IP to output port index
    }
}

// Queues for each input and output port
queue<Packet> inputQueue[NUM_PORTS];
queue<Packet> outputQueue[NUM_PORTS];

// Pointer to track round-robin scheduling for each output port
int outputPointer[NUM_PORTS] = {0};

// Performance metrics
atomic<int> packetProcessed(0);
atomic<int> totalTurnaroundTime(0);
atomic<int> totalWaitingTime(0);
atomic<int> input_totalPacketDrops(0);
atomic<int> output_totalPacketDrops(0);
atomic<int> totalBufferOccupancy(0);

// Mutex for protecting shared resources
mutex queueMutex;

// Global termination flag
atomic<bool> terminateThreads(false);

// Generate random packet with variable priority
Packet generateRandomPacket(int id) {
    Packet pkt;
    pkt.id = id;
    pkt.size = 1000+rand()%1001;
    pkt.priority = rand() % 2;  // 0 for low priority, 1 for high priority
    pkt.arrivalTime = GetTickCount(); // Current tick count in ms
    pkt.processingTime = 1;     // Fixed processing time for simplicity
    pkt.dest_ip = poss_ip[rand() % NUM_PORTS];
    return pkt;
}

// Add packet to input queue if space allows
bool addPacketToInputQueue(Packet pkt, int where) {// Lock shared resources
    if (inputQueue[where].size() < BUFFER_SIZE) {
        inputQueue[where].push(pkt);
        totalBufferOccupancy++;
        return true;
    }
    input_totalPacketDrops++;
    return false;  // Drop packet if buffer is full
}

// Round Robin Scheduling
void WFQ() {
    while (!terminateThreads) {
        bool processedAny = false;

        for (int o = 0; o < NUM_PORTS; o++) { // Lock shared resources
            for(int i = 0 ; i < WFQ_weights[o] ; i++){
                    if (!inputQueue[o].empty()) {
                        Sleep(5);
                        Packet pkt = inputQueue[o].front();
                        cout << "Weighted Fair Queue: Input " << o << " to Output " << o << " for Packet " << pkt.id << endl;
                        DWORD currentTick = GetTickCount();
                        totalWaitingTime+= currentTick - pkt.arrivalTime;
                        // Check if output queue has space
                        if (outputQueue[forwarding_table[pkt.dest_ip]].size() < BUFFER_SIZE) {
                            outputQueue[forwarding_table[pkt.dest_ip]].push(pkt);
                            processedAny = true;
                        } else {
                            cout << "PACKET " << pkt.id << " DROPPED AT OUTPUT QUEUE " << forwarding_table[pkt.dest_ip] << endl;
                            output_totalPacketDrops++;
                        }
                        pkt.processingTime = (currentTick-pkt.arrivalTime + pkt.size/processing_rate);
                        totalTurnaroundTime += pkt.processingTime;
                        Sleep(pkt.size/processing_rate);
                        inputQueue[o].pop();
                        packetProcessed++;

                        if (packetProcessed >= MAX_PACKETS) {
                            terminateThreads = true;  // Set termination flag
                            break;
                        }
                    
                }
                else {
                    Sleep(5);
                    break;
                }
                
            }
        }
    }
}

// Uniform packet generation
void packetGenerator() {
    int id = 1;
    while (id <= MAX_PACKETS) {
        if (terminateThreads) break;  // Check termination flag
        
        int portIdx = rand() % NUM_PORTS;
        Packet pac = generateRandomPacket(id);
        bool added = addPacketToInputQueue(pac, portIdx);

        if (added) {
            cout << "PACKET " << pac.id << " SUCCESSFULLY ADDED TO INPUT QUEUE " << portIdx << endl;
        } else {
            cout << "PACKET " << pac.id << " DROPPED AT INPUT QUEUE " << portIdx << endl;
        }

        id++;
        Sleep(1);  // Increase packet generation speed
    }
    
    terminateThreads = true;  // Signal the scheduler to stop after finishing the packet generation
}
void non_uni_packetGenerator() {
    int id = 1;
    while (id <= MAX_PACKETS) {
        if (terminateThreads) break;  // Check termination flag
        
        // Non-uniform distribution: port 0 has 3/8 chance, others have 1/8 each
        int portIdx;
        int randValue = rand() % 10;  // Random value between 0 and 7

        if (randValue < 3) {
            portIdx = 0;  // 3 packets to port 0
        } else {
            portIdx = randValue - 2;  // Ports 1-7 (1/8 chance each)
        }

        Packet pac = generateRandomPacket(id);
        bool added = addPacketToInputQueue(pac, portIdx);

        if (added) {
            cout << "PACKET " << pac.id << " SUCCESSFULLY ADDED TO INPUT QUEUE " << portIdx << endl;
        } else {
            cout << "PACKET " << pac.id << " DROPPED AT INPUT QUEUE " << portIdx << endl;
        }

        id++;
        Sleep(1);  // Adjust sleep time as necessary
    }
    
    terminateThreads = true;  // Signal the scheduler to stop after finishing the packet generation
}
void bursty_data(){
    
    int id = 1 ;
    while(id<=MAX_PACKETS){
    int y = rand()%10;
    if(y<7){
        //do nothing
    }
    else {
        int port = rand()%8;
        for(int i=0;i<10;i++){
            Packet pac = generateRandomPacket(id);
            bool added = addPacketToInputQueue(pac, port);
            if (added) {
                cout << "PACKET " << pac.id << " SUCCESSFULLY ADDED TO INPUT QUEUE " << port << endl;
            } else {
                cout << "PACKET " << pac.id << " DROPPED AT INPUT QUEUE " << port << endl;
            }
        }
        id+=10;

    }
    Sleep(1);
    }
    terminateThreads = true;

}



// Function to clear the output queue
void clearOutput() {
    while (!terminateThreads) {
        for (int i = 0; i < NUM_PORTS; i++) {
            lock_guard<mutex> lock(queueMutex);  // Lock shared resources
            if (!outputQueue[i].empty()) {
                Packet pkt = outputQueue[i].front();
                outputQueue[i].pop();
            }
        }
        Sleep(20);  // Reduce sleep time to clear output queues faster
    }
}

int main() {
    srand(static_cast<unsigned int>(time(0)));  // Seed for random number generation
    initialize_ftable();

    // Record the start time
    auto start_time = chrono::steady_clock::now();
    // Create three threads: packet generator, round-robin scheduler, and output clearer
    thread traffic_gen(packetGenerator);
    thread scheduler(WFQ);
    thread output_clearer(clearOutput);

    // Wait for the packet generator and scheduler to complete
    traffic_gen.join();
    scheduler.join();

    // Once packet processing is done, signal the output clearer to terminate
    terminateThreads = true;  // Signal output clearer to terminate as well

    // Wait for the output clearer to finish
    output_clearer.join();

    // Record the end time
    auto end_time = chrono::steady_clock::now();
    auto elapsed_seconds = chrono::duration_cast<chrono::seconds>(end_time - start_time).count();

    // Print final metrics
    cout << "\nSimulation Metrics:\n";
    cout << "--------------------\n";
    cout << "Total Packets Processed: " << packetProcessed.load() << endl;
    cout << "Average Turnaround Time: " 
         << (packetProcessed > 0 ? totalTurnaroundTime.load() / packetProcessed.load() : 0) 
         << " ms" << endl;
    cout << "Average Waiting Time: " 
         << (packetProcessed > 0 ? totalWaitingTime.load() / packetProcessed.load() : 0) 
         << " ms" << endl;
    cout << "Total Packet Drops at input: " << input_totalPacketDrops.load() << endl;
    cout << "Total Packet Drops at output: " << output_totalPacketDrops.load() << endl;
    cout << "Total Buffer Occupancy: " << totalBufferOccupancy.load() << endl;
    cout << "Elapsed Time: " << elapsed_seconds << " seconds" << endl;

    return 0;
   // 
   /*
        thread traffic_gen(non_uni_packetGenerator);
        thread scheduler(WFQ);
        thread output_clearer(clearOutput);

        // Wait for the packet generator and scheduler to complete
        traffic_gen.join();
        scheduler.join();

        // Once packet processing is done, signal the output clearer to terminate
        terminateThreads = true;  // Signal output clearer to terminate as well

        // Wait for the output clearer to finish
        output_clearer.join();

        // Record the end time
        auto end_time = chrono::steady_clock::now();
        auto elapsed_seconds = chrono::duration_cast<chrono::seconds>(end_time - start_time).count();

        // Print final metrics
        cout << "\nSimulation Metrics:\n";
        cout << "--------------------\n";
        cout << "Total Packets Processed: " << packetProcessed.load() << endl;
        cout << "Average Turnaround Time: " 
            << (packetProcessed > 0 ? totalTurnaroundTime.load() / packetProcessed.load() : 0) 
            << " ms" << endl;
        cout << "Average Waiting Time: " 
            << (packetProcessed > 0 ? totalWaitingTime.load() / packetProcessed.load() : 0) 
            << " ms" << endl;
        cout << "Total Packet Drops at input: " << input_totalPacketDrops.load() << endl;
        cout << "Total Packet Drops at output: " << output_totalPacketDrops.load() << endl;
        cout << "Total Buffer Occupancy: " << totalBufferOccupancy.load() << endl;
        cout << "Elapsed Time: " << elapsed_seconds << " seconds" << endl;

        return 0;
        */

/*
        thread traffic_gen(bursty_data);
        thread scheduler(WFQ);
        thread output_clearer(clearOutput);

        // Wait for the packet generator and scheduler to complete
        traffic_gen.join();
        scheduler.join();

        // Once packet processing is done, signal the output clearer to terminate
        terminateThreads = true;  // Signal output clearer to terminate as well

        // Wait for the output clearer to finish
        output_clearer.join();

        // Record the end time
        auto end_time = chrono::steady_clock::now();
        auto elapsed_seconds = chrono::duration_cast<chrono::seconds>(end_time - start_time).count();

        // Print final metrics
        cout << "\nSimulation Metrics:\n";
        cout << "--------------------\n";
        cout << "Total Packets Processed: " << packetProcessed.load() << endl;
        cout << "Average Turnaround Time: " 
            << (packetProcessed > 0 ? totalTurnaroundTime.load() / packetProcessed.load() : 0) 
            << " ms" << endl;
        cout << "Average Waiting Time: " 
            << (packetProcessed > 0 ? totalWaitingTime.load() / packetProcessed.load() : 0) 
            << " ms" << endl;
        cout << "Total Packet Drops at input: " << input_totalPacketDrops.load() << endl;
        cout << "Total Packet Drops at output: " << output_totalPacketDrops.load() << endl;
        cout << "Total Buffer Occupancy: " << totalBufferOccupancy.load() << endl;
        cout << "Elapsed Time: " << elapsed_seconds << " seconds" << endl;

        return 0;

    */
    
}
