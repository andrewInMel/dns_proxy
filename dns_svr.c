#include "phase1.h"

int
main(int argc, char *argv[]){
  unsigned char *dns_message;
  FILE *log_file;
  Question_t *question_info;
  Answer_t *answer_info;
  /* create a log file for read and write */
  log_file = fopen("dns_svr.log", "w+");
  /* read message from file descriptor*/
  dns_message = read_dns_message(STDIN_FILENO);
  /* extract infomation & write log entry, then free the stored infomations */
  if(strcmp(argv[1], "query") == 0){
    question_info = parse_request(dns_message);
    write_request_log(question_info, log_file);
    free_question_t(question_info);
  }else if(strcmp(argv[1], "response") == 0){
    answer_info = parse_response(dns_message);
    write_response_log(answer_info, log_file);
    free_answer_t(answer_info);
  }
  /* close the log file */
  fclose(log_file);
  /* free the memory */
  free(dns_message);
  dns_message = NULL;
  /* return */
  return 0;
}
