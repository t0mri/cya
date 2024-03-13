#include "cya.h"
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT_FAILURE_CURL_INIT 10
#define EXIT_FAILURE_TOO_FEW_ARGUMENTS 20
#define EXIT_FAILURE_MEMORY_ALLOCATION 30
#define EXIT_FAILURE_DATA_FETCH 40

#define EXIT_FAILURE_JSON_PARSE 50

struct memory {
  char *response;
  size_t size;
};

static size_t write_function(void *data, size_t size, size_t nmemb,
                             void *clientp) {
  size_t realsize = size * nmemb;
  struct memory *mem = (struct memory *)clientp;

  char *ptr = realloc(mem->response, mem->size + realsize + 1);
  if (!ptr)
    return 0;

  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;

  return realsize;
}

char *cya_search(char *query) {
  char *baseURL = "https://www.youtube.com/results?search_query=",
       *encoded_query, *url,
       *start_delimiter_substring = "itemSectionRenderer\":",
       *end_delimiter_substring = "},{\"continuationItemRenderer",
       *data_extract, *data;
  size_t data_extract_length;

  CURLcode response_status_code;
  CURL *curl;
  struct memory chunk = {0};

  curl = curl_easy_init();
  if (!curl) {
    fprintf(stderr, "Error: Unable to initiate cURL\n");
    exit(EXIT_FAILURE_CURL_INIT);
  }

  if (!query) {
    fprintf(stderr,
            "Error: A query (aka keywords) is required to make a search\n");
    curl_easy_cleanup(curl);
    exit(EXIT_FAILURE_TOO_FEW_ARGUMENTS);
  }

  encoded_query = curl_easy_escape(curl, query, strlen(query));
  url = malloc(strlen(baseURL) + strlen(encoded_query) + 1);
  if (!url) {
    fprintf(stderr, "Error: Failed to allocate memory for URL\n");
    curl_easy_cleanup(curl);
    exit(EXIT_FAILURE_MEMORY_ALLOCATION);
  }
  strcpy(url, baseURL);
  strcat(url, encoded_query);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl, CURLOPT_URL, url);

  response_status_code = curl_easy_perform(curl);
  if (response_status_code != CURLE_OK) {
    fprintf(stderr, "Error: Unable to search the video\n");
    curl_easy_cleanup(curl);
    exit(EXIT_FAILURE_DATA_FETCH);
  }

  data_extract = strstr(chunk.response, start_delimiter_substring);
  if (!data_extract) {
    fprintf(stderr, "Error: Something went wrong on extracting data\n");
  }
  data_extract += strlen(start_delimiter_substring);
  *(strstr(data_extract, end_delimiter_substring)) = '\0';

  data_extract_length = strlen(data_extract);
  data = malloc(data_extract_length + 1);
  if (!data) {
    fprintf(stderr, "Error: Failed to allocate memory for data copy\n");
    free(url);
    free(chunk.response);
    curl_easy_cleanup(curl);
    curl_free(encoded_query);
    exit(EXIT_FAILURE_MEMORY_ALLOCATION);
  }
  strcpy(data, data_extract);

  free(url);
  free(chunk.response);
  curl_easy_cleanup(curl);
  curl_free(encoded_query);
  return data;
}

constructed_data **cya_parse(char *buffer) {
  constructed_data **data;
  cJSON *json, *contents;
  int8_t contents_length, data_index = 0;

  json = cJSON_Parse(buffer);
  if (!json) {
    fprintf(stderr, "Error: Unable to parse JSON\n");
    cJSON_Delete(json);
    exit(EXIT_FAILURE_JSON_PARSE);
  }

  contents = cJSON_GetObjectItem(json, "contents");
  contents_length = cJSON_GetArraySize(contents);

  data = malloc(contents_length * sizeof(constructed_data));

  for (int i = 0; i < contents_length; i++) {
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

    // extract video id
    cJSON *video_id = cJSON_GetObjectItem(videoRenderer, "videoId");

    data[data_index] = malloc(sizeof(constructed_data));

    data[data_index]->title = cJSON_GetStringValue(title);
    data[data_index]->channel = cJSON_GetStringValue(channel);
    data[data_index]->video_id = cJSON_GetStringValue(video_id);

    printf("%s\n", data[data_index]->video_id);
    data_index++;
  }

  return data;
}
