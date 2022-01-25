# dns_proxy

## About
This is a practice project that improves my understanding of network layers, TCP protocol, multi-thread, DNS, socket programming & basic C programming. 
It aims to write a miniature DNS proxy server. The client sends the query to the proxy server, which is forwarded to the upstream server. Then, reply to the client with the response that is returned from the upstream server.

## Features
* It is designed only to deal with AAAA (IPv6) type of Resource Record, any other query will be rejected. (see figure 1 below)
* Error handling, it will check if the query and response is a well-formed DNS message. (see figure 2 below)
* Multi-thread, the proxy server can process queries concurrently.
* Caching, the proxy server stores up to 10 most recent active responses from the upstream server.
* log file stores essential the information of all query & response.
* The connections between these parties are TCP connections other than usually UDP.
* Bitwise operation is used to manipulate the binary data.

## Running Locally
### Prerequisite
* Ubuntu/Mingw/Powershell etc.
* compiler: GCC/Clang
* make & dig 

### Start Server (see figure 3 below)
Please ignore Angle brackets when it is around a command
1. Open the terminal, then navigate to the folder of source files
2. Enter command < make clean && make >  
(you need to change makefile if clang is used as the compiler front end)
3. Start the server by enter the command < ./dns_svr 8.8.8.8 53 >  
8.8.8.8 is google's public DNS server, and 53 is the port, it serves as the upstream server in the project

### Testing (see figure 4 below)
Open another terminal, enter command < dig +tcp @127.0.0.1 -p 8053 AAAA {domain name} >.  
* Replace {domain name} with the domain you want to query, such as google.com, facebook.com etc.
* @127.0.0.1 is the local machine that host the server, and 8053 is the port to the service
* AAAA specifies the type of RR requested
* +tcp specifies the connection type

If the domain has AAAA(IPv6) resource record it will return a message with an answer. Otherwise, there will be no answer in the message 

### Reference 
Figure 1  
![figure 1](/images/1.jpg)

Figure 2  
![figure 2](/images/2.jpeg)

Figure 3  
![figure 3](/images/3.jpg)

Figure 4  
![figure 4](/images/4.jpg)

