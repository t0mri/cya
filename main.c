#include "libcya/cya.h"
#include <cjson/cJSON.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE_TOO_FEW_ARGUMENTS 10
#define EXIT_FAILURE_JSON_PARSE 20

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr,
            "Error: A query (aka keywords) is required to make a search\n");
    exit(EXIT_FAILURE_TOO_FEW_ARGUMENTS);
  }

  char *query = malloc(0), *buffer;
  cJSON *json;
  constructed_data **data;
  int8_t data_length = 0;

  for (int i = 1; i < argc; i++) {
    query = realloc(query, strlen(query) + strlen(argv[i]));
    strcat(query, " ");
    strcat(query, argv[i]);
  }

  buffer = cya_search(query);
  free(query);

  data = cya_parse(buffer);
  free(buffer);

  for (int i = 0; data[i]; i++) {
    printf("Title: %s\nChanel: %s\nVideo id: %s\n\n", data[i]->title,
           data[i]->channel, data[i]->video_id);
  }

  printf("PROCESS COMPLETED\n");

  // cJSON_Delete(json);
  // free(data);
  return EXIT_SUCCESS;
}
