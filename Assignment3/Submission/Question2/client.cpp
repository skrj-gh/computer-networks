#include <iostream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <zlib.h>
#include <windows.h>  // For Sleep
#include<ctime>
#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define PORT 65432
#define BUFFER_SIZE 1024
#define MAX_CWND 1024  // Maximum congestion window size
using namespace std;
// Function to compress data before sending
vector<char> compress_data(const string& data) {
    uLongf compressed_size = compressBound(data.size());
    vector<char> compressed_data(compressed_size);
    
    int result = compress((Bytef*)compressed_data.data(), &compressed_size, (const Bytef*)data.c_str(), data.size());
    if (result == Z_OK) {
        compressed_data.resize(compressed_size);
    } else {
        cerr << "Compression failed." << endl;
        compressed_data.clear();
    }
    
    return compressed_data;
}

// Function to simulate TCP Reno congestion control
void tcp_reno(SOCKET client_socket) {
    
    int cwnd = 1;            // Congestion window starts at 1
    int ssthresh = 64;       // Slow start threshold
    int acked = 0;           // Simulated acknowledgment counter

    while (true) {
        int temp=-10 +rand()% 45;
        int humid = 10 +rand()%90;
        int pressure =800 + rand()%200;
        string weather_data = "Temperature: "+to_string(temp)+"C, Humidity:"+ to_string(humid)+"%, Pressure: " +to_string(pressure)+ " hPa";
        // Compress data before sending
        vector<char> compressed_data = compress_data(weather_data);
        if (compressed_data.empty()) {
            continue;
        }

        // Send the compressed data in chunks based on the current congestion window
        int data_size = compressed_data.size();
        int bytes_sent = 0;

        while (bytes_sent < data_size) {
            int chunk_size = min(cwnd, data_size - bytes_sent);
            int result = send(client_socket, compressed_data.data() + bytes_sent, chunk_size, 0);

            if (result == SOCKET_ERROR) {
                cerr << "Failed to send data." << endl;
                return;
            }

            bytes_sent += result;

            // Simulate receiving an ACK (for simplicity, assuming ACKs are always received)
            acked++;

            // Slow Start phase
            if (cwnd < ssthresh) {
                cwnd = min(cwnd * 2, MAX_CWND);  // Exponential growth
            } 
            // Congestion Avoidance phase
            else {
                cwnd = min(cwnd + 1, MAX_CWND);  // Linear growth
            }

            // Simulate waiting time before sending the next chunk of data
            Sleep(100); // Simulate network delay
        }

        // Simulate packet loss after some time
        if (acked % 20 == 0 || cwnd>= MAX_CWND) {
            ssthresh =cwnd / 2;  // Cut ssthresh in half
            cwnd = cwnd / 2; 
            cout << "Packet loss detected, reducing cwnd to " << cwnd << endl;
        }

        // Simulate delay between sending weather data
        Sleep(1000);
    }
}

int main() {
    
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return 1;
    }

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        cerr << "Failed to create socket." << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "Connection failed." << endl;
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }
    
    

    // Implement TCP Reno
    tcp_reno(client_socket);

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
