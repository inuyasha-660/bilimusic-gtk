#include "include/api.h"
#include <curl/curl.h>
#include <stdlib.h>

CURL *Curl_bili;

// 获取收藏夹列表
const char *API_GET_FAVO = "https://api.bilibili.com/x/v3/fav/folder/created/list-all?up_mid=";
// 获取收藏夹信息 pn: 页数 ps: 每页视频个数 max_ps: 40
const char *API_GET_FAVO_INFO = "https://api.bilibili.com/x/v3/fav/resource/list?media_id=";
// 获取账号基本信息
const char *API_GET_BASIC_INFO = "https://api.bilibili.com/x/web-interface/nav";
// 获取视频信息
const char *API_GET_VIDEO_INFO = "https://api.bilibili.com/x/web-interface/view?bvid=";

const char *PATH_ACCOUNT = "./bilimusic/account.json";
const char *PATH_AVATAR = "./bilimusic/avatar.jpg";
const char *DIR_FAVO = "./bilimusic/favo/";
const char *PATH_MUSIC = "./bilimusic/music.json";

void api_init()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    Curl_bili = curl_easy_init();
}

char *int_to_str(long num)
{
    int len = 0;
    long num_c = num;
    do {
        num_c /= 10;
        len++;
    } while (num_c > 10);

    char *ret = (char *)malloc((len + 2) * sizeof(char));
    sprintf(ret, "%ld", num);
    ret[len + 2] = '\0';

    return ret;
}

int is_file_exists(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        fclose(file);
        return 1;
    } else {
        return 0;
    }
}

size_t api_curl_finish(void *buffer, size_t size, size_t nmemb, void *userp)
{
    size_t len_buffer = size * nmemb;
    Buffer *buffer_s = (Buffer *)userp;
    buffer_s->buffer = (char *)realloc(buffer_s->buffer, (buffer_s->length + len_buffer + 1) * sizeof(char));

    memcpy(buffer_s->buffer + buffer_s->length, buffer, len_buffer);
    buffer_s->length += len_buffer;
    buffer_s->buffer[buffer_s->length] = '\0';

    return len_buffer;
}