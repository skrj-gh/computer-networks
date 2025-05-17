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
        cout << "Failed to initialize Winsock." << endl;
    } else {
        cout << "Winsock initialized successfully." << endl;
    }

    SOCKET client = INVALID_SOCKET;
    client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client == INVALID_SOCKET) {
        cout << "Socket could not be created: " << WSAGetLastError() << endl;
        WSACleanup();
        return 0;
    } else {
        sockaddr_in client_struc;
        client_struc.sin_family = AF_INET;
        client_struc.sin_addr.s_addr = inet_addr("127.0.0.1");
        client_struc.sin_port = htons(55555);

        // Example binary data: an array of unsigned integers
        unsigned int binaryData[5] = {0x12345678, 0x9ABCDEF0, 0x55555555, 0xAAAAAAAA, 0xDEADBEEF};

        // XOR the binary data before sending
        xorData((char*)binaryData, sizeof(binaryData));

        while (true) {
            this_thread::sleep_for(chrono::seconds(3));
            int byte = sendto(client, (const char*)binaryData, sizeof(binaryData), 0, (struct sockaddr*)&client_struc, sizeof(client_struc));
            if (byte == -1) {
                cout << "Error: Could not send data." << endl;
                WSACleanup();
                return 0;
            }
        }
    }

    closesocket(client);
    WSACleanup();
    return 0;
}
