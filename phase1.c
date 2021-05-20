#include"phase1.h"

/* parse DNS request message */
Question_t*
parse_request(unsigned char *dns_message){
  Question_t *question_info = NULL;
  unsigned char *request_ptr = NULL;
  /* initialise <question_info> */
  question_info = create_question_info();
  /* extract domain name */
  question_info->domain_name = doamin_name_extraction(dns_message, &request_ptr);
  /* move the tracking pointer to QTYPE section and extract QTYPE */
  request_ptr++;
  question_info->qtype = (request_ptr[0] << BYTE_SIZE) | request_ptr[1];
  /* move the tracking pointer to QTYPE section and extract QTYPE */
  request_ptr += STANDARD_SECTION_SIZE;
  question_info->qclass = (request_ptr[0] << BYTE_SIZE) | request_ptr[1];
  /* return extracted infomation */
  return question_info;
}

/* parse DNS response message */
Answer_t*
parse_response(unsigned char *dns_message){
  Answer_t *answer_info = NULL;
  unsigned char *response_ptr = NULL;
  /* initialise <answer_info> */
  answer_info = create_answer_info();
  /* move track pointer to ANCOUNT section, then extract ancount flag value */
  response_ptr = dns_message + MESSAGE_SIZE_FLAG + STANDARD_SECTION_SIZE * 3;
  answer_info->ancount = (response_ptr[0] << BYTE_SIZE) | response_ptr[1];
  /* only parse the response message if there is an answer in it */
  if(answer_info->ancount > 0){
    /* extract domain name */
    answer_info->domain_name = doamin_name_extraction(dns_message, &response_ptr);
    /* move <response_ptr> to answer segment */
    response_ptr += STANDARD_SECTION_SIZE * 2 + 1;
    /* move to TYPE section & extract TYPE value */
    response_ptr += STANDARD_SECTION_SIZE;
    answer_info->type = (response_ptr[0] << BYTE_SIZE) | response_ptr[1];
    if(answer_info->type == AAAA_QTYPE){
      /* move to CLASS section & extract CLASS value */
      response_ptr += STANDARD_SECTION_SIZE;
      answer_info->class = (response_ptr[0] << BYTE_SIZE) | response_ptr[1];
      /* move to TTL section & extract TTL value */
      response_ptr += STANDARD_SECTION_SIZE;
      answer_info->ttl = (response_ptr[0] << BYTE_SIZE * 3)
                          | (response_ptr[1] << BYTE_SIZE * 2)
                          | (response_ptr[2] << BYTE_SIZE)
                          | response_ptr[3];
      /* move to RDLENGTH section & extract RDLENGTH value */
      response_ptr += STANDARD_SECTION_SIZE * 2;
      answer_info->rdlength = (response_ptr[0] << BYTE_SIZE) | response_ptr[1];
      /* move to IP address section & extract IP address */
      response_ptr += STANDARD_SECTION_SIZE;
      answer_info->rdata = (char *)malloc(sizeof(char) * INET6_ADDRSTRLEN);
      inet_ntop(AF_INET6, response_ptr, answer_info->rdata, INET6_ADDRSTRLEN);
    }
  }
  /* return extracted information */
  return answer_info;
}

/* read DNS message from socket */
unsigned char*
read_dns_message(int fd, int *message_len){
  unsigned char *dns_message;
  int left_size = 0, read_size = 0;
  int i = 0, total_read = 0;
  /* allocate memory & read message size flag */
  dns_message = (unsigned char*)malloc(sizeof(unsigned char) * MESSAGE_SIZE_FLAG);
  assert(dns_message);
  for(i = MESSAGE_SIZE_FLAG; i > 0; i-= read_size){
    if((read_size = read(fd, dns_message + read_size, i) == -1){
      perror("fail to read message size flag from socket");
      exit(EXIT_FAILURE);
    }
  }
  /* convert binary data of message size flag to decimal value */
  left_size = ((dns_message[0] << BYTE_SIZE) | dns_message[1]);
  *message_len = left_size + MESSAGE_SIZE_FLAG;
  /* allocate enough memory, then read the rest of dns message */
  dns_message = (unsigned char*)realloc(dns_message, sizeof(unsigned char)
                                                     * (*message_len));
  assert(dns_message);
  /* restore read size before while loop */
  read_size = 0;
  while(left_size > 0){
    if((read_size = read(fd, dns_message + MESSAGE_SIZE_FLAG
                                         + total_read, left_size)) == -1){
      perror("fail to read dns message from socket");
      exit(EXIT_FAILURE);
    }
    left_size -= read_size;
    total_read += read_size;
  }
  return dns_message;
}

/* extract doamin name from a well-formed DNS message */
char*
doamin_name_extraction(unsigned char *dns_message, unsigned char **track_ptr){
  char *domain_name = NULL;
  unsigned int index = 0, section_length = 0;
  /* skip header and points to the beginning of question part */
  *track_ptr = dns_message + MESSAGE_SIZE_FLAG + HEADER_SIZE;
  /* loop though the QNAME segement to extract damain name */
  while(*track_ptr[0] != 0){
    /* once finished read one section of domain name,
     * get the size of next section.
     */
    if(section_length == 0 ){
      section_length = *track_ptr[0];
      /* realloc enough memory to store characters and dot between sections */
      domain_name = (char *)realloc(domain_name,
                              sizeof(char)*(section_length + index + DOT_SIZE));
      assert(domain_name);
      /* move to next byte */
      (*track_ptr)++;
      /* add a dot charater between two sections */
      if(index != 0){
        domain_name[index] = '.';
        index++;
      }
    }
    /* assemble domain name */
    domain_name[index] = *track_ptr[0];
    (*track_ptr)++;
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

/* write request infomation to log file */
void
write_request_log(Question_t* question_info, FILE *log_file){
  char formatted_time[MAX_TIME_SIZE];
  /* write log input at current time */
  generate_format_time(formatted_time);
  fprintf(log_file, "%s requested %s\n", formatted_time,
                                         question_info->domain_name);
  fflush(log_file);
  /* if the request if not a type <AAAA> request, output following log */
  if(question_info->qtype != AAAA_QTYPE){
    fprintf(log_file, "%s unimplemented request\n", formatted_time);
    fflush(log_file);
  }
}

/* write response infomation to log file */
void
write_response_log(Answer_t* answer_info, FILE *log_file){
  char formatted_time[MAX_TIME_SIZE];
  /* write log input at current time */
  generate_format_time(formatted_time);
  if(answer_info->ancount > 0 && answer_info->type == AAAA_QTYPE){
    fprintf(log_file, "%s %s is at %s\n", formatted_time,
            answer_info->domain_name, answer_info->rdata);
    fflush(log_file);
  }
}

/* generate format time */
void
generate_format_time(char *formatted_time){
  time_t current;
  struct tm *tm_time;
  /* initialise broken-down time struct variable based on the current time*/
  time(&current);
  tm_time = localtime(&current);
  /* generet required format of time */
  strftime(formatted_time,MAX_TIME_SIZE,TIME_FORMAT, tm_time);
}
