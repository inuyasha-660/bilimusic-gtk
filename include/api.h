#include <gtk/gtk.h>
#include <stddef.h>

typedef struct {
    char *buffer;
    size_t length;
} Buffer;

typedef struct {
    char **bvid;
    char **title;
    int num; // 视频数量
    char **upper_name;
} FavoJson ;

typedef struct {
    int inx;
    char **cid;
    char **part;
} Pages;

typedef struct {
    char *bvid;
    char *title;
    char *upper_name;
    Pages *pages;
} Video;

int api_parse_account();
void api_init();
char *read_file(const char *filename);
int is_file_exists(const char *filename);
gboolean api_get_favo();
int api_get_basic_info();
gboolean api_import_favo(gpointer data);
gboolean api_import_manually(gpointer method_p);
char *int_to_str(long num);
size_t api_curl_finish(void *buffer, size_t size, size_t nmemb, void *userp);

#ifndef API_H
#define API_H

extern const char *API_GET_FAVO;
extern const char *API_GET_FAVO_INFO;
extern const char *API_GET_BASIC_INFO;
extern const char *API_GET_VIDEO_INFO;

extern const char *PATH_ACCOUNT;
extern const char *PATH_AVATAR;
extern const char *DIR_FAVO;
extern const char *PATH_MUSIC;

#endif