#include "include/api.h"
#include "include/ui.h"
#include <cJSON.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ucontext.h>

extern Account *account;
extern Favo *favo_s;
extern FavoJson *favo_json;
extern Import *import;
extern CURL *Curl_bili;

int api_import_man_favo_parse(Buffer *buffer_man_favo)
{
    cJSON *res_json = cJSON_Parse(buffer_man_favo->buffer);
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
    return 0;
}

int api_import_man_bvid_write(Video *video)
{
    printf("INFO: Write to %s\n", PATH_MUSIC);

    cJSON *root, *list, *video_item, *pages;
    if (is_file_exists(PATH_MUSIC)) {
        char *music = read_file(PATH_MUSIC);
        if (music == NULL) {
            return 1;
        }

        if (!strcmp(music, "")) {
            printf("Warn: (root)Parse %s is NULL\n", PATH_MUSIC);
            root = cJSON_CreateObject();
            list = cJSON_AddArrayToObject(root, "list");
            goto write;
        }

        root = cJSON_Parse(music);
        list = cJSON_GetObjectItemCaseSensitive(root, "list");
        goto write;
    }

    root = cJSON_CreateObject();
    list = cJSON_AddArrayToObject(root, "list");

write: {
    FILE *file = fopen(PATH_MUSIC, "w+");
    if (file == NULL) {
        printf("Error: Open %s fail\n", PATH_MUSIC);
        return 1;
    }

    video_item = cJSON_CreateObject();
    pages = cJSON_CreateArray();

    cJSON_AddItemToObject(video_item, "bvid", cJSON_CreateString(video->bvid));
    cJSON_AddItemToObject(video_item, "title", cJSON_CreateString(video->title));
    cJSON_AddItemToObject(video_item, "upper_name", cJSON_CreateString(video->upper_name));

    int p = atoi(import->p);
    for (int i = 0; i < video->pages->inx; i++) {
        if (p) {
            if (i != (p - 1)) {
                continue;
            }
        }

        cJSON *page = cJSON_CreateObject();
        cJSON_AddItemToObject(page, "cid", cJSON_CreateString(video->pages->cid[i]));
        cJSON_AddItemToObject(page, "part", cJSON_CreateString(video->pages->part[i]));
        cJSON_AddItemToArray(pages, page);
    }

    cJSON_AddItemToObject(video_item, "pages", pages);
    cJSON_AddItemToArray(list, video_item);
    fprintf(file, "%s", cJSON_Print(root));

    fclose(file);
}
    cJSON_Delete(root);
    return 0;
}

Video *api_get_video_info_parse(Buffer *buffer_video)
{
    cJSON *data, *title, *owner, *owner_name, *pages, *page;
    cJSON *info_json = cJSON_Parse(buffer_video->buffer);
    Video *video = malloc(sizeof(Video));
    video->bvid = NULL;
    video->title = NULL;
    video->upper_name = NULL;
    video->pages = malloc(sizeof(Pages));
    video->pages->cid = NULL;
    video->pages->part = NULL;
    video->pages->inx = 0;

    data = cJSON_GetObjectItemCaseSensitive(info_json, "data");
    title = cJSON_GetObjectItemCaseSensitive(data, "title");
    owner = cJSON_GetObjectItemCaseSensitive(data, "owner");
    owner_name = cJSON_GetObjectItemCaseSensitive(owner, "name");
    if (!cJSON_IsString(title) || !cJSON_IsString(owner_name)) {
        cJSON_Delete(info_json);
        return NULL;
    }
    video->bvid = strdup(import->bvid);
    video->title = strdup(title->valuestring);
    video->upper_name = strdup(owner_name->valuestring);

    pages = cJSON_GetObjectItemCaseSensitive(data, "pages");
    int inx = 0;
    cJSON_ArrayForEach(page, pages)
    {
        cJSON *cid = cJSON_GetObjectItemCaseSensitive(page, "cid");
        cJSON *part = cJSON_GetObjectItemCaseSensitive(page, "part");
        if (!cJSON_IsNumber(cid) || !cJSON_IsString(part)) {
            cJSON_Delete(info_json);
            return NULL;
        }

        char *cid_str = int_to_str((long)cid->valuedouble);
        video->pages->cid = (char **)realloc(video->pages->cid, (inx + 1) * sizeof(char *));
        video->pages->part = (char **)realloc(video->pages->part, (inx + 1) * sizeof(char *));

        video->pages->cid[inx] = strdup(cid_str);
        video->pages->part[inx] = strdup(part->valuestring);

        inx++;
    }
    video->pages->inx = inx;

    return video;
}

gboolean api_import_manually(gpointer method_p)
{
    int method = GPOINTER_TO_INT(method_p);
    Curl_bili = curl_easy_init();
    if (!Curl_bili) {
        puts("Error: Init Curl_bili error");
        return FALSE;
    }

    char *cookie = (char *)malloc(10 + strlen(account->SESSDATA));
    sprintf(cookie, "SESSDATA=%s", account->SESSDATA);
    if (method) {
        char *url_import_favo =
            (char *)malloc((strlen(API_GET_FAVO_INFO) + strlen(import->favo_id) + 12) * sizeof(char));
        sprintf(url_import_favo, "%s%s&pn=1&ps=40", API_GET_FAVO_INFO, import->favo_id);
        char *log_favo = (char *)malloc((strlen("导入收藏夹: ") + strlen(import->favo_id)) * sizeof(char));
        sprintf(log_favo, "导入收藏夹: %s", import->favo_id);
        g_idle_add((GSourceFunc)api_import_update_widget, log_favo);

        Buffer *buffer_man_favo = malloc(sizeof(Buffer));
        buffer_man_favo->buffer = NULL;
        buffer_man_favo->length = 0;

        curl_easy_setopt(Curl_bili, CURLOPT_WRITEDATA, buffer_man_favo);
        curl_easy_setopt(Curl_bili, CURLOPT_WRITEFUNCTION, &api_curl_finish);
        curl_easy_setopt(Curl_bili, CURLOPT_COOKIE, cookie);
        curl_easy_setopt(Curl_bili, CURLOPT_URL, url_import_favo);
        curl_easy_perform(Curl_bili);
        int res_man_favo_parse = api_import_man_favo_parse(buffer_man_favo);
        if (res_man_favo_parse) {
            puts("Error: Parse json(favo-manually) error");
            g_idle_add((GSourceFunc)api_import_update_widget, "导入失败");
            return FALSE;
        }

        free(url_import_favo);
        free(buffer_man_favo->buffer);
        free(buffer_man_favo);

    } else {
        char *url_import_bvid =
            (char *)malloc((strlen(API_GET_VIDEO_INFO) + strlen(import->bvid) + 12) * sizeof(char));
        sprintf(url_import_bvid, "%s%s", API_GET_VIDEO_INFO, import->bvid);
        char *log_bvid = (char *)malloc((strlen("导入视频: ") + strlen(import->bvid) + 1) * sizeof(char));
        sprintf(log_bvid, "导入视频: %s", import->bvid);
        g_idle_add((GSourceFunc)api_import_update_widget, "导入完成");

        Buffer *buffer_man_video = malloc(sizeof(Buffer));
        buffer_man_video->buffer = NULL;
        buffer_man_video->length = 0;

        printf("INFO: Import %s(bvid)\n", import->bvid);
        printf("INFO: Get %s\n", url_import_bvid);

        curl_easy_setopt(Curl_bili, CURLOPT_WRITEDATA, buffer_man_video);
        curl_easy_setopt(Curl_bili, CURLOPT_WRITEFUNCTION, &api_curl_finish);
        curl_easy_setopt(Curl_bili, CURLOPT_COOKIE, cookie);
        curl_easy_setopt(Curl_bili, CURLOPT_URL, url_import_bvid);
        curl_easy_perform(Curl_bili);
        Video *video = api_get_video_info_parse(buffer_man_video);
        int res_write = api_import_man_bvid_write(video);
        if (res_write) {
            printf("Error: Write to %s\n error", PATH_MUSIC);
            g_idle_add((GSourceFunc)api_import_update_widget, "导入失败");
            return FALSE;
        }

        free(url_import_bvid);
        free(buffer_man_video->buffer);
        free(buffer_man_video);
        free(video->bvid);
        free(video->pages);
        free(video->title);
        free(video->upper_name);
        free(video);
    }
    g_idle_add((GSourceFunc)api_import_update_widget, "导入完成");
    curl_easy_cleanup(Curl_bili);
    free(cookie);
    return FALSE;
}
