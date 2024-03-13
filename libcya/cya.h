#ifndef CYA_H
#define CYA_H

typedef struct {
  char *title, *channel, *video_id;
} constructed_data;

char *cya_search(char *query);
constructed_data **cya_parse(char *buffer);

#endif // !CYA_H
