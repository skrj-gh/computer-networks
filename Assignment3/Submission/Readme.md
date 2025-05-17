# Assignment 3

## All the codes mentioned are programmed for the Windows Operating System, so compile and run them on Windows only.


### Question 1
For question 1 To send telemetry data go to the path and then open two terminals 
In one terminal run 
```sh
       g++ .\serverr.cpp -o server -lws2_32
      .\server.exe  
```
In other run 
```sh
      g++ .\send_telemetry.cpp -o tel -lws2_32
      .\tel.exe
```
Whenever a new drone is created run 
```sh
      g++ .\client.cpp -o client -lws2_32
      .\client.exe
```
### Question 2
First download zlib and add to path.
After this to run the server 
```sh
 g++ server.cpp -I %path to zlib% -L %path to zlib% -lz -lws2_32 -o  server.exe
.\server.exe
```
To run the client
```sh
 g++ client.cpp -I %path to zlib% -L %path to zlib% -lz -lws2_32 -o  client.exe
.\client.exe
```
If already added to path 
After this to run the server 
```sh
 g++ server.cpp  -lz -lws2_32 -o  server.exe
.\server.exe
```
To run the client
```sh
 g++ client.cpp -lz -lws2_32 -o  client.exe
.\client.exe
```
might also work.