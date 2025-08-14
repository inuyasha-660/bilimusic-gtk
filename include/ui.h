#include <gtk/gtk.h>

typedef struct {
    int islogin; // 1: 已登陆 0: 未登录
    char *mid;
    char *face;
    char *DedeUserID;
    char *DedeUserID__ckMd5;
    char *SESSDATA;
    char *bili_jct;
    char *sid;
} Account;

typedef struct {
    char *bvid;
    char *favo_id;
    char *p;
} Import;

typedef struct {
    char **id;
    char **title;
    char **media_count;
    int inx;
} Favo;

void ui_main(GtkApplication *app_bmg);
void api_update_music_list();
GtkWidget *ui_source(GtkApplication *app_bmg);
void bili_login(GtkWidget *widget, gpointer app);
gboolean api_get_favo_update_widget();
gboolean api_import_update_widget(gpointer log_g);