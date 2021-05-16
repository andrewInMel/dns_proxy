#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include<string.h>
#include<time.h>

/* defien constant */
#define MESSAGE_SIZE_FLAG 2
#define TIME_FORMAT "%FT%T%z"
#define MAX_TIME_SIZE 30
#define HEADER_SIZE 12
#define QTYPE 28
#define DOT_SIZE 1

unsigned char* read_dns_message(unsigned int);
int parse_request(unsigned char *, char **);
char* generate_format_time(char *);
void write_request_log(int, char *, FILE *);
char* doamin_name_extraction(unsigned char **);

int
main(int argc, char *argv[]){
  unsigned char *dns_message;
  char *domain_name;
  int valid = 123;
  FILE *log_file;
  /* create a log file for read and write */
  log_file = fopen("dns_svr.log", "w+");
  /* phase1: read from stdin */
  dns_message = read_dns_message(STDIN_FILENO);
  /* phase DNS message */
  if(strcmp(argv[1], "request") == 0 ){
    valid = parse_request(dns_message, &domain_name);
    write_request_log(valid, domain_name, log_file);
  }
  // }else if(strcmp(argv[1], "response") == 0){
  //
  // }

  /* close the log file */
  fclose(log_file);
  /* free the memory */
  free(dns_message);
  free(domain_name);
  dns_message = NULL;
  domain_name = NULL;
  return 0;
}

/* read DNS message from socket */
unsigned char*
read_dns_message(unsigned int fd){
  unsigned char *dns_message;
  int message_size = 0;
  /* read message size flag */
  dns_message = (unsigned char*)malloc(sizeof(unsigned char) * MESSAGE_SIZE_FLAG);
  assert(dns_message);
  if(read(fd, dns_message, MESSAGE_SIZE_FLAG) == -1){
    printf("fail to message size flag from socket");
    exit(EXIT_FAILURE);
  }
  /* convert binary data of message size flag to decimal value */
  message_size = ((dns_message[0] << 8) | dns_message[1]);
  /* read the rest of dns message */
  dns_message = (unsigned char*)realloc(dns_message,
                 sizeof(unsigned char) * (message_size + MESSAGE_SIZE_FLAG));
  assert(dns_message);
  if(read(fd, dns_message + MESSAGE_SIZE_FLAG, message_size) == -1){
    printf("fail to dns message from socket");
    exit(EXIT_FAILURE);
  }
  return dns_message;
}

/* parse DNS request message */
int
parse_request(unsigned char *dns_message, char **domain_name){
  unsigned char *question_ptr;
  unsigned int section_size = 0;
  int a = 0;
  /* skip header and points to the beginning of question part */
  question_ptr = dns_message + HEADER_SIZE + MESSAGE_SIZE_FLAG;
  /* extract domain name */
  *domain_name = doamin_name_extraction(&question_ptr);
  /* move pointer to QTYPE segment & verify if it is a IPv6 request */
  question_ptr += 1;
  if(a = ((question_ptr[0] << 8) | question_ptr[1]) != QTYPE){
    return 1;
  }else{
    return 0;
  }
}

/* extract doamin name */
char*
doamin_name_extraction(unsigned char **question_ptr){
  char *domain_name = NULL;
  unsigned int index = 0, section_length = 0;
  /* loop though the QNAME segement to extract damain name */
  while(*question_ptr[0] != 0){
    /* once finished read one section of domain name,
     * get the size of next section.
     */
    if(section_length == 0 ){
      section_length = *question_ptr[0];
      /* realloc enough memory to store characters and dot between sections */
      domain_name = (char *)realloc(domain_name,
                              sizeof(char)*(section_length + index + DOT_SIZE));
      assert(domain_name);
      /* move to next byte */
      *question_ptr += 1;
      /* add a dot charater between two sections */
      if(index != 0){
        domain_name[index] = '.';
        index++;
      }
    }

    /* assemble domain name */
    domain_name[index] = *question_ptr[0];
    *question_ptr += 1;
    section_length--;
    index++;
  }
  /* allocate memory for terminate null byte, make <domain_name> a string */
  domain_name = (char *)realloc(domain_name, sizeof(char)*(index + 1));
  assert(domain_name);
  domain_name[index] = '\0';
  /* return domain name */
  return domain_name;
}

/* write to log file */
void
write_request_log(int valid, char* domain_name, FILE *log_file){
  char formatted_time[MAX_TIME_SIZE];
  /* write log input at current time */
  generate_format_time(formatted_time);
  fprintf(log_file, "%s requested %s\n", formatted_time, domain_name);
  fflush(log_file);
  /* if the request if not a type <AAAA> request, output following log */
//  printf("\n\n%d\n\n", valid);
  if(valid){
    fprintf(log_file, "%s unimplemented request\n", formatted_time);
    fflush(log_file);
  }
}

/* generate format time */
char*
generate_format_time(char *formatted_time){
  time_t current;
  struct tm *tm_time;
  /* initialise broken-down time struct variable based on the current time*/
  time(&current);
  tm_time = localtime(&current);
  /* generet required format of time */
  strftime(formatted_time,MAX_TIME_SIZE,TIME_FORMAT, tm_time);
}
