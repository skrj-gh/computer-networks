#include <bits/stdc++.h>
#include <chrono>
#include <fstream>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#define ll long long
using namespace std;

class triplet {
public:
    int x; int y; int z;
};

struct dronestate {
    int speed;
    triplet pos;
    int state;
};

int Y;
vector<sockaddr_in> store;
char myArray[200];

// XOR function
void xorData(char* data, int len) {
    for (int i = 0; i < len; i++) {
        data[i] ^= myArray[i % 200];
    }
}

void tcp_connect_manager(SOCKET acceptsock) {
    struct dronestate curr;
    while (true) { // Keep the connection open to receive multiple messages
        int bytess = recv(acceptsock, (char*)(&curr), sizeof(curr), 0);
        if (bytess <= 0) {
            // Connection closed or error occurred
            cout << "Connection closed or recv failed: " << WSAGetLastError() << endl;
            break;
        } else {
            xorData((char*)&curr, sizeof(curr));  // XOR after receiving
            printf("Received: speed is %d, state %d, position (%d, %d, %d)\n", 
                   curr.speed, curr.state, curr.pos.x, curr.pos.y, curr.pos.z);
        }
    }
    closesocket(acceptsock); // Close the socket after done with receiving messages
}

void accept_telemetry(SOCKET server) {
    while (true) {
        SOCKET acceptsock;
        acceptsock = accept(server, NULL, NULL);
        if (acceptsock == INVALID_SOCKET) {
            cout << "accept failed" << endl;
            WSACleanup();
            return;
        }
        tcp_connect_manager(acceptsock);
    }
}

void udp_accept_manager(SOCKET server_udp) {
    char recive[200];
    sockaddr_in clientAddr;
    int clength = int(sizeof(clientAddr));
    int bytess = recvfrom(server_udp, recive, 200, 0, (struct sockaddr*)&clientAddr, &clength);
    if (bytess >= 0) {
        xorData(recive, bytess);  // XOR after receiving
        printf("Received: %s\n", recive);
        store.push_back(clientAddr);
        Y++;
    }
}

void udp_thread(SOCKET udp_sock) {
    while (true) {
        udp_accept_manager(udp_sock);
    }
}

void sendcomm(SOCKET server_udp) {
    char COMMAND[200] = "go right";
    xorData(COMMAND, strlen(COMMAND));  // XOR before sending
    if (Y == 0) return;
    int bytes;
    for (int i = 1; i <= Y; i++) {
        bytes = sendto(server_udp, COMMAND, strlen(COMMAND), 0, (struct sockaddr*)&store[i - 1], sizeof(store[i - 1]));
    }
    if (bytes <= 0) {
        cout << "not working" << endl;
        return;
    }
}

void command(SOCKET udp_sock) {
    while (true) {
        sendcomm(udp_sock);
        this_thread::sleep_for(chrono::seconds(5));
    }
}

int main() {
    // Fill the myArray with some data
    const char* pattern = "ABC123";
    int patternLength = 6; // Length of "ABC123"
    for (int i = 0; i < 200; i++) {
        myArray[i] = pattern[i % patternLength];
    }

    cout << "ran this " << endl;
    WSADATA wsadata;
    int wsaerr;
    WORD versionreq = MAKEWORD(2, 2);
    wsaerr = WSAStartup(versionreq, &wsadata);
    if (wsaerr != 0) {
        cout << "Failed to initialize Winsock: " << wsaerr << endl;
    } else {
        cout << "Winsock initialized successfully.\n" << endl;
    }

    SOCKET server = INVALID_SOCKET;
    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKET server_udp = INVALID_SOCKET;
    server_udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server == INVALID_SOCKET || server_udp == INVALID_SOCKET) {
        cout << "SOCKET COULD NOT BE FORMED: " << WSAGetLastError() << endl;
        WSACleanup();
        return 0;
    } else {
        sockaddr_in serv;
        serv.sin_family = AF_INET;
        serv.sin_addr.s_addr = inet_addr("127.0.0.1");
        serv.sin_port = htons(55555);
        if (bind(server, (SOCKADDR*)&serv, sizeof(serv)) == SOCKET_ERROR) {
            cout << "Bind failed: " << WSAGetLastError() << endl;
            WSACleanup();
            return 0;
        }
        if (bind(server_udp, (SOCKADDR*)&serv, sizeof(serv)) == SOCKET_ERROR) {
            cout << "Bind failed: " << WSAGetLastError() << endl;
            WSACleanup();
            return 0;
        }
        cout << "Bind successful, ready for connections." << endl;
        if (listen(server, 5) == SOCKET_ERROR) {
            cout << "Listen failed: " << WSAGetLastError() << endl;
            return -1;
        }
    }

    thread tcpThread(accept_telemetry, server);
    thread udpThread(udp_thread, server_udp);
    thread commandThread(command, server_udp);
    tcpThread.join();
    udpThread.join();
    commandThread.join();

    closesocket(server);
    closesocket(server_udp);
    WSACleanup();
    return 0;
}
