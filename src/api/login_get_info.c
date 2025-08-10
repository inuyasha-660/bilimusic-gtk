
#include "include/api.h"
#include "include/ui.h"
#include <cJSON.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

extern Account *account;
extern CURL *Curl_bili;

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

        printf("INFO: Get(Avatar): %s", account->face);
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

int api_get_basic_info_parse(Buffer *buffer_info)
{
    mkdir("./bilimusic", S_IRWXU | S_IRWXG | S_IRWXO);
    FILE *file = fopen(PATH_ACCOUNT, "w+");
    if (file != NULL) {
        cJSON *data, *face, *mid;
        cJSON *json_net = cJSON_Parse(buffer_info->buffer);
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
        free(mid_str);
        return 0;
    }

end:
    account->mid = NULL;
    account->face = NULL;

    return 1;
}

int api_get_basic_info()
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
        Buffer *buffer_info = malloc(sizeof(Buffer));
        buffer_info->buffer = NULL;
        buffer_info->length = 0;

        printf("INFO: Get(Basic information): %s", API_GET_BASIC_INFO);
        curl_easy_setopt(Curl_bili, CURLOPT_WRITEFUNCTION, &api_curl_finish);
        curl_easy_setopt(Curl_bili, CURLOPT_COOKIE, cookie);
        curl_easy_setopt(Curl_bili, CURLOPT_URL, API_GET_BASIC_INFO);
        res = curl_easy_perform(Curl_bili);
        api_get_basic_info_parse(buffer_info);

        curl_easy_cleanup(Curl_bili);
        free(cookie);
        return res;
    }

    puts("Error: api_get_basic_info return code 1");
    return 1;
}
