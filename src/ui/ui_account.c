#include "../../include/api.h"
#include "../../include/ui.h"
#include "gtk/gtkshortcut.h"
#include <gtk/gtk.h>

const char *APP_LOGIN_NAME = "Bili Login";
const int WIN_LOGIN_WIDTH = 650;
const int WIN_LOGIN_HEIGHT = 350;

GtkWidget *Label_log;

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

void bili_login(GtkWidget *widget, gpointer app)
{
    GtkWidget *win_login;
    GtkWidget *box_login, *label_prompt, *entry_uid, *btn_confirm;
    GtkApplication *app_bmg = (GtkApplication *)app;

    win_login = gtk_application_window_new(app_bmg);
    gtk_window_set_title(GTK_WINDOW(win_login), APP_LOGIN_NAME);
    gtk_window_set_default_size(GTK_WINDOW(win_login), WIN_LOGIN_WIDTH, WIN_LOGIN_HEIGHT);

    box_login = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    label_prompt = gtk_label_new("输入 UID 以登陆");
    gtk_widget_set_margin_top(label_prompt, 50);

    entry_uid = gtk_entry_new();
    gtk_widget_set_margin_top(entry_uid, 10);
    gtk_widget_set_margin_start(entry_uid, 100);
    gtk_widget_set_margin_end(entry_uid, 100);

    btn_confirm = gtk_button_new_with_label("登陆");
    gtk_widget_set_margin_top(btn_confirm, 10);
    gtk_widget_set_margin_start(btn_confirm, 100);
    gtk_widget_set_margin_end(btn_confirm, 100);
    g_signal_connect(btn_confirm, "clicked", G_CALLBACK(login_check), entry_uid);

    Label_log = gtk_label_new("返回:");
    gtk_widget_set_margin_top(Label_log, 20);

    gtk_box_append(GTK_BOX(box_login), label_prompt);
    gtk_box_append(GTK_BOX(box_login), entry_uid);
    gtk_box_append(GTK_BOX(box_login), btn_confirm);
    gtk_box_append(GTK_BOX(box_login), Label_log);

    gtk_window_set_child(GTK_WINDOW(win_login), box_login);
    gtk_window_present(GTK_WINDOW(win_login));
}