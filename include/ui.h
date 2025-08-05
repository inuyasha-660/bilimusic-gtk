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
    int *id;
    char **title;
    int *media_count;
} Favo;

void ui_main(GtkApplication *app_bmg);
GtkWidget *ui_source(GtkApplication *app_bmg);
void bili_login(GtkWidget *widget, gpointer app);