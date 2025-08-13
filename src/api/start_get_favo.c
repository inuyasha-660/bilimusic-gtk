#include "include/api.h"
#include "include/ui.h"
#include <cJSON.h>
#include <curl/curl.h>

extern Account *account;
extern Favo *favo_s;
extern CURL *Curl_bili;

int api_get_favo_parse(Buffer *buffer_favo)
{
    cJSON *data, *list, *favo, *id, *title, *media_count;

    cJSON *res_json = cJSON_Parse(buffer_favo->buffer);
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
    return 0;
}

gboolean api_get_favo()
{
    if (!account->islogin) {
        puts("INFO: No login");
        return FALSE;
    }

    Curl_bili = curl_easy_init();
    if (Curl_bili) {
        char *cookie = (char *)malloc((10 + strlen(account->SESSDATA)) * sizeof(char));
        sprintf(cookie, "SESSDATA=%s", account->SESSDATA);
        char *url_favo = (char *)malloc((66 + sizeof(account->mid)) * sizeof(char));
        sprintf(url_favo, "%s%s", API_GET_FAVO, account->mid);
        Buffer *buffer_favo = malloc(sizeof(Buffer));
        buffer_favo->buffer = NULL;
        buffer_favo->length = 0;

        printf("INFO: Get(favo): %s\n", url_favo);

        curl_easy_setopt(Curl_bili, CURLOPT_WRITEDATA, buffer_favo);
        curl_easy_setopt(Curl_bili, CURLOPT_WRITEFUNCTION, &api_curl_finish);
        curl_easy_setopt(Curl_bili, CURLOPT_COOKIE, cookie);
        curl_easy_setopt(Curl_bili, CURLOPT_URL, url_favo);
        curl_easy_perform(Curl_bili);
        api_get_favo_parse(buffer_favo);

        free(cookie);
        free(url_favo);
    }

    curl_easy_cleanup(Curl_bili);
    return FALSE;
}