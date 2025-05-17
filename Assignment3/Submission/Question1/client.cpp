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

char myArray[200];

// XOR function
void xorData(char* data, int len) {
    for (int i = 0; i < len; i++) {
        data[i] ^= myArray[i % 200];
    }
}

int main() {
    // Fill the myArray with some data
    const char* pattern = "ABC123";
    int patternLength = 6; // Length of "ABC123"
    for (int i = 0; i < 200; i++) {
        myArray[i] = pattern[i % patternLength];
    }

    WSADATA wsadata;
    int wsaerr;
    WORD versionreq = MAKEWORD(2, 2);
    wsaerr = WSAStartup(versionreq, &wsadata);
    if (wsaerr != 0) {
        cout << "failed" << endl;
    } else {
        cout << "we did it " << endl;
    }

    SOCKET client = INVALID_SOCKET;
    client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client == INVALID_SOCKET) {
        cout << "SOCKET COULD NOT BE FORMED " << WSAGetLastError() << endl;
        WSACleanup();
        return 0;
    } else {
        sockaddr_in client_struc;
        client_struc.sin_family = AF_INET;
        client_struc.sin_addr.s_addr = inet_addr("127.0.0.1");
        client_struc.sin_port = htons(55555);
        char buffer[200] = "READY";

        // XOR the buffer before sending
        xorData(buffer, strlen(buffer));

        int byte = sendto(client, (const char*)buffer, strlen(buffer), 0, (struct sockaddr*)&client_struc, sizeof(client_struc));
        if (byte == -1) {
            cout << "error could not send data" << endl;
            WSACleanup();
            return 0;
        } else {
            cout << endl;
            cout << "Data sent" << endl;
            while(true){
            sockaddr_in serv_mesage;
            int cy = int(sizeof(serv_mesage));
            byte = recvfrom(client, buffer, 200, 0, (struct sockaddr*)&serv_mesage, &cy);

            // XOR the buffer after receiving
            xorData(buffer, byte);

            cout << buffer << endl;
            }
        }
    }

    closesocket(client);
    WSACleanup();
}
