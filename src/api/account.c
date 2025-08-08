#include "include/api.h"
#include "include/ui.h"
#include <cJSON.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

extern Account *account;
extern Favo *favo_s;
extern FavoJson *favo_json;
extern GtkWidget *Box_favo;
extern Import *import;
CURL *Curl_bili;

// 获取收藏夹列表
const char *API_GET_FAVO = "https://api.bilibili.com/x/v3/fav/folder/created/list-all?up_mid=";
// 获取收藏夹信息 pn: 页数 ps: 每页视频个数 max_ps: 40
const char *API_GET_FAVO_INFO = "https://api.bilibili.com/x/v3/fav/resource/list?media_id=";
// 获取账号基本信息
const char *API_GET_BASIC_INFO = "https://api.bilibili.com/x/web-interface/nav";

const char *PATH_ACCOUNT = "./bilimusic/account.json";
static const char *PATH_AVATAR = "./bilimusic/avatar.jpg";
const char *DIR_FAVO = "./bilimusic/favo/";

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

int api_import_favo_write(int inx)
{
    mkdir(DIR_FAVO, S_IRWXU | S_IRWXG | S_IRWXO);
    char *path_favo = (char *)malloc((strlen(favo_s->id[inx]) + 23) * sizeof(char));
    sprintf(path_favo, "%s%s.json", DIR_FAVO, favo_s->id[inx]);

    FILE *file = fopen(path_favo, "w+");
    if (file == NULL) {
        printf("Error: oepn file(%s) is NULL \n", path_favo);
        return 1;
    }
    printf("INFO: Save favo to %s\n", path_favo);

    cJSON *root, *favos;
    root = cJSON_CreateObject();
    favos = cJSON_AddArrayToObject(root, "favo");

    for (int i = 0; i < favo_json->num; i++) {
        cJSON *favo = cJSON_CreateObject();

        cJSON_AddItemToObject(favo, "bvid", cJSON_CreateString(favo_json->bvid[i]));
        cJSON_AddItemToObject(favo, "title", cJSON_CreateString(favo_json->title[i]));
        cJSON_AddItemToObject(favo, "upper_name", cJSON_CreateString(favo_json->upper_name[i]));
        cJSON_AddItemToObject(favo, "page", cJSON_CreateString(favo_json->page[i]));
        cJSON_AddItemToArray(favos, favo);
    }
    fprintf(file, "%s", cJSON_Print(root));

    fclose(file);
    cJSON_Delete(root);
    return 0;
}

size_t api_import_favo_finish(void *buffer, size_t size, size_t nmemb, void *userp)
{
    char *res = (char *)buffer;
    cJSON *data, *medias, *media, *title, *page, *upper, *upper_name, *bvid;

    cJSON *res_json = cJSON_Parse(res);
    data = cJSON_GetObjectItemCaseSensitive(res_json, "data");
    if (data == NULL) {
        cJSON_Delete(res_json);
        return 1;
    }
    medias = cJSON_GetObjectItemCaseSensitive(data, "medias");
    if (medias == NULL) {
        cJSON_Delete(res_json);
        return 1;
    }

    int inx = favo_json->num;
    cJSON_ArrayForEach(media, medias)
    {
        title = cJSON_GetObjectItemCaseSensitive(media, "title");
        page = cJSON_GetObjectItemCaseSensitive(media, "page");
        upper = cJSON_GetObjectItemCaseSensitive(media, "upper");
        upper_name = cJSON_GetObjectItemCaseSensitive(upper, "name");
        bvid = cJSON_GetObjectItemCaseSensitive(media, "bvid");

        if (!cJSON_IsString(title) || !cJSON_IsString(upper_name) || !cJSON_IsString(bvid) ||
            !cJSON_IsNumber(page)) {
            cJSON_Delete(res_json);
            return 1;
        }

        char *page_str = int_to_str((long)page->valuedouble);
        favo_json->title = (char **)realloc(favo_json->title, (inx + 1) * sizeof(char *));
        favo_json->upper_name = (char **)realloc(favo_json->upper_name, (inx + 1) * sizeof(char *));
        favo_json->page = (char **)realloc(favo_json->page, (inx + 1) * sizeof(char *));
        favo_json->bvid = (char **)realloc(favo_json->bvid, (inx + 1) * sizeof(char *));

        favo_json->title[inx] = strdup(title->valuestring);
        favo_json->upper_name[inx] = strdup(upper_name->valuestring);
        favo_json->page[inx] = strdup(page_str);
        favo_json->bvid[inx] = strdup(bvid->valuestring);

        inx++;
    }
    favo_json->num = inx;

    cJSON_Delete(res_json);

    return size;
}

gboolean api_import_favo(gpointer data)
{
    int inx = GPOINTER_TO_INT(data);

    Curl_bili = curl_easy_init();
    if (Curl_bili) {
        int total_pg = (int)ceil(atoi(favo_s->media_count[inx]) / 40.0);
        for (int i = 1; i <= total_pg; i++) {
            char *cookie = (char *)malloc(10 + strlen(account->SESSDATA));
            sprintf(cookie, "SESSDATA=%s", account->SESSDATA);
            char *url_signal_favo =
                (char *)malloc((strlen(API_GET_FAVO_INFO) + strlen(favo_s->id[inx]) + 12) * sizeof(char));
            sprintf(url_signal_favo, "%s%s&pn=%d&ps=40", API_GET_FAVO_INFO, favo_s->id[inx], i);

            printf("INFO: Import %s\n", favo_s->title[inx]);
            printf("INFO: Get(%d/%d) %s\n", i, total_pg, url_signal_favo);
            curl_easy_setopt(Curl_bili, CURLOPT_WRITEFUNCTION, &api_import_favo_finish);
            curl_easy_setopt(Curl_bili, CURLOPT_COOKIE, cookie);
            curl_easy_setopt(Curl_bili, CURLOPT_URL, url_signal_favo);
            curl_easy_perform(Curl_bili);
        }
    }
    curl_easy_cleanup(Curl_bili);
    api_import_favo_write(inx);

    return FALSE;
}

size_t api_import_man_favo_finish(void *buffer, size_t size, size_t nmemb, void *userp)
{
    char *res_str = (char *)buffer;
    cJSON *res_json = cJSON_Parse(res_str);
    cJSON *data, *info, *title, *media_count;

    data = cJSON_GetObjectItemCaseSensitive(res_json, "data");
    info = cJSON_GetObjectItemCaseSensitive(data, "info");
    title = cJSON_GetObjectItemCaseSensitive(info, "title");
    media_count = cJSON_GetObjectItemCaseSensitive(info, "media_count");
    if (!cJSON_IsString(title) || !cJSON_IsNumber(media_count)) {
        cJSON_Delete(res_json);
        return 1;
    }

    // 将手动导入的收藏夹添加到 favo_s
    int inx = favo_s->inx;
    favo_s->id = (char **)realloc(favo_s->id, inx * sizeof(char *));
    favo_s->title = (char **)realloc(favo_s->title, inx * sizeof(char *));
    favo_s->media_count = (char **)realloc(favo_s->media_count, inx * sizeof(char *));

    favo_s->id[inx] = strdup(import->favo_id);
    favo_s->title[inx] = strdup(title->valuestring);
    char *media_count_str = int_to_str((long)media_count->valuedouble);
    favo_s->media_count[inx] = strdup(media_count_str);

    api_import_favo(GINT_TO_POINTER(inx));

    cJSON_Delete(res_json);
    free(media_count_str);
    return size;
}

gboolean api_import_manually(gpointer method_p)
{
    int method = GPOINTER_TO_INT(method_p);
    Curl_bili = curl_easy_init();
    if (!Curl_bili) {
        puts("Error: Init Curl_bili error");
        return FALSE;
    }

    if (method) {
        char *cookie = (char *)malloc(10 + strlen(account->SESSDATA));
        sprintf(cookie, "SESSDATA=%s", account->SESSDATA);
        char *url_import_favo =
            (char *)malloc((strlen(API_GET_FAVO_INFO) + strlen(import->favo_id) + 12) * sizeof(char));
        sprintf(url_import_favo, "%s%s&pn=1&ps=40", API_GET_FAVO_INFO, import->favo_id);

        curl_easy_setopt(Curl_bili, CURLOPT_WRITEFUNCTION, &api_import_man_favo_finish);
        curl_easy_setopt(Curl_bili, CURLOPT_COOKIE, cookie);
        curl_easy_setopt(Curl_bili, CURLOPT_URL, url_import_favo);
        curl_easy_perform(Curl_bili);

        free(cookie);
        free(url_import_favo);

    } else {
        // TODO: 完成 bvid 导入
    }

    curl_easy_cleanup(Curl_bili);
    return FALSE;
}

size_t api_get_favo_finish(void *buffer, size_t size, size_t nmemb, void *userp)
{
    char *res_str = (char *)buffer;
    cJSON *data, *list, *favo, *id, *title, *media_count;

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

        char *id_str = int_to_str((long)id->valuedouble);
        char *meida_count_id = int_to_str((long)media_count->valuedouble);
        favo_s->id = (char **)realloc(favo_s->id, (inx + 1) * sizeof(char *));
        favo_s->title = (char **)realloc(favo_s->title, (inx + 1) * sizeof(char *));
        favo_s->media_count = (char **)realloc(favo_s->media_count, (inx + 1) * sizeof(char *));

        favo_s->id[inx] = strdup(id_str);
        favo_s->title[inx] = strdup(title->valuestring);
        favo_s->media_count[inx] = strdup(meida_count_id);

        inx++;
    }
    favo_s->inx = inx;
    g_idle_add((GSourceFunc)api_get_favo_update_widget, NULL);

    cJSON_Delete(res_json);
    return size;
}

gboolean api_get_favo()
{
    if (!account->islogin) {
        return FALSE;
    }

    Curl_bili = curl_easy_init();
    if (Curl_bili) {
        char *cookie = (char *)malloc(10 + strlen(account->SESSDATA));
        sprintf(cookie, "SESSDATA=%s", account->SESSDATA);
        char *url_favo = (char *)malloc((66 + sizeof(account->mid)) * sizeof(char));
        sprintf(url_favo, "%s%s", API_GET_FAVO, account->mid);

        curl_easy_setopt(Curl_bili, CURLOPT_WRITEFUNCTION, &api_get_favo_finish);
        curl_easy_setopt(Curl_bili, CURLOPT_COOKIE, cookie);
        curl_easy_setopt(Curl_bili, CURLOPT_URL, url_favo);
        curl_easy_perform(Curl_bili);

        curl_easy_cleanup(Curl_bili);
        free(cookie);
        free(url_favo);
        return FALSE;
    }

    return FALSE;
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

size_t api_get_basic_info_finish(void *buffer, size_t size, size_t nmemb, void *userp)
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
        return size;
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

        printf("INFO: Get(Basic information): %s", API_GET_BASIC_INFO);
        curl_easy_setopt(Curl_bili, CURLOPT_WRITEFUNCTION, &api_get_basic_info_finish);
        curl_easy_setopt(Curl_bili, CURLOPT_COOKIE, cookie);
        curl_easy_setopt(Curl_bili, CURLOPT_URL, API_GET_BASIC_INFO);
        res = curl_easy_perform(Curl_bili);

        curl_easy_cleanup(Curl_bili);
        free(cookie);
        return res;
    }

    puts("Error: api_get_basic_info return code 1");
    return 1;
}
