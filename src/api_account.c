#include "../../include/api.h"
#include "../../include/ui.h"
#include <cJSON.h>
#include <curl/curl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

extern Account *account;
extern Favo *favo_s;
CURL *Curl_bili;

// 获取收藏夹列表
const char *API_GET_FAVO = "https://api.bilibili.com/x/v3/fav/folder/created/list-all?up_mid=";
// 获取账号基本信息
const char *API_GET_BASIC_INFO = "https://api.bilibili.com/x/web-interface/nav";

const char *PATH_ACCOUNT = "./bilimusic/account.json";
static const char *PATH_AVATAR = "./bilimusic/avatar.jpg";

void api_init()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    Curl_bili = curl_easy_init();
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
    Curl_bili = curl_easy_init();
    if (Curl_bili) {
        CURLcode res;
        char *url_favo = (char *)malloc((66 + sizeof(account->mid)) * sizeof(char));
        sprintf(url_favo, "%s%s", API_GET_FAVO, account->mid);

        curl_easy_setopt(Curl_bili, CURLOPT_WRITEFUNCTION, &api_read_favo);
        curl_easy_setopt(Curl_bili, CURLOPT_URL, url_favo);
        res = curl_easy_perform(Curl_bili);

        curl_easy_cleanup(Curl_bili);

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
        cJSON *mid = cJSON_GetObjectItemCaseSensitive(json, "mid");
        cJSON *DedeUserID = cJSON_GetObjectItemCaseSensitive(json, "DedeUserID");
        cJSON *DedeUserID__ckMd5 = cJSON_GetObjectItemCaseSensitive(json, "DedeUserID__ckMd5");
        cJSON *SESSDATA = cJSON_GetObjectItemCaseSensitive(json, "SESSDATA");
        cJSON *bili_jct = cJSON_GetObjectItemCaseSensitive(json, "bili_jct");
        cJSON *sid = cJSON_GetObjectItemCaseSensitive(json, "sid");

        if (!cJSON_IsString(mid) || !cJSON_IsString(DedeUserID) || !cJSON_IsString(DedeUserID__ckMd5) ||
            !cJSON_IsString(SESSDATA) || !cJSON_IsString(bili_jct) || !cJSON_IsString(sid)) {
            puts("Error: Parse bilimusic/account.json error");
            goto end;
        }

        account->mid = strdup(mid->valuestring);
        account->islogin = 1;
        account->bili_jct = strdup(bili_jct->valuestring);
        account->DedeUserID = strdup(DedeUserID->valuestring);
        account->DedeUserID__ckMd5 = strdup(DedeUserID__ckMd5->valuestring);
        account->sid = strdup(sid->valuestring);
        account->SESSDATA = strdup(SESSDATA->valuestring);

        free(file_str);
        fclose(file);
        cJSON_Delete(json);
        return 0;
    }

end:
    account->mid = "未登录";
    account->islogin = 0;
    account->bili_jct = NULL;
    account->DedeUserID = NULL;
    account->DedeUserID__ckMd5 = NULL;
    account->sid = NULL;
    account->SESSDATA = NULL;

    return 1;
}

void api_get_avatar()
{
    if (account->face == NULL) {
        puts("Error: account->face is NULL");
        return;
    }
    printf("INFO: Get %s\n", account->face);
    Curl_bili = curl_easy_init();
    if (Curl_bili) {
        FILE *img_f = fopen(PATH_AVATAR, "wb");
        if (img_f == NULL) {
            puts("Error: open avatar.jpg fail");
        }

        CURLcode res;
        curl_easy_setopt(Curl_bili, CURLOPT_WRITEDATA, img_f);
        curl_easy_setopt(Curl_bili, CURLOPT_URL, account->face);

        res = curl_easy_perform(Curl_bili);
        if (res != CURLE_OK) {
            printf("-> Failed(%d)\n", res);
            printf("Error: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(Curl_bili);
        fclose(img_f);
        return;
    }
    puts("Error: Curl_bili error");
}

char *int_to_str(int num)
{
    int len = 0, num_c = num;
    do {
        num_c /= 10;
        len++;
    } while (num_c > 10);

    char *ret = (char *)malloc((len + 2) * sizeof(char));
    sprintf(ret, "%d", num);
    ret[len + 2] = '\0';

    return ret;
}

size_t api_rw_basic_info_from_buffer(void *buffer, size_t size, size_t nmemb, void *userp)
{
    char *res_info = (char *)buffer;
    mkdir("./bilimusic", S_IRWXU | S_IRWXG | S_IRWXO);
    FILE *file = fopen(PATH_ACCOUNT, "w+");
    if (file != NULL) {
        cJSON *data, *face, *mid;
        cJSON *json_net = cJSON_Parse(res_info);
        if (json_net == NULL) {
            puts("Error: Parse account json error");
            cJSON_Delete(json_net);
            goto end;
        }

        data = cJSON_GetObjectItemCaseSensitive(json_net, "data");
        if (data == NULL) {
            puts("Error: read account data error");
            cJSON_Delete(json_net);
            goto end;
        }
        face = cJSON_GetObjectItemCaseSensitive(data, "face");
        if (face == NULL) {
            puts("Error: read account face error");
            cJSON_Delete(json_net);
            goto end;
        }
        mid = cJSON_GetObjectItemCaseSensitive(data, "mid");
        if (mid == NULL) {
            puts("Error: read account mid error");
            cJSON_Delete(json_net);
            goto end;
        }

        if (!cJSON_IsString(face) || !cJSON_IsNumber(mid)) {
            puts("Error: get face and mid error");
            cJSON_Delete(json_net);
            goto end;
        }

        char *mid_str = int_to_str(mid->valueint);
        account->face = (char *)malloc(strlen(face->valuestring) * sizeof(char));
        account->mid = (char *)malloc(strlen(mid_str) * sizeof(char));
        account->face = strdup(face->valuestring);
        account->mid = strdup(mid_str);
        cJSON_Delete(json_net);

        cJSON *root;
        root = cJSON_CreateObject();
        cJSON_AddItemToObject(root, "mid", cJSON_CreateString(account->mid));
        cJSON_AddItemToObject(root, "DedeUserID", cJSON_CreateString(account->DedeUserID));
        cJSON_AddItemToObject(root, "DedeUserID__ckMd5", cJSON_CreateString(account->DedeUserID__ckMd5));
        cJSON_AddItemToObject(root, "SESSDATA", cJSON_CreateString(account->SESSDATA));
        cJSON_AddItemToObject(root, "bili_jct", cJSON_CreateString(account->bili_jct));
        cJSON_AddItemToObject(root, "sid", cJSON_CreateString(account->sid));

        fprintf(file, "%s", cJSON_Print(root));
        api_get_avatar();
        api_get_favo();

        fclose(file);
        cJSON_Delete(root);
        return 0;
    }

end:
    account->mid = NULL;
    account->face = NULL;

    return 1;
}

int api_get_basic_info_net()
{
    if (account->SESSDATA == NULL) {
        puts("Error: SESSDATA is NULL");
        return 1;
    }
    Curl_bili = curl_easy_init();
    if (Curl_bili) {
        int res;
        char *cookie = (char *)malloc(10 + strlen(account->SESSDATA));
        sprintf(cookie, "SESSDATA=%s", account->SESSDATA);

        curl_easy_setopt(Curl_bili, CURLOPT_WRITEFUNCTION, &api_rw_basic_info_from_buffer);
        curl_easy_setopt(Curl_bili, CURLOPT_COOKIE, cookie);
        curl_easy_setopt(Curl_bili, CURLOPT_URL, API_GET_BASIC_INFO);
        res = curl_easy_perform(Curl_bili);

        curl_easy_cleanup(Curl_bili);
        return res;
    }

    puts("Error: api_get_basic_info return code 1");
    return 1;
}
