#include <gtk/gtk.h>

typedef struct {
    int islogin; // 1: 已登陆 0: 未登录
    char *uid;
} Account;

void ui_main(GtkApplication *app_bmg);
GtkWidget *ui_source(GtkApplication *app_bmg);
void bili_login(GtkWidget *widget, gpointer app);