#include "include/api.h"
#include "include/ui.h"
#include <cJSON.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>

extern Account *account;

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

        fclose(file);
        cJSON_Delete(json);
        free(file_str);
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