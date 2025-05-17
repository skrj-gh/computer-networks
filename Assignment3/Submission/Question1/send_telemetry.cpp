#include <bits/stdc++.h>
#include <chrono>
#include <fstream>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <ctime>
#pragma comment(lib, "ws2_32.lib")
#define ll long long
using namespace std;

class triplet {
public:
    int x;
    int y;
    int z;
};

struct dronestate {
    int speed;
    triplet pos;
    int state;
};

char myArray[200];

// XOR function for a given buffer
void xorData(char* data, int len) {
    for (int i = 0; i < len; i++) {
        data[i] ^= myArray[i % 200];
    }
}

// Function to serialize the dronestate structure
void serialize_dronestate(const dronestate& state, char* buffer) {
    memcpy(buffer, &state.speed, sizeof(state.speed));
    memcpy(buffer + sizeof(state.speed), &state.pos.x, sizeof(state.pos.x));
    memcpy(buffer + sizeof(state.speed) + sizeof(state.pos.x), &state.pos.y, sizeof(state.pos.y));
    memcpy(buffer + sizeof(state.speed) + sizeof(state.pos.x) + sizeof(state.pos.y), &state.pos.z, sizeof(state.pos.z));
    memcpy(buffer + sizeof(state.speed) + sizeof(state.pos.x) + sizeof(state.pos.y) + sizeof(state.pos.z), &state.state, sizeof(state.state));
}

dronestate gen_state() {
    triplet POS;
    POS.x = rand() % 100;
    POS.y = rand() % 100;
    POS.z = rand() % 100;
    int sped = 10 + rand() % 20;
    int stte = rand() % 2;
    struct dronestate curr;
    curr.pos = POS;
    curr.speed = sped;
    curr.state = stte;
    return curr;
}

int main() {
    // Fill the myArray with some data
    srand(static_cast<unsigned>(time(nullptr)));
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
        cout << "Failed to initialize Winsock: " << wsaerr << endl;
        return 0;
    } else {
        cout << "Winsock initialized successfully." << endl;
    }

    SOCKET client = INVALID_SOCKET;
    client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == INVALID_SOCKET) {
        cout << "Socket could not be created: " << WSAGetLastError() << endl;
        WSACleanup();
        return 0;
    } else {
        sockaddr_in client_struc;
        client_struc.sin_family = AF_INET;
        client_struc.sin_addr.s_addr = inet_addr("127.0.0.1");
        client_struc.sin_port = htons(55555);
        if (connect(client, (SOCKADDR*)&client_struc, sizeof(client_struc)) == SOCKET_ERROR) {
            cout << "Connection failed: " << WSAGetLastError() << endl;
            closesocket(client);
            WSACleanup();
            return 0;
        } else {
            cout << "Connected successfully." << endl;

            char buffer[sizeof(dronestate)]; // Buffer to hold serialized data

            // XOR the data before sending
            while (true) {
                struct dronestate current = gen_state();
                this_thread::sleep_for(chrono::seconds(3));

                // Serialize the dronestate structure
                serialize_dronestate(current, buffer);

                // XOR the serialized data
                xorData(buffer, sizeof(buffer));

                // Send the XORed data
                int bytes = send(client, buffer, sizeof(buffer), 0);
                if (bytes == SOCKET_ERROR) {
                    cout << "Failed to send data: " << WSAGetLastError() << endl;
                    break;
                } else {
                    cout << "Data sent successfully." << endl;
                }
            }
        }
    }

    closesocket(client);
    WSACleanup();
    return 0;
}
