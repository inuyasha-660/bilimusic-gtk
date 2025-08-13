#include "cJSON.h"
#include "include/api.h"
#include "include/ui.h"
#include <curl/curl.h>
#include <stdio.h>

extern List *list_default;
extern Account *account;
extern CURL *Curl_bili;

int api_read_music_json(Buffer *buffer_stream)
{
    cJSON *data, *dash, *audio;
    cJSON *root = cJSON_Parse(buffer_stream->buffer);

    data = cJSON_GetObjectItemCaseSensitive(root, "data");
    dash = cJSON_GetObjectItemCaseSensitive(data, "dash");
    audio = cJSON_GetObjectItemCaseSensitive(dash, "audio");

    cJSON_Delete(root);
    return 0;
}

int api_get_music(gpointer get_video_p)
{
    GetMusic *get_video = (GetMusic *)get_video_p;

    Curl_bili = curl_easy_init();
    if (Curl_bili) {
        char *cookie = (char *)malloc((10 + strlen(account->SESSDATA)) * sizeof(char));
        sprintf(cookie, "SESSDATA=%s", account->SESSDATA);
        size_t len_url = strlen(list_default->video[get_video->i_bvid]->bvid) +
                         strlen(list_default->video[get_video->i_bvid]->pages->cid[get_video->j_part]) +
                         strlen(API_GET_STREAM) + 16;
        char *url_stream = (char *)malloc((len_url + 1) * sizeof(char));
        sprintf(url_stream, "%s%s&cid=%s&fnval=4048", API_GET_STREAM,
                list_default->video[get_video->i_bvid]->bvid,
                list_default->video[get_video->i_bvid]->pages->cid[get_video->j_part]);

        Buffer *buffer_stream = malloc(sizeof(Buffer));
        buffer_stream->buffer = NULL;
        buffer_stream->length = 0;

        curl_easy_setopt(Curl_bili, CURLOPT_URL, url_stream);
        curl_easy_setopt(Curl_bili, CURLOPT_COOKIE, cookie);
        curl_easy_setopt(Curl_bili, CURLOPT_WRITEFUNCTION, &api_curl_finish);
        curl_easy_setopt(Curl_bili, CURLOPT_WRITEDATA, buffer_stream);
        CURLcode res_stream = curl_easy_perform(Curl_bili);
        if (res_stream != CURLE_OK) {
            printf("Error: Get audio stream code %d\n", res_stream);
            return 1;
        }
        api_read_music_json(buffer_stream);

        free(cookie);
        free(url_stream);
        free(buffer_stream->buffer);
        free(buffer_stream);
    }

    curl_easy_cleanup(Curl_bili);
    return 0;
}