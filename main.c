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
  cya_parse_result *result;
  int8_t data_length = 0, user_choice;

  for (int i = 1; i < argc; i++) {
    query = realloc(query, strlen(query) + strlen(argv[i]));
    strcat(query, " ");
    strcat(query, argv[i]);
  }

  buffer = cya_search(query);
  free(query);

  result = cya_parse(buffer);
  free(buffer);

  for (int8_t i = result->length - 1; i >= 0; i--) {
    printf("%d: %s\n", i, result->data[i]->title);
  }

  printf("Your choice: ");
  while (scanf("%hhu", &user_choice) != 1 || user_choice >= result->length ||
         user_choice < 0) {
    printf("Invalid choice. Please enter a number between 0 and %d.\n",
           result->length - 1);
    while (getchar() != '\n')
      ;
    printf("Your choice: ");
  }

  cya_play(result->data[user_choice]);

  free(result);
  return EXIT_SUCCESS;
}
