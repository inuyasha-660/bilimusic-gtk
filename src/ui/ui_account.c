#include "../../include/api.h"
#include "../../include/ui.h"
#include "libsoup/soup-cookie.h"
#include <gtk/gtk.h>
#include <string.h>
#include <webkit/webkit.h>

// 跳转登陆界面
const char *API_LOGIN = "https://passport.bilibili.com/login";

const char *APP_LOGIN_NAME = "Bili Login";
const int WIN_LOGIN_WIDTH = 1350;
const int WIN_LOGIN_HEIGHT = 850;

extern Account *account;

void get_cookie(WebKitCookieManager *cookie_mgr, GAsyncResult *res)
{
    GList *cookit_ls = webkit_cookie_manager_get_cookies_finish(cookie_mgr, res, NULL);

    for (GList *cookie_l = cookit_ls; cookie_l != NULL; cookie_l = cookie_l->next) {
        SoupCookie *soup_cookie = (SoupCookie *)cookie_l->data;
        if (!strcmp(soup_cookie_get_name(soup_cookie), "DedeUserID")) {
            const char *value = soup_cookie_get_value(soup_cookie);
            account->DedeUserID = (char *)malloc(strlen(value) * sizeof(char));
            account->DedeUserID = strdup(value);
            continue;
        }
        if (!strcmp(soup_cookie_get_name(soup_cookie), "DedeUserID__ckMd5")) {
            const char *value = soup_cookie_get_value(soup_cookie);
            account->DedeUserID__ckMd5 = (char *)malloc(strlen(value) * sizeof(char));
            account->DedeUserID__ckMd5 = strdup(value);
            continue;
        }
        if (!strcmp(soup_cookie_get_name(soup_cookie), "SESSDATA")) {
            const char *value = soup_cookie_get_value(soup_cookie);
            account->SESSDATA = (char *)malloc(strlen(value) * sizeof(char));
            account->SESSDATA = strdup(value);
            continue;
        }
        if (!strcmp(soup_cookie_get_name(soup_cookie), "bili_jct")) {
            const char *value = soup_cookie_get_value(soup_cookie);
            account->bili_jct = (char *)malloc(strlen(value) * sizeof(char));
            account->bili_jct = strdup(value);
            continue;
        }
        if (!strcmp(soup_cookie_get_name(soup_cookie), "sid")) {
            const char *value = soup_cookie_get_value(soup_cookie);
            account->sid = (char *)malloc(strlen(value) * sizeof(char));
            account->sid = strdup(value);
            continue;
        }
    }
    g_list_free(cookit_ls);

    api_get_basic_info_net();
}

void load_changed(GtkWidget *widget)
{
    WebKitNetworkSession *net_session = webkit_network_session_get_default();
    WebKitCookieManager *cookie_mgr = webkit_network_session_get_cookie_manager(net_session);

    webkit_cookie_manager_get_all_cookies(cookie_mgr, NULL, (GAsyncReadyCallback)get_cookie, NULL);
}

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