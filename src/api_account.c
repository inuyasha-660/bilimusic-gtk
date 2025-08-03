#include "../../include/api.h"
#include "../../include/ui.h"
#include <cJSON.h>
#include <curl/curl.h>
#include <stdio.h>

extern Account *account;
extern Favo *favo_s;
CURL *Curl_login;

// 获取收藏夹列表
const char *API_GET_FAVO = "https://api.bilibili.com/x/v3/fav/folder/created/list-all?up_mid=";

const char *PATH_ACCOUNT = "./account.json";

void api_init()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    Curl_login = curl_easy_init();
}

size_t api_read_favo(void *buffer, size_t size, size_t nmemb, void *userp)
{
    char *res_str = (char *)buffer;
    cJSON *data, *list, *favo, *id, *title, *media_count;
    puts(res_str);
    cJSON *res_json = cJSON_Parse(res_str);
    data = cJSON_GetObjectItemCaseSensitive(res_json, "data");
    list = cJSON_GetObjectItemCaseSensitive(data, "list");

    int inx = 0;
    cJSON_ArrayForEach(favo, list)
    {
        id = cJSON_GetObjectItemCaseSensitive(favo, "id");
        title = cJSON_GetObjectItemCaseSensitive(favo, "title");
        media_count = cJSON_GetObjectItemCaseSensitive(favo, "media_count");

        if (!cJSON_IsNumber(id) || !cJSON_IsString(title) || !cJSON_IsNumber(media_count)) {
            cJSON_Delete(res_json);
            return 1;
        }
        printf("%d\n", id->valueint);
        favo_s->id = (int *)realloc(favo_s->id, (inx + 1) * sizeof(int));

        favo_s->id[inx] = id->valueint;
        inx++;
    }

    return 0;
}

int api_get_favo()
{
    if (Curl_login) {
        CURLcode res;
        char *url_favo = (char *)malloc((66 + sizeof(account->uid)) * sizeof(char));
        sprintf(url_favo, "%s%s", API_GET_FAVO, account->uid);

        curl_easy_setopt(Curl_login, CURLOPT_WRITEFUNCTION, &api_read_favo);
        curl_easy_setopt(Curl_login, CURLOPT_URL, url_favo);
        res = curl_easy_perform(Curl_login);

        return res;
    }

    return 1;
}

int api_parse_account()
{
    FILE *file = fopen(PATH_ACCOUNT, "r");
    if (file != NULL) {
        fseek(file, 0L, SEEK_END);
        long size = ftell(file);
        char *file_str = (char *)malloc((size + 1) * sizeof(char));
        fseek(file, 0L, SEEK_SET);

        int ch, inx = 0;
        while ((ch = fgetc(file)) != EOF) {
            file_str[inx] = ch;
            inx++;
        }
        file_str[inx] = '\0';

        cJSON *json = cJSON_Parse(file_str);
        cJSON *uid = cJSON_GetObjectItemCaseSensitive(json, "uid");

        if (!cJSON_IsString(uid)) {
            goto end;
        }

        account->uid = strdup(uid->valuestring);
        account->islogin = 1;

        free(file_str);
        fclose(file);
        cJSON_Delete(json);
        return 0;
    }

end:
    account->uid = "未登录";
    account->islogin = 0;

    return 1;
}

// 返回错误码, 0: 无错误 1: 账号文件写入错误
int api_login(const char *uid_str)
{
    // printf("INFO: Write uid(%s) to account.json\n", uid_str);

    // FILE *file = fopen(PATH_ACCOUNT, "w+");
    // if (file != NULL) {
    //     cJSON *root = cJSON_CreateObject();
    //     cJSON_AddItemToObject(root, "uid", cJSON_CreateString(uid_str));

    //     fprintf(file, "%s", cJSON_Print(root));
    // } else {
    //     return 1;
    // }

    // fclose(file);

    return 0;
}