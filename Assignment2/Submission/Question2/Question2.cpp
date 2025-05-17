#include <bits/stdc++.h>
#include <cstring>   // For memset and memcpy

#ifndef SOCKET_INCLUDES_H

    #ifdef _WIN32
        // Windows-specific headers
        #include <winsock2.h>
        #include <ws2tcpip.h>
        #include <windows.h>

        // Link with Ws2_32.lib
        #pragma comment(lib, "Ws2_32.lib")

    #elif defined(__linux__)
        // Linux-specific headers
        #include <sys/types.h>
        #include <sys/socket.h>
        #include <netinet/in.h>
        #include <arpa/inet.h>
        #include <unistd.h>
        #include <netdb.h>

    #else
        #error "Platform not supported"

    #endif


#endif // SOCKET_INCLUDES_H

using namespace std;

//parsing string to get particualr auth tld DNSserver
vector<string> parseString(const string& input, char delimiter) {

    vector<string> result;
    stringstream ss(input);
    string item;
    while (getline(ss, item, delimiter)) {
        result.push_back(item);
    }

    return result;
}

//creating of LRU cache maintainance also implemented using list and unordered map
class LRUCache {
    private:
        int capacity; // needs capacity to create a list
        list<pair<string, string>> cacheList;
        unordered_map<string, list<pair<string, string>>::iterator> cacheMap;

        string fetchPage(const string &hostname, const string &path) { // brings data with get request
            WSADATA wsaData;
            SOCKET sock = INVALID_SOCKET;

            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
                cerr << "Startup failed\n";
                return "";
            }

            sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

            if(sock == INVALID_SOCKET){
                cerr << "Socket creation failed\n";
                WSACleanup();
                return "";
            }

            struct addrinfo *result = NULL;
            struct addrinfo hints = {0};
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;

            if (getaddrinfo(hostname.c_str(), "80", &hints, &result) != 0) {
                cerr << "Again failed\n";
                closesocket(sock);
                WSACleanup();
                return "";
            }


            
            struct sockaddr_in server;
            char buffer[4096];
            int bytesReceived;
            string response;

            server.sin_family = AF_INET;
            server.sin_port = htons(80);
            server.sin_addr.s_addr = ((struct sockaddr_in *)(result->ai_addr))->sin_addr.s_addr;

            freeaddrinfo(result);

            if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
                cerr << "Could not secure connection failed\n";
                closesocket(sock);
                WSACleanup();
                return "";
            }

            string request = "GET " + path + " HTTP/1.1\r\n"
                                "Host: " + hostname + "\r\n"
                                "Connection: close\r\n\r\n";
            send(sock, request.c_str(), request.size(), 0);

            while ((bytesReceived = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
                response.append(buffer, bytesReceived);
            }

            closesocket(sock);
            WSACleanup();

            int headerEndPos = response.find("\r\n\r\n");
            if(headerEndPos != string::npos){
                return response.substr(headerEndPos + 4);
            } else {
                return response;
            }
        }

        void putPage(const string &key, const string &pageContent) {
            if (cacheMap.find(key) == cacheMap.end()) {
                if (cacheList.size() == capacity) {
                    auto last = cacheList.back();
                    cacheMap.erase(last.first);
                    cacheList.pop_back();
                }
                cacheList.push_front({key, pageContent});
                cacheMap[key] = cacheList.begin();
            } else {
                cacheList.splice(cacheList.begin(), cacheList, cacheMap[key]);
                cacheMap[key]->second = pageContent;
            }
        }

    public:
        LRUCache(int capacity){
            this -> capacity = capacity;
        }

        string getPage(const string &hostname, const string &path) {
            string key = hostname + path;
            if(cacheMap.find(key) != cacheMap.end()){
                cacheList.splice(cacheList.begin(), cacheList, cacheMap[key]);
                return cacheMap[key]->second;
            }
            else{
                cout<<"NOT AVAILABLE IN CACHE HAS TO FETCH FROM SERVER "<<endl;
                string pageContent = fetchPage(hostname, path);
                putPage(key, pageContent);
                return pageContent;
            }
        }

};



int waitAndReturn() {
    int tot_wait = 0;

    for (int attempt = 0; attempt < 3; attempt++){
        int prob = rand() % 3;
        int waitTime = (prob == 0) ? 1 : (prob == 1) ? 3 : 4;
        int result= (rand() % 100 < 85) ? 1 : -1;
        //all decisions are taken beforehand because it is difficult to wait in one thread and delay in other thread

        if (waitTime > 2 || result == -1) {
           this_thread::sleep_for(chrono::seconds(2));
           // if wait is greater than 2 secs system waits for 2 secs and then breaks so does it again
           tot_wait+=2;
           continue;
        }

        this_thread::sleep_for(chrono::seconds(waitTime));
        tot_wait+=waitTime;
        
        if (result == 1) {
            cout << "Waited for " << tot_wait << " seconds, result: " << result << endl;
            return result;
        } else {
            cout << "Received -1, retrying..." << endl;
        }
    }

    cout << "failed" << endl;
    return -1;

}

// ___________________________________________________

//LRU cache for local DNS SERVER everything is same except the the pair data structure
class LRUCachedns {

    private:
        int capacity;
        list<string> access_list;
        unordered_map<string, pair<string, list<string>::iterator>> cache;

    public:
        LRUCachedns(int capacity){
            this -> capacity = capacity;
        }

        string get(const string& domain) {
            auto it = cache.find(domain);
            if (it == cache.end()) {
                return "";
            } else {
                access_list.splice(access_list.begin(), access_list, it->second.second);
                return it->second.first;
            }
        }

        void put(const string& domain,  string& ip_address) {
            auto it = cache.find(domain);
            if (it != cache.end()) {
                it->second.first = ip_address;
                access_list.splice(access_list.begin(), access_list, it->second.second);
            } else {
                if (cache.size() == capacity) {
                    cache.erase(access_list.back());
                    access_list.pop_back();
                }
                access_list.push_front(domain);
                cache[domain] = {ip_address, access_list.begin()};
            }
        }
        
};

// _____________________________

map<string,string>root;

map<string,string>com_tld;
map<string,string>org_tld;

map<string,string>google;
map<string,string>example;
map<string,string>example_org;
map<string,string>new_domain;

map<string, map<string,string>> servers;

// _____________________________________________________________

void init_servers(){
    root["com"]="1.1.1.1";
    root["org"]="1.1.1.2";

    com_tld["google"]="1.1.1.3";
    com_tld["new_domain"]="1.1.1.4";
    com_tld["example"]="1.1.1.5";
    org_tld["example_org"]="1.1.1.6";

    google["www.google.com"]="192.12.24.78";
    example["www.example.com"]="162.145.19.80";
    new_domain["www.new_domain.com"]="93.48.70.98";
    example_org["www.example_org.org"]="69.69.6.9";


    servers["1.1.1.0"]=root;

    servers["1.1.1.1"]=com_tld;
    servers["1.1.1.2"]=org_tld;
    
    servers["1.1.1.3"]=google;
    servers["1.1.1.4"]=new_domain;
    servers["1.1.1.5"]=example;
    servers["1.1.1.6"]=example_org;
}

// ______________________________________________________________________________________

string dns_lookup(const string& domain, LRUCachedns& cache){

    string ip_address = cache.get(domain);
    
    if (!ip_address.empty()) {
        cout << "Cache hit: " << ip_address << endl;
        return ip_address;
    }
    
    auto it = parseString(domain , '.');

    if(it.size()!=3){
        cout<<"DNS look_up :: failed host not found"<<endl;
        return "";
    }
    else{
        int succ_fail=waitAndReturn();

        if(succ_fail == 1){
            if(root.find(it[2]) == root.end()){
                cout<<"DNS look_up :: failed host not found"<<endl;
                return "";
            }

            auto sec_map = servers[root[it[2]]];
            if(sec_map.find(it[1]) == sec_map.end()){
                cout<<"DNS look_up :: failed host not found"<<endl;
                return "";
            }

            auto third_map = servers[sec_map[it[1]]];
            if(third_map.find(domain) == third_map.end()){
                cout<<"DNS look_up :: failed host not found"<<endl;
                return "";
            }

            string ans_ip=third_map[domain];
            cout << "ip address is found to be " << ans_ip << endl;

            cache.put(domain, ans_ip);
            
            return ans_ip;
        }
        else {
            cout<<"DNS server not responding :: request failed"<<endl;
            return "";
        }

    }
}


int main() {

    LRUCachedns only_cache(3);
    
    init_servers();
    
    string ip_addr = dns_lookup( "www.google.com", only_cache );
    string ip2_addr = dns_lookup( "www.google.com", only_cache );
    string ip3_addr = dns_lookup( "www.google.com", only_cache );
    
    cout << "First part done now to see how the http proxy server runs type 1" << endl; // first part for dns resolve
    
    int y;
    cin >> y;

    if(y != 1){
        return 0;
    }

    // ___________________________________________________________________________

    LRUCache cache(5); //for printing the webpages
    
    string page1 = cache.getPage("example.com", "/");
    cout << "Page 1 content:\n" << page1 << endl;

    string page2 = cache.getPage("example.com", "/about");
    cout << "Page 2 content:\n" << page2 << endl;

    string page3 = cache.getPage("example.com", "/contact");
    cout << "Page 3 content:\n" << page3 << endl;

    string page4 = cache.getPage("example.com", "/services");
    cout << "Page 4 content:\n" << page4 << endl;

    string page5 = cache.getPage("example.com", "/blog");
    cout << "Page 5 content:\n" << page5 << endl;

    string page1_again = cache.getPage("example.com", "/");
    cout << "Page 1 again content:\n" << page1_again << endl;
    
    string page6 = cache.getPage("example.com", "/newpage");
    cout << "Page 6 content:\n" << page6 << endl;
    
    return 0;

}
