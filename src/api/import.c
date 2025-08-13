#include "include/api.h"
#include "include/ui.h"
#include <cJSON.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

extern Account *account;
extern Favo *favo_s;
extern FavoJson *favo_json;
extern CURL *Curl_bili;

Pages *api_import_get_part_parse(Buffer *buffer_part)
{
    Pages *pages_s = malloc(sizeof(Pages));
    pages_s->cid = NULL;
    pages_s->inx = 0;
    pages_s->part = NULL;
    cJSON *res_json = cJSON_Parse(buffer_part->buffer);
    cJSON *data, *pages, *page;

    data = cJSON_GetObjectItemCaseSensitive(res_json, "data");
    pages = cJSON_GetObjectItemCaseSensitive(data, "pages");

    int inx = 0;
    cJSON_ArrayForEach(page, pages)
    {
        cJSON *cid = cJSON_GetObjectItemCaseSensitive(page, "cid");
        cJSON *part = cJSON_GetObjectItemCaseSensitive(page, "part");
        if (!cJSON_IsNumber(cid) || !cJSON_IsString(part)) {
            puts("->Error: code 1");
            cJSON_Delete(res_json);

            return NULL;
        }

        char *cid_str = int_to_str((long)cid->valuedouble);
        pages_s->cid = (char **)realloc(pages_s->cid, (inx + 1) * sizeof(char *));
        pages_s->part = (char **)realloc(pages_s->part, (inx + 1) * sizeof(char *));

        pages_s->cid[inx] = strdup(cid_str);
        pages_s->part[inx] = strdup(part->valuestring);

        inx++;
    }
    pages_s->inx = inx;

    return pages_s;
}

Pages *api_import_favo_get_part(char *bvid)
{
    char *cookie = (char *)malloc((10 + strlen(account->SESSDATA)) * sizeof(char));
    sprintf(cookie, "SESSDATA=%s", account->SESSDATA);
    CURL *curl_part = curl_easy_init();
    Pages *pages_s;

    if (curl_part) {
        char *url_bvid = (char *)malloc((strlen(API_GET_VIDEO_INFO) + strlen(bvid)) * sizeof(char));
        sprintf(url_bvid, "%s%s", API_GET_VIDEO_INFO, bvid);
        Buffer *buffer_part = malloc(sizeof(Buffer));
        buffer_part->buffer = NULL;
        buffer_part->length = 0;

        curl_easy_setopt(curl_part, CURLOPT_COOKIE, cookie);
        curl_easy_setopt(curl_part, CURLOPT_URL, url_bvid);
        curl_easy_setopt(curl_part, CURLOPT_WRITEFUNCTION, &api_curl_finish);
        curl_easy_setopt(curl_part, CURLOPT_WRITEDATA, buffer_part);
        CURLcode res = curl_easy_perform(curl_part);
        if (res != CURLE_OK) {
            printf("Error: return code %d\n", res);
            return NULL;
        }
        pages_s = api_import_get_part_parse(buffer_part);

        free(cookie);
        free(buffer_part->buffer);
        free(buffer_part);
        return pages_s;
    }

    curl_easy_cleanup(curl_part);

    return NULL;
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

    cJSON *root, *list;
    root = cJSON_CreateObject();
    list = cJSON_AddArrayToObject(root, "list");

    for (int i = 0; i < favo_json->num; i++) {
        Pages *pages_s = api_import_favo_get_part(favo_json->bvid[i]);

        cJSON *favo = cJSON_CreateObject();
        cJSON *pages = cJSON_CreateArray();

        for (int i = 0; i < pages_s->inx; i++) {
            cJSON *page = cJSON_CreateObject();
            cJSON_AddItemToObject(page, "cid", cJSON_CreateString(pages_s->cid[i]));
            cJSON_AddItemToObject(page, "part", cJSON_CreateString(pages_s->part[i]));
            cJSON_AddItemToArray(pages, page);
        }

        cJSON_AddItemToObject(favo, "bvid", cJSON_CreateString(favo_json->bvid[i]));
        cJSON_AddItemToObject(favo, "title", cJSON_CreateString(favo_json->title[i]));
        cJSON_AddItemToObject(favo, "upper_name", cJSON_CreateString(favo_json->upper_name[i]));
        cJSON_AddItemToObject(favo, "pages", pages);
        cJSON_AddItemToArray(list, favo);

        free(pages_s->cid);
        free(pages_s->part);
        free(pages_s);
    }

    printf("INFO: Save favo to %s\n", path_favo);
    fprintf(file, "%s", cJSON_Print(root));

    fclose(file);
    cJSON_Delete(root);
    return 0;
}

int api_import_favo_parse(Buffer *buffer_favo)
{
    cJSON *data, *medias, *media, *title, *upper, *upper_name, *bvid;

    cJSON *res_json = cJSON_Parse(buffer_favo->buffer);
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
        upper = cJSON_GetObjectItemCaseSensitive(media, "upper");
        upper_name = cJSON_GetObjectItemCaseSensitive(upper, "name");
        bvid = cJSON_GetObjectItemCaseSensitive(media, "bvid");

        if (!cJSON_IsString(title) || !cJSON_IsString(upper_name) || !cJSON_IsString(bvid)) {
            cJSON_Delete(res_json);
            return 1;
        }

        favo_json->title = (char **)realloc(favo_json->title, (inx + 1) * sizeof(char *));
        favo_json->upper_name = (char **)realloc(favo_json->upper_name, (inx + 1) * sizeof(char *));
        favo_json->bvid = (char **)realloc(favo_json->bvid, (inx + 1) * sizeof(char *));

        favo_json->title[inx] = strdup(title->valuestring);
        favo_json->upper_name[inx] = strdup(upper_name->valuestring);
        favo_json->bvid[inx] = strdup(bvid->valuestring);

        inx++;
    }
    favo_json->num = inx;

    cJSON_Delete(res_json);

    return 0;
}

gboolean api_import_favo(gpointer data)
{
    int inx = GPOINTER_TO_INT(data);

    Curl_bili = curl_easy_init();
    favo_json->bvid = NULL;
    favo_json->num = 0;
    favo_json->title = NULL;
    favo_json->upper_name = NULL;

    CURLcode res;
    if (Curl_bili) {
        int total_pg = (int)ceil(atoi(favo_s->media_count[inx]) / 40.0);

        char *cookie = (char *)malloc((10 + strlen(account->SESSDATA)) * sizeof(char));
        sprintf(cookie, "SESSDATA=%s", account->SESSDATA);
        char *log_favo =
            (char *)malloc((strlen("导入收藏夹: ") + strlen(favo_s->id[inx]) + 1) * sizeof(char));
        sprintf(log_favo, "导入收藏夹: %s", favo_s->id[inx]);

        g_idle_add((GSourceFunc)api_import_update_widget, log_favo);

        for (int i = 1; i <= total_pg; i++) {
            char *url_signal_favo =
                (char *)malloc((strlen(API_GET_FAVO_INFO) + strlen(favo_s->id[inx]) + 12) * sizeof(char));
            sprintf(url_signal_favo, "%s%s&pn=%d&ps=40", API_GET_FAVO_INFO, favo_s->id[inx], i);
            Buffer *buffer_favo = malloc(sizeof(Buffer));
            buffer_favo->buffer = NULL;
            buffer_favo->length = 0;

            printf("INFO: Import %s\n", favo_s->title[inx]);
            printf("INFO: Get(%d/%d) %s\n", i, total_pg, url_signal_favo);
            curl_easy_setopt(Curl_bili, CURLOPT_WRITEFUNCTION, &api_curl_finish);
            curl_easy_setopt(Curl_bili, CURLOPT_WRITEDATA, buffer_favo);
            curl_easy_setopt(Curl_bili, CURLOPT_COOKIE, cookie);
            curl_easy_setopt(Curl_bili, CURLOPT_URL, url_signal_favo);

            res = curl_easy_perform(Curl_bili);
            if (res != CURLE_OK) {
                printf("Error: code %d\n", res);
                return FALSE;
            }

            int res_parse = api_import_favo_parse(buffer_favo);
            if (res_parse) {
                puts("Error: Parse json(favo) error");
                g_idle_add((GSourceFunc)api_import_update_widget, "导入失败");
                return FALSE;
            }

            free(url_signal_favo);
            free(buffer_favo->buffer);
            free(buffer_favo);
        }
        free(cookie);
    }
    curl_easy_cleanup(Curl_bili);
    int res_write = api_import_favo_write(inx);
    if (!res_write) {
        g_idle_add((GSourceFunc)api_import_update_widget, "导入完成");
    }

    return FALSE;
}