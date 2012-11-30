#include "wget.h"

#include <string>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  int written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

void wget(const string& url, const string& save_path)
{
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

    FILE *bodyfile = fopen(save_path.c_str(), "w");
    curl_easy_setopt(curl_handle,   CURLOPT_WRITEDATA, bodyfile);
    curl_easy_perform(curl_handle);
    fclose(bodyfile);
    curl_easy_cleanup(curl_handle);
}


