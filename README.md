# dns_proxy

## About
This is a practice project that improves my understanding of network layers, TCP protocol, multi-thread, DNS, socket programming & basic C programming. 
It aims to write a miniature DNS proxy server. Client sends query to the proxy server, which is forwarded to upstream server. Then, reply the client with reponse that 
returned from upstream server.

## Features
* It is designed only to deal with AAAA (IPv6) type of Resource Record, any other query will be rejected. (see firgue 1 below)
* Error handling, it will check if the query and response is well-formed DNS message. (see firgue 2 below)
* Multi-thread, the proxy server is able to process query concurrently.
* Caching, the proxy server stores up to 10 most recent active responses from upstream server.
* log file stores essential the informations of all query & response
* The conenctions between these parties are TCP connection other than usually UDP

## Running Locally
### Prerequisite
* Ubuntu/Mingw/Powershell etc.
* compiler: GCC/Clang
* make & dig 

### Start Server (see firgue 3 below)
Please ignore Angle brackets when it is around command
1. Open the terminal, then navigate to the folder of source files
2. Enter command < make clean && make >  
(you need to change makefile if clang is used as the compiler front end)
3. Start the server by enter the command < ./dns_svr 8.8.8.8 53 >  
8.8.8.8 is google's public DNS server, and 53 is the port , it serves as the upstream server in the project

### Testing (see firgue 4 below)
Open another terminal, enter command < dig +tcp @127.0.0.1 -p 8053 AAAA {domain name} >.  
* Replace {domain name} with the domain you want to qury, such as google.com, facebook.com ect.
* @127.0.0.1 is the local machine that host the server, and 8053 is the port to the service
* AAAA specifies the type of RR requested
* +tcp specifies the connection type

If the domain has AAAA(IPv6) resource record it will return message with anwser, otherwise it will be no anwser in the message 

### Reference 
Firgue 1  
![firgue 1](/images/1.jpg)

Firgue 2  
![firgue 2](/images/2.jpeg)

Firgue 3  
![firgue 3](/images/3.jpg)

Firgue 4  
![firgue 4](/images/4.jpg)

