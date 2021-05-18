#ifndef STRUCT
#define STRUCT

/* library included */
#include<stdlib.h>
#include<assert.h>

/* data structure to store information */
typedef struct Question{
  char *domain_name;
  int qtype;
  int qclass;
}Question_t;

typedef struct Answer{
  char *domain_name;
  int ancount;
  int type;
  int class;
  int ttl;
  int rdlength;
  char *rdata;
}Answer_t;

/* function prototype */
void free_question_t(Question_t *);
void free_answer_t(Answer_t *);
Question_t* create_question_info();
Answer_t* create_answer_info();

#endif
