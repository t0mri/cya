#include "libcya/cya.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE_TOO_FEW_ARGUMENTS 20

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr,
            "Error: A query (aka keywords) is required to make a search\n");
    exit(EXIT_FAILURE_TOO_FEW_ARGUMENTS);
  }

  char *query = malloc(0), *data;

  for (int i = 1; i < argc; i++) {
    query = realloc(query, strlen(query) + strlen(argv[i]));
    strcat(query, " ");
    strcat(query, argv[i]);
  }

  data = cya_search(query);
  cya_save_json("output.json");

  free(query);
  free(data);
  return EXIT_SUCCESS;
}
