#include <bits/stdc++.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <fstream>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

class Ck {
    public:
        string n;
        string v;
        chrono::time_point<chrono::system_clock> exp;
};

void clearFile(const string& fileName) {
    ofstream file(fileName, ios::trunc);
    file.close();
}

class CkMgr {
    private:
        unordered_map<string, unordered_map<string, Ck>> ckMap;
        mutex mtx;

    public:
        vector<Ck> getCks(const string& sId) {
            lock_guard<mutex> lock(mtx);
            vector<Ck> cks;
            if (ckMap.find(sId) != ckMap.end()) {
                for (const auto& p : ckMap[sId]) {
                    cks.push_back(p.second);
                }
            }
            return cks;
        }

        void addCk(const string& sId, const Ck& ck){
            lock_guard<mutex> lock(mtx);
            ckMap[sId][ck.n] = ck;
        }

        void rmExpCks(const string& sId){
            lock_guard<mutex> lock(mtx);
            auto& cks = ckMap[sId];
            auto now = chrono::system_clock::now();
            for (auto it = cks.begin(); it != cks.end(); ) {
                if (it->second.exp < now) {
                    it = cks.erase(it);
                } else {
                    ++it;
                }
            }
        }
};


void logReqResp(const string& sId, const string& req, const string& resp){

    FILE* logFile = fopen("logfile.txt", "w");
    if(logFile != nullptr){
        fclose(logFile);
    }
    
    logFile = fopen("logfile.txt", "a");
    if(logFile != nullptr){
        fprintf(logFile, "ID: %s\n", sId.c_str());
        fprintf(logFile, "Req: %s\n", req.c_str());
        fprintf(logFile, "Resp: %s\n", resp.c_str());
        fprintf(logFile, "----\n");
        fclose(logFile);
    }
}


class Proxy {
    private:
        CkMgr ckMgr;

        string getCookies(const string& resp) {
            string ckStr;
            int pos = resp.find("Set-Cookie:");
            if (pos != string::npos) {
                int end = resp.find("\n", pos);
                ckStr = resp.substr(pos + 11, end - pos - 11);
            }
            return ckStr;
        }

        void saveCookies(const string& sId, const string& resp) {
            string ckStr = getCookies(resp);
            if (!ckStr.empty()) {
                int pos = ckStr.find("=");
                string ckName = ckStr.substr(0, pos);
                string ckVal = ckStr.substr(pos + 1, ckStr.find(";") - pos - 1);

                Ck ck = {ckName, ckVal, chrono::system_clock::now() + chrono::hours(24)};
                ckMgr.addCk(sId, ck);
            }
        }

        string appendCks(const string& sId, const string& req) {
            auto cks = ckMgr.getCks(sId);
            if (!cks.empty()) {
                stringstream ckHdr;
                ckHdr << "Cookie: ";
                for (const auto& ck : cks) {
                    ckHdr << ck.n << "=" << ck.v << "; ";
                }
                return req + ckHdr.str() + "\r\n";
            }
            return req;
        }


        string sendReq(const string& host, const string& req){

            struct addrinfo hints, *res, *p;
            SOCKET sock = INVALID_SOCKET;
            int resStatus;
            string resp;

            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;

            if ((resStatus = getaddrinfo(host.c_str(), "80", &hints, &res)) != 0) {
                cerr << "gai: " << gai_strerror(resStatus) << endl;
                return "";
            }

            for (p = res; p != NULL; p = p->ai_next) {
                sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
                if (sock == INVALID_SOCKET) {
                    cerr << "sock: " << WSAGetLastError() << endl;
                    continue;
                }

                if (connect(sock, p->ai_addr, p->ai_addrlen) == SOCKET_ERROR) {
                    cerr << "conn: " << WSAGetLastError() << endl;
                    closesocket(sock);
                    continue;
                }

                break;
            }

            if (p == NULL) {
                cerr << "No connect" << endl;
                freeaddrinfo(res);
                return "";
            }

            freeaddrinfo(res);

            if (send(sock, req.c_str(), req.size(), 0) == SOCKET_ERROR) {
                cerr << "send: " << WSAGetLastError() << endl;
                closesocket(sock);
                return "";
            }

            char buf[4096];
            int bytes;
            while ((bytes = recv(sock, buf, sizeof(buf), 0)) > 0) {
                resp.append(buf, bytes);
            }

            if (bytes == SOCKET_ERROR) {
                cerr << "recv: " << WSAGetLastError() << endl;
            } else if (bytes == 0) {
                cerr << "connection closed with succesfull saving of response." << endl;
            }

            closesocket(sock);
            return resp;
        }

    public:
        void handleClient(SOCKET cliSock) {
            char buf[4096];
            int bytes = recv(cliSock, buf, sizeof(buf), 0);

            if (bytes > 0) {
                string req(buf, bytes);
                string sId = std::to_string(cliSock);

                int hostStart = req.find("Host: ") + 6;
                int hostEnd = req.find("\r\n", hostStart);

                string host = req.substr(hostStart, hostEnd - hostStart);

                string resp = sendReq(host, req);

                send(cliSock, resp.c_str(), resp.size(), 0);

                logReqResp(sId, req, resp);
            }

            closesocket(cliSock);
        }

        void start(int port) {
            WSADATA wsaData;
            
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
                cerr << "WSA: failed" << endl;
                exit(EXIT_FAILURE);
            }

            SOCKET srvSock = socket(AF_INET, SOCK_STREAM, 0);
            if (srvSock == INVALID_SOCKET) {
                cerr << "Socket: failed" << endl;
                WSACleanup();
                exit(EXIT_FAILURE);
            }

            struct sockaddr_in srvAddr;
            srvAddr.sin_family = AF_INET;
            srvAddr.sin_addr.s_addr = INADDR_ANY;
            srvAddr.sin_port = htons(port);

            if (bind(srvSock, (struct sockaddr*)&srvAddr, sizeof(srvAddr)) == SOCKET_ERROR) {
                cerr << "Bind: failed" << endl;
                closesocket(srvSock);
                WSACleanup();
                exit(EXIT_FAILURE);
            }

            if (listen(srvSock, 10) == SOCKET_ERROR) {
                cerr << "Listen: failed" << endl;
                closesocket(srvSock);
                WSACleanup();
                exit(EXIT_FAILURE);
            }

            cout << "Proxy on " << port << endl;

            while(true){
                struct sockaddr_in cliAddr;
                int cliAddrLen = sizeof(cliAddr);
                SOCKET cliSock = accept(srvSock, (struct sockaddr*)&cliAddr, &cliAddrLen);
                if (cliSock == INVALID_SOCKET) {
                    cerr << "Accept: failed" << endl;
                    continue;
                }

                thread(&Proxy::handleClient, this, cliSock).detach();
            }

            closesocket(srvSock);
            WSACleanup();
        }
};

int main() {
    Proxy proxy;
    proxy.start(8080);

    return 0;
}
