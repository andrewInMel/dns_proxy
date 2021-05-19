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
void response_to_client(unsigned char *, Question_t *, int, FILE *, char **, int);

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
  int client_size, new_socket, query_len;
  struct sockaddr_in client;
  unsigned char *query_msg;
  Question_t *question_info;
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
    /* read, store & parse query message, then log query entry */
    query_msg = read_dns_message(new_socket, &query_len);
    question_info = parse_request(query_msg);
    write_request_log(question_info, log_file);
    /* reponse to query */
    response_to_client(query_msg, question_info, new_socket,
                       log_file, argv, query_len);
    /* close the socket then free query related memory */
    close(new_socket);
    free_question_t(question_info);
    free(query_msg);
  }
}

void
response_to_client(unsigned char *query_msg, Question_t *question_info,
                   int new_socket, FILE *log_file, char **argv, int query_len){
  int response_len;
  Answer_t *answer_info;
  unsigned char *to_client_message, *response_msg;
  if(question_info->qtype != AAAA_QTYPE){
    /* non AAAA query, sent back to client with RCODE 4, then log the entry */
    to_client_message = error_handler(query_msg);
    if(write(new_socket, to_client_message, query_len) < 0){
      perror("fail to write message to client");
      exit(EXIT_FAILURE);
    }
  }else{
    /* forward query to upstream server, and return the reponse message.
     * then, parse the response and write log entries based on <answer_info>.
     */
    response_msg = query_upstream_svr(query_msg, query_len, argv, &response_len);
    answer_info = parse_response(response_msg);
    write_response_log(answer_info, log_file);
    /* sent the response back to client. */
    if(write(new_socket, response_msg, response_len) < 0){
      perror("fail to write message to client");
      exit(EXIT_FAILURE);
    }
    /* free reponse related memory */
    free_answer_t(answer_info);
    free(response_msg);
  }
}

unsigned char*
query_upstream_svr(unsigned char *query_msg, int query_len,
                   char** argv, int *response_len){
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
  flag = write(client_sock, query_msg, query_len);
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
error_handler(unsigned char *query_msg) {
  unsigned char* track_ptr;
  /* move the track pointer to the flag section, change the <QR> value to 1 */
  track_ptr = query_msg + MESSAGE_SIZE_FLAG + STANDARD_SECTION_SIZE;
  track_ptr[0] = track_ptr[0] | 128;
  /* move to the byte where <RA> & <RCODE> resides,
   * change <RCODE> value to 4, AND <RA> value to 1
   */
  track_ptr++;
  track_ptr[0] = (((track_ptr[0] >> 4) << 4) | 4) | 128;
  /* return modified message addfress */
  return query_msg;
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
