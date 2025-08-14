#include "cJSON.h"
#include <include/api.h>
#include <include/ui.h>
#include <stdio.h>
#include <stdlib.h>

extern List *list_default;

static void init_video(int inx)
{
    list_default->video[inx] = malloc(sizeof(Video));
    list_default->video[inx]->bvid = NULL;
    list_default->video[inx]->pages = NULL;
    list_default->video[inx]->upper_name = NULL;

    list_default->video[inx]->pages = malloc(sizeof(Pages));
    list_default->video[inx]->pages->inx = 0;
    list_default->video[inx]->pages->cid = NULL;
    list_default->video[inx]->pages->part = NULL;
}

gboolean api_read_music_list()
{
    printf("INFO: Read %s\n", PATH_MUSIC);
    char *music_str = read_file(PATH_MUSIC);
    if (music_str == NULL) {
        printf("Error: Read %s err(1)\n", PATH_MUSIC);
        return FALSE;
    }

    cJSON *root, *list, *video;
    root = cJSON_Parse(music_str);
    list = cJSON_GetObjectItemCaseSensitive(root, "list");
    int inx = 0;
    cJSON_ArrayForEach(video, list)
    {
        cJSON *bvid = cJSON_GetObjectItemCaseSensitive(video, "bvid");
        cJSON *title = cJSON_GetObjectItemCaseSensitive(video, "title");
        cJSON *upper_name = cJSON_GetObjectItemCaseSensitive(video, "upper_name");
        if (!cJSON_IsString(bvid) || !cJSON_IsString(title) || !cJSON_IsString(upper_name)) {
            cJSON_Delete(root);
            printf("Error: Read %s err(2)\n", PATH_MUSIC);
            return FALSE;
        }

        init_video(inx);

        list_default->video[inx]->bvid = strdup(bvid->valuestring);
        list_default->video[inx]->title = strdup(title->valuestring);
        list_default->video[inx]->upper_name = strdup(upper_name->valuestring);

        cJSON *pages = cJSON_GetObjectItemCaseSensitive(video, "pages");

        cJSON *page;
        int inx_pages = 0;
        cJSON_ArrayForEach(page, pages)
        {
            cJSON *cid = cJSON_GetObjectItemCaseSensitive(page, "cid");
            cJSON *part = cJSON_GetObjectItemCaseSensitive(page, "part");

            list_default->video[inx]->pages->cid =
                (char **)realloc(list_default->video[inx]->pages->cid, (inx_pages + 1) * sizeof(char *));
            list_default->video[inx]->pages->part =
                (char **)realloc(list_default->video[inx]->pages->part, (inx_pages + 1) * sizeof(char *));

            list_default->video[inx]->pages->cid[inx_pages] = strdup(cid->valuestring);
            list_default->video[inx]->pages->part[inx_pages] = strdup(part->valuestring);

            inx_pages++;
        }
        list_default->video[inx]->pages->inx = inx_pages;

        inx++;
    }
    list_default->inx = inx;

    api_update_music_list();

    cJSON_Delete(root);
    return FALSE;
}