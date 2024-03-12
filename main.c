#include "libcya/cya.h"
#include <cjson/cJSON.h>
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

  char *query = malloc(0), *data;
  cJSON *json, *contents;
  int contents_length;

  for (int i = 1; i < argc; i++) {
    query = realloc(query, strlen(query) + strlen(argv[i]));
    strcat(query, " ");
    strcat(query, argv[i]);
  }

  data = cya_search(query);
  json = cJSON_Parse(data);
  if (!json) {
    fprintf(stderr, "Error: Unable to parse JSON\n");
    cJSON_Delete(json);
    exit(EXIT_FAILURE_JSON_PARSE);
  }
  contents = cJSON_GetObjectItem(json, "contents");
  contents_length = cJSON_GetArraySize(contents);

  for (int i = contents_length-1; i >=0; i--) {
    cJSON *content = cJSON_GetArrayItem(contents, i);
    cJSON *videoRenderer = cJSON_GetObjectItem(content, "videoRenderer");
    if (!videoRenderer) {
      continue;
    }

    // extract title
    cJSON *title_outer_object = cJSON_GetObjectItem(videoRenderer, "title");
    cJSON *title_runs = cJSON_GetObjectItem(title_outer_object, "runs");
    cJSON *title_inner_object = cJSON_GetArrayItem(title_runs, 0);
    cJSON *title = cJSON_GetObjectItem(title_inner_object, "text");

    // extract channel name
    cJSON *owner_text = cJSON_GetObjectItem(videoRenderer, "ownerText");
    cJSON *channel_runs = cJSON_GetObjectItem(owner_text, "runs");
    cJSON *channel_inner_object = cJSON_GetArrayItem(channel_runs, 0);
    cJSON *channel = cJSON_GetObjectItem(channel_inner_object, "text");

    printf("%d: %s\n\t- %s\n", i, cJSON_GetStringValue(title),
           cJSON_GetStringValue(channel));
  }

  free(query);
  free(data);
  cJSON_Delete(json);
  return EXIT_SUCCESS;
}
