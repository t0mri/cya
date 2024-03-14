#ifndef CYA_H
#define CYA_H

#include <stdint.h>
typedef struct {
  char *title, *channel, *video_id;
} cya_parse_data;

typedef struct {
  cya_parse_data **data;
  int8_t length;
} cya_parse_result;

char *cya_search(char *query);
cya_parse_result *cya_parse(char *buffer);
void cya_play(cya_parse_data *data);

#endif // !CYA_H
