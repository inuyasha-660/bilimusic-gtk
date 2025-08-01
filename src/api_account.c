#include "../../include/api.h"
#include "../../include/ui.h"
#include <cJSON.h>
#include <curl/curl.h>
#include <stdio.h>

extern Account *account;
CURL *Curl_login;

const char *API_GT_CHALLENGE = "https://passport.bilibili.com/x/passport-login/captcha?source=main_web";

const char *PATH_ACCOUNT = "./account.json";

void api_init()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    Curl_login = curl_easy_init();
}

size_t api_read_gt_challenge_from_json(void *buffer, size_t size, size_t nmemb, void *userp)
{
    char *res_str = (char *)buffer;
    cJSON *data, *geetest, *challenge, *gt;

    cJSON *res_json = cJSON_Parse(res_str);
    data = cJSON_GetObjectItemCaseSensitive(res_json, "data");
    geetest = cJSON_GetObjectItemCaseSensitive(data, "geetest");
    gt = cJSON_GetObjectItemCaseSensitive(geetest, "gt");
    challenge = cJSON_GetObjectItemCaseSensitive(geetest, "challenge");

    return 0;
}

int api_get_gt_challenge()
{
    if (Curl_login) {
        CURLcode res;
        curl_easy_setopt(Curl_login, CURLOPT_WRITEFUNCTION, &api_read_gt_challenge_from_json);
        curl_easy_setopt(Curl_login, CURLOPT_URL, API_GT_CHALLENGE);
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
        return 0;
    }

end:
    account->uid = "未登录";
    account->islogin = 0;

    fclose(file);
    return 1;
}

// 返回错误码, 0: 无错误 1: 账号文件写入错误
int api_login(const char *uid_str)
{
    printf("INFO: Write uid(%s) to account.json\n", uid_str);

    FILE *file = fopen(PATH_ACCOUNT, "w+");
    if (file != NULL) {
        cJSON *root = cJSON_CreateObject();
        cJSON_AddItemToObject(root, "uid", cJSON_CreateString(uid_str));

        fprintf(file, "%s", cJSON_Print(root));
    } else {
        return 1;
    }

    fclose(file);
    return 0;
}