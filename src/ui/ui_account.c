#include "../../include/api.h"
#include "../../include/ui.h"
#include <gtk/gtk.h>
#include <webkit/webkit.h>

// 跳转登陆界面
const char *API_LOGIN = "https://passport.bilibili.com/login";

const char *APP_LOGIN_NAME = "Bili Login";
const int WIN_LOGIN_WIDTH = 1350;
const int WIN_LOGIN_HEIGHT = 850;

GtkWidget *Label_log;

int Status_login; // 1: 登录完成(点击下一步) 0: 未点击

void login_check(GtkWidget *widget, gpointer entry)
{
    const char *uid = gtk_editable_get_text((GTK_EDITABLE((GtkWidget *)entry)));

    int err = api_login(uid);
    if (err) {
        gtk_label_set_label(GTK_LABEL(Label_log), "返回: 登陆错误(1)");
    } else {
        gtk_label_set_label(GTK_LABEL(Label_log), "返回: 登陆成功");
    }
}

void get_cookie(WebKitCookieManager *cookie_mgr, GAsyncResult *res)
{
    GList *cookit_ls = webkit_cookie_manager_get_cookies_finish(cookie_mgr, res, NULL);
    for (GList *cookie_l = cookit_ls; cookie_l != NULL; cookie_l = cookie_l->next) {
        SoupCookie *soup_cookie = (SoupCookie *)cookie_l->data;
        printf("Cookie: ");
        printf("%s=%s\n", soup_cookie_get_name(soup_cookie), soup_cookie_get_value(soup_cookie));
    }
    g_list_free(cookit_ls);
}

void load_changed(GtkWidget *widget)
{
    WebKitNetworkSession *net_session = webkit_network_session_get_default();
    WebKitCookieManager *cookie_mgr = webkit_network_session_get_cookie_manager(net_session);

    webkit_cookie_manager_get_all_cookies(cookie_mgr, NULL, (GAsyncReadyCallback)get_cookie, NULL);
}

void change_login_status() { Status_login = 1; }

void bili_login(GtkWidget *widget, gpointer app)
{
    GtkWidget *win_login;
    GtkWidget *box_login, *box_log, *login_webkit, *entry_log, *btn_finish_login;
    GtkApplication *app_bmg = (GtkApplication *)app;

    win_login = gtk_application_window_new(app_bmg);
    gtk_window_set_title(GTK_WINDOW(win_login), APP_LOGIN_NAME);
    gtk_window_set_default_size(GTK_WINDOW(win_login), WIN_LOGIN_WIDTH, WIN_LOGIN_HEIGHT);

    box_login = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    box_log = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

    login_webkit = webkit_web_view_new();
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(login_webkit), "https://passport.bilibili.com/login");
    gtk_widget_set_size_request(login_webkit, 1300, 800);

    entry_log = gtk_entry_new();
    btn_finish_login = gtk_button_new_with_label("下一步");
    gtk_editable_set_text(GTK_EDITABLE(entry_log), "完成登录后, 点击下一步开始读取 cookie");
    gtk_editable_set_editable(GTK_EDITABLE(entry_log), FALSE);
    gtk_widget_set_margin_top(box_log, 10);
    gtk_widget_set_margin_start(box_log, 20);
    gtk_widget_set_margin_end(box_log, 20);
    gtk_widget_set_size_request(entry_log, 1200, -1);

    gtk_box_append(GTK_BOX(box_log), entry_log);
    gtk_box_append(GTK_BOX(box_log), btn_finish_login);

    gtk_box_append(GTK_BOX(box_login), box_log);
    gtk_box_append(GTK_BOX(box_login), login_webkit);

    g_signal_connect(btn_finish_login, "clicked", G_CALLBACK(load_changed), NULL);

    gtk_window_set_child(GTK_WINDOW(win_login), box_login);
    gtk_window_present(GTK_WINDOW(win_login));
}