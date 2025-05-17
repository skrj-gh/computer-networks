# Computer Networks Lab Assignments
These are the lab submissions done for CS342 Computer Networks Laboratory.

## Assignment1
- Perform various activities according to functionalities available in the assigned application and collect the traces for the application using Wireshark
  - Zoom
  - Live Sport Streaming
  - Youtube - uploading video
  - Youtube - downloading and buffering
  - NPTEL video lectures
  - Twitch (live streaming video platform) or Hotstar video streaming
  - MS-Teams
  - P2P connectivity using Remote desktop and softwares like Team Viewer
  - Online games
  - Dropbox
- List out all the protocols used by the application at different layers
- Highlight and explain the observed values for various fields of protocols
- Explain the sequence of messages exchanged by the application for using the available functionalities in the application
- Explain how the protocol(s) used by the application is relevant for its functioning
- Calculate the following statistics
  - Throughput
  - RTT
  - Packet size
  - Number of packets lost
  - Number of UDP & TCP packets
  - Number of responses received with respect to one request sent
- Use TCPDUMP with necessary filters for actual capture and wireshark for analysis

## Assignment2
- HTTP using Wireshark
  - Analyze HTTP Request
  - Inspect HTTP Headers
  - Calculate Response Time
  - Examine HTTP Content
- Web Server, HTTP Proxy with Caching, and DNS Lookup Simulation
  - Develop a cache data structure capable of storing a limited number of web pages
  - If the page is not in the cache, simulate the DNS lookup process, perform an HTTP GET request to fetch the page, store it in the cache, and return the content
  - Implement HTTP GET requests using sockets to communicate with the Web server
  - Use a linked list to keep track of the order in which pages are accessed
  - Implement cache eviction by removing the least recently used (LRU) page when the cache reaches its maximum capacity
- Develop an HTTP Proxy Server
  - Implement an HTTP Proxy server capable of handling HTTP GET requests from clients.
  - Cookie Parsing
  - Cookie Storage
  - Cookie Forwarding
  - Session Management
  - Utilize an appropriate data structure to efficiently manage cookies for each client session
  - Ensure the proxy server can handle multiple client sessions concurrently
  - Implement detailed logging to capture each request and response, including the cookies involved

## Assignment3
- A tri-mode communication system: Control Commands, Telemetry Data, File Transfer
  - Mode 1 (Control Commands): A high-performance mode where the server sends control commands to drones with minimal latency
  - Mode 2 (Telemetry Data): A reliable mode where drones send telemetry data to the server. Ensure data is delivered accurately and in order
  - Mode 3 (File Transfers): Efficiently transfer large files from drones to the server using the QUIC protocol
- A real-time weather monitoring system
  - Efficient Data Handling: Implement a mechanism to ensure that the server can receive and process weather data from multiple weather stations without being overwhelmed. The data transmission should be optimized to avoid overloading the server
  - Adaptive Data Transmission: Implement a congestion control algorithm, specifically *TCP Reno*, at the weather stations to adjust their data transmission rate based on network conditions
  - Simulated Network Constraints: Simulate a limited network environment where the server's capacity and available bandwidth are constrained
  - Dynamic Data Compression: Introduce a dynamic data compression feature

## Assignment4
- A network router switch fabric that handles high-throughput traffic: under Uniform, Non-uniform and Bursty traffic
  - WFQ Scheduling Algorithm
  - Round Robin Scheduling Algorithm
  - Priority Scheduling Algorithm
  - iSLIP Scheduling Algorithm

## Assignment5
- Implementing the Distance Vector Routing Algorithm
- Implementing the Poisoned Reverse Mechanism
- Implementing the Split Horizon Mechanism

## Assignment6
- Collision Management in Ethernet (CSMA/CD) and Wi-Fi (CSMA/CA) using NS3
  - RTS/CTS Experimentation: Repeat the entire experiment with the RTS/CTS mechanism enabled on the Wi-Fi devices
