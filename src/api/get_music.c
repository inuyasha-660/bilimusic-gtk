#include "cJSON.h"
#include "include/api.h"
#include "include/ui.h"
#include <curl/curl.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>

extern List *list_default;
extern Account *account;
extern CURL *Curl_bili;
extern int Quality;

int api_down_stream(char *url, GetMusic *get_music)
{
    puts("INFO: Start geting stream");

    mkdir(PATH_CACHE, S_IRWXU | S_IRWXG | S_IRWXO);

    Curl_bili = curl_easy_init();
    if (Curl_bili) {
        FILE *file = fopen(get_music->filename, "w+");
        if (file == NULL) {
            printf("Error: Create %s error\n", get_music->filename);
            return 1;
        }

        curl_easy_setopt(Curl_bili, CURLOPT_USERAGENT, USER_AGENT);
        curl_easy_setopt(Curl_bili, CURLOPT_REFERER, REFERER);
        curl_easy_setopt(Curl_bili, CURLOPT_URL, url);
        curl_easy_setopt(Curl_bili, CURLOPT_WRITEDATA, file);
        CURLcode res = curl_easy_perform(Curl_bili);
        if (res != CURLE_OK) {
            printf("Error: Get stream err(%d)\n", res);
            fclose(file);
            return 1;
        }

        fclose(file);
        g_idle_add((GSourceFunc)api_play_from_filename, get_music);
        curl_easy_cleanup(Curl_bili);

        return 0;
    }

    puts("Error: Init Curl_bili error");
    curl_easy_cleanup(Curl_bili);

    return 1;
}

char *api_read_music_json(Buffer *buffer_stream)
{
    cJSON *data, *dash, *audio_arr, *audio;
    cJSON *root = cJSON_Parse(buffer_stream->buffer);

    data = cJSON_GetObjectItemCaseSensitive(root, "data");
    dash = cJSON_GetObjectItemCaseSensitive(data, "dash");
    audio_arr = cJSON_GetObjectItemCaseSensitive(dash, "audio");

    cJSON_ArrayForEach(audio, audio_arr)
    {
        cJSON *id = cJSON_GetObjectItemCaseSensitive(audio, "id");
        if (!cJSON_IsNumber(id)) {
            printf("Error: Parse stream json err(id is not a number)\n");
            cJSON_Delete(root);
            return NULL;
        }

        if (id->valueint == Quality) {
            cJSON *url_audio = cJSON_GetObjectItemCaseSensitive(audio, "baseUrl");
            if (!cJSON_IsString(url_audio)) {
                printf("Error: url_aduio is not a string");
                cJSON_Delete(root);
                return NULL;
            }

            char *url_clone = strdup(url_audio->valuestring);
            cJSON_Delete(root);

            return url_clone;
        }
    }

    /* 无法正确获取音频流链接有两种可能
     * 一是未找到目标音质(非大会员选择了杜比或 Hi-Res 音质)
     * 二是触发了 BiliBili 的安全风控策略(错误号: 412)
     */
    puts("Error: Prase stream error");

    cJSON_Delete(root);
    return NULL;
}

int api_get_music(GetMusic *get_music)
{
    Curl_bili = curl_easy_init();
    if (Curl_bili) {
        char *cookie = (char *)malloc((10 + strlen(account->SESSDATA)) * sizeof(char));
        sprintf(cookie, "SESSDATA=%s", account->SESSDATA);
        size_t len_url = strlen(list_default->video[get_music->i_bvid]->bvid) +
                         strlen(list_default->video[get_music->i_bvid]->pages->cid[get_music->j_part]) +
                         strlen(API_GET_STREAM) + 16;
        char *url_stream = (char *)malloc((len_url + 1) * sizeof(char));
        sprintf(url_stream, "%s%s&cid=%s&fnval=4048", API_GET_STREAM,
                list_default->video[get_music->i_bvid]->bvid,
                list_default->video[get_music->i_bvid]->pages->cid[get_music->j_part]);

        Buffer *buffer_stream = malloc(sizeof(Buffer));
        buffer_stream->buffer = NULL;
        buffer_stream->length = 0;

        printf("INFO: Get audio json: %s\n", url_stream);

        curl_easy_setopt(Curl_bili, CURLOPT_URL, url_stream);
        curl_easy_setopt(Curl_bili, CURLOPT_COOKIE, cookie);
        curl_easy_setopt(Curl_bili, CURLOPT_WRITEFUNCTION, &api_curl_finish);
        curl_easy_setopt(Curl_bili, CURLOPT_WRITEDATA, buffer_stream);
        CURLcode res_stream = curl_easy_perform(Curl_bili);
        if (res_stream != CURLE_OK) {
            printf("Error: Get audio stream code %d\n", res_stream);
            return 1;
        }
        char *url_audio = api_read_music_json(buffer_stream);
        if (url_audio == NULL) {
            return 1;
        }
        api_down_stream(url_audio, get_music);

        free(cookie);
        free(url_stream);
        free(buffer_stream->buffer);
        free(buffer_stream);
    }

    curl_easy_cleanup(Curl_bili);
    return 0;
}
