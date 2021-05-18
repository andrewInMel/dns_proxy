#include "struct.h"

/* free Question_t structure data */
void
free_question_t(Question_t *node){
  free(node->domain_name);
  node->domain_name = NULL;
  free(node);
  node = NULL;
}

/* free Answer_t structure data */
void
free_answer_t(Answer_t *node){
  free(node->domain_name);
  node->domain_name = NULL;
  free(node->rdata);
  node->rdata = NULL;
  free(node);
  node = NULL;
}

/* create & initialise Question_t struct */
Question_t*
create_question_info(){
  Question_t* question_info;
    /* initialise <question_info> */
  question_info = (Question_t *)malloc(sizeof(Question_t));
  assert(question_info);
  question_info->domain_name = NULL;
  question_info->qtype = 0;
  question_info->qclass = 0;
  /* return the struct */
  return question_info;
}

/* create & initialise Answer_t struct */
Answer_t*
create_answer_info(){
  Answer_t* answer_info;
  /* initialise <question_info> */
  answer_info = (Answer_t *)malloc(sizeof(Answer_t));
  assert(answer_info);
  answer_info->domain_name = NULL;
  answer_info->ancount = 0;
  answer_info->type = 0;
  answer_info->class = 0;
  answer_info->ttl = 0;
  answer_info->rdlength = 0;
  answer_info->rdata = NULL;
    /* return the struct */
  return answer_info;
}
