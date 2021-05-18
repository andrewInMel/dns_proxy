#include"phase1.h"
#include <netdb.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>

#define PORT_LISTEN_ON 8053
#define QUEUE_SIZE 5

/* Function prototype */
int create_server_socket(const int);
void live_dns_svr(FILE *, char**);
unsigned char* error_handler(unsigned char *);
unsigned char* query_upstream_svr(unsigned char *, int, char **, int *);

int
main(int argc, char *argv[]){
  FILE *log_file;
  /* create a log file for read and write log entries */
  log_file = fopen("dns_svr.log", "w+");
  /* running a live DNS server */
  live_dns_svr(log_file, argv);
  /* close the log file if the server goes down */
  fclose(log_file);
  /* return */
  return 0;
}

/* live DNS server */
void
live_dns_svr(FILE *log_file, char **argv){
  int client_size, new_socket, query_len, response_len;
  struct sockaddr_in client;
  unsigned char *quey_message, *response_message, *to_client_message;
  Question_t *question_info;
  Answer_t *answer_info;
  /* create a socket, bind to port 8053, & start listen */
  int socket_svr;
  socket_svr = create_server_socket(PORT_LISTEN_ON);
  if(listen(socket_svr, QUEUE_SIZE) < 0){
		perror("listen");
		exit(EXIT_FAILURE);
	};
  /* Live server: running the server within an infinite loop */
  client_size = sizeof(client);
  while(1){
    /* accept connection */
    if((new_socket = accept(socket_svr, (struct sockaddr *)&client,
                            (socklen_t *)&client_size)) < 0){
      perror("accept");
			exit(EXIT_FAILURE);
    }
    /* read, store & parse query message, then log request entry */
    quey_message = read_dns_message(new_socket, &query_len);
    question_info = parse_request(quey_message);
    write_request_log(question_info, log_file);
    /* reponse to query base on the infomation within <question_info> */
    if(question_info->qtype != AAAA_QTYPE){
      /* non AAAA query, sent back to client with RCODE 4, then log the entry */
      to_client_message = error_handler(quey_message);
      if(write(new_socket, to_client_message, query_len) < 0){
        perror("fail to write message to client");
        exit(EXIT_FAILURE);
      }
    }else{
      /* forward query to upstream server, and return the reponse message.
       * then, parse the response and write log entries based on <answer_info>.
       */
      response_message = query_upstream_svr(quey_message, query_len, argv, &response_len);
      answer_info = parse_response(response_message);
      write_response_log(answer_info, log_file);
      /* sent the response back to client. */
      if(write(new_socket, response_message, response_len) < 0){
        perror("fail to write message to client");
        exit(EXIT_FAILURE);
      }
    }
  }
}

unsigned char*
query_upstream_svr(unsigned char *query_message, int query_len, char** argv, int *response_len){
  int client_sock, port, flag;
  struct sockaddr_in server;
  unsigned char *response;
  /* create an internet socket */
  client_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (client_sock < 0) {
    perror("fail to create a socket");
    exit(EXIT_FAILURE);
  }
  /* initialise server address */
  memset(&server, 0, sizeof(server));
  /* fill in the server address structure */
  port = atoi(argv[2]);
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(argv[1]);
  server.sin_port = htons(port);
  /* connect to the server */
  flag = connect(client_sock, (struct sockaddr *)&server , sizeof(server));
  if(flag < 0){
    perror("fail to connect server");
    exit(EXIT_FAILURE);
  }
  /* write query into the socket */
  flag = write(client_sock, query_message, query_len);
  if(flag < 0){
    perror("fail to write message to socket");
    exit(EXIT_FAILURE);
  }
  /* read server response */
  response = read_dns_message(client_sock, response_len);
  /* close the socket & return */
  close(client_sock);
  return response;
}

unsigned char *
error_handler(unsigned char *quey_message) {
  unsigned char* track_ptr;
  /* move the track pointer to the flag section, change the <QR> value to 1 */
  track_ptr = quey_message + MESSAGE_SIZE_FLAG + STANDARD_SECTION_SIZE;
  track_ptr[0] = track_ptr[0] | 128;
  /* move to the byte where <RCODE> resides, change <RCODE> value to 4 */
  track_ptr++;
  track_ptr[0] = ((track_ptr[0] >> 4) << 4) | 4;
  /* return modified message addfress */
  return quey_message;
}

/*********************************************************
 * This function is from practice 9 sulotion: server-1.3 *
 * Reference: Beej's networking guide, man pages         *
 *********************************************************/
/* Create and return a socket bound to the given port */
int
create_server_socket(const int port) {
	int sockfd;
	struct sockaddr_in serv_addr;
	/* Create socket */
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("fail to create a socket");
		exit(EXIT_FAILURE);
	}
	/* Create listen address for given port number (in network byte order)
	for all IP addresses of this machine */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	/* Reuse port if possible */
	int re = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(int)) < 0) {
		perror("Could not reopen socket");
		exit(EXIT_FAILURE);
	}
	/* Bind address to socket */
	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
	return sockfd;
}


/* debug printf code */
//    printf("%s --- %d --- %d\n", question_info->domain_name, question_info->qtype, question_info->qclass);

// for(int i = 0 ; i < 54; i += 2){
//   printf("%02x %02x ", quey_message[i], quey_message[i+1]);
// }
// printf("\n");
