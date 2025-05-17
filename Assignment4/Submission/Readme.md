# Assignment 4

### These codes are written for Windows OS, so run them on the Windows system only.

#### Also each code has Uniform Traffic running by default, in order to run for non-uniform and bursty traffic, comment the code for other two and uncomment the code for required traffic generation method. 

To run the "Priority Scheduling" Algorithm:
```terminal
g++ priority.cpp -o priority
./priority
```

To run the "Round Robing Scheduling" Algorithm:
```terminal
g++ round_robin.cpp -o round_robin
./round_robin
```

To run the "Weighted Fair Queuing Scheduling" Algorithm:
```terminal
g++ WFQ.cpp -o WFQ
./WFQ
```

To run the "iSLIP Scheduling" Algorithm:
```terminal
g++ islip.cpp -o islip
./islip
```