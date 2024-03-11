#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT_FAILURE_CURL_INIT 10
#define EXIT_FAILURE_TOO_FEW_ARGUMENTS 20
#define EXIT_FAILURE_MEMORY_ALLOCATION 30
#define EXIT_FAILURE_FETCH_DATA 40

struct memory {
  char *response;
  size_t size;
};

static size_t cb(void *data, size_t size, size_t nmemb, void *clientp) {
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

char *data;

char *cya_search(char *query) {
  char *baseURL = "https://www.youtube.com/results?search_query=", *url,
       *substringStartDelimiter = "itemSectionRenderer\":",
       *substringEndDelimiter = "},{\"continuationItemRenderer";

  data = "";
  struct memory chunk = {0};

  CURL *curl = curl_easy_init();
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

  char *escaped_query = curl_easy_escape(curl, query, strlen(query));
  url = malloc(strlen(baseURL) + strlen(escaped_query) + 1);
  if (!url) {
    fprintf(stderr, "Error: Failed to allocate memory for URL\n");
    curl_easy_cleanup(curl);
    exit(EXIT_FAILURE_MEMORY_ALLOCATION);
  }
  strcpy(url, baseURL);
  strcat(url, escaped_query);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl, CURLOPT_URL, url);

  CURLcode statusCode = curl_easy_perform(curl);
  if (statusCode != CURLE_OK) {
    fprintf(stderr, "Error: Unable to search the video\n");
    curl_easy_cleanup(curl);
    exit(EXIT_FAILURE_FETCH_DATA);
  }

  data = strstr(chunk.response, substringStartDelimiter);
  if (!data) {
    fprintf(stderr, "Error: Something went wrong on extracting data\n");
  }
  *data = '\0';
  data += strlen(substringStartDelimiter);
  char *tempData = strstr(data, substringEndDelimiter);
  if (tempData) {
    *tempData = '\0';
  }

  free(url);
  free(chunk.response);
  curl_easy_cleanup(curl);
  curl_free(escaped_query);
  return data;
}

void cya_save_json(char *fileName) {
  FILE *file = fopen(fileName, "w");
  fputs(data, file);
  fclose(file);
}
