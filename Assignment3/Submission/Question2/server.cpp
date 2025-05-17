#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <winsock2.h>
#include <zlib.h>
#include <mutex>
#include <condition_variable>

#pragma comment(lib, "ws2_32.lib")

#define PORT 65432
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
using namespace std;

queue<vector<char>> data_queue;
mutex queue_mutex;
condition_variable data_available;

// Function to handle each client connection
void handle_client(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    while (true) {
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            std::cerr << "Client disconnected or error occurred." << std::endl;
            break;
        }

        // Decompress data
        uLongf decompressed_size = BUFFER_SIZE * 2;
        vector<char> decompressed_data(decompressed_size);
        int result = uncompress((Bytef*)decompressed_data.data(), &decompressed_size, (const Bytef*)buffer, bytes_received);
        
        if (result == Z_OK) {
            decompressed_data.resize(decompressed_size);
            lock_guard<mutex> lock(queue_mutex);
            data_queue.push(decompressed_data);
            data_available.notify_one();
        } else {
            cerr << "Failed to decompress data." << endl;
        }
    }

    closesocket(client_socket);
}

// Function to process data in the queue
void process_data() {
    while (true) {
        unique_lock<mutex> lock(queue_mutex);
        data_available.wait(lock, []{ return !data_queue.empty(); });

        auto data = data_queue.front();
        data_queue.pop();
        lock.unlock();

        cout << "Processed data: " << string(data.begin(), data.end()) << endl;
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." <<endl;
        return 1;
    }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        cerr << "Failed to create socket." << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "Bind failed." << endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, MAX_CLIENTS) == SOCKET_ERROR) {
        cerr << "Listen failed." << endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    cout << "Server listening on port " << PORT << endl;

    thread data_processing_thread(process_data);
    data_processing_thread.detach();

    while (true) {
        SOCKET client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
            cerr << "Accept failed." << endl;
            continue;
        }

        thread client_thread(handle_client, client_socket);
        client_thread.detach();
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
