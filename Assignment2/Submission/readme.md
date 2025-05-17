# Assignment 2

## question no 2 :: simulating DNS look up and web cache 
We have used windows socket so To run the program  in a windows go to the program repo and and in terminal run 
```sh
  g++ -o output Question2.cpp -lws2_32
  ./output
```
If you are using codeBlocks or VS code or other IDE go to projects and then to build section and there in the add files in linker section add Ws2_32 if not already done.
It first prints the DNSlook up process and then asks you to input 1 to print the second part 
## Question no 3 :: HTTP proxy server 
```sh
  g++ -o output2 Question3.cpp -lws2_32
  ./output2
```
it may give binding failed if already some operation is being done in port 8080 localhost so take care of that or wait to run it again 
in other terminal run 
```sh
  curl -x http://localhost:8080 http://httpbin.org
```
and then wait it may take some time.
If there is some error in fetching file it will return empty response in command prompt.