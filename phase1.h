#ifndef PHASE1
#define PHASE1

/* library included */
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<arpa/inet.h>
#include"struct.h"

/* defien constant */
#define MESSAGE_SIZE_FLAG 2
#define TIME_FORMAT "%FT%T%z"
#define MAX_TIME_SIZE 30
#define HEADER_SIZE 12
#define AAAA_QTYPE 28
#define DOT_SIZE 1
#define BYTE_SIZE 8
#define STANDARD_SECTION_SIZE 2

/* function prototype */
unsigned char* read_dns_message(unsigned int);
char* doamin_name_extraction(unsigned char *, unsigned char **);
Question_t* parse_request(unsigned char *);
Answer_t* parse_response(unsigned char *);
void generate_format_time(char *);
void write_request_log(Question_t *, FILE *);
void write_response_log(Answer_t *, FILE *);

#endif
