#include "../../include/api.h"
#include "../include/ui.h"
#include "gtk/gtkshortcut.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

extern Account *account;

GtkWidget *ui_source(GtkApplication *app_bmg)
{
    GtkWidget *box_source, *box_account, *box_btn, *center_box;
    GtkWidget *label_info, *btn_login, *btn_refresh;
    GtkCssProvider *provider;

    box_source = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    center_box = gtk_center_box_new();
    box_account = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    box_btn = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    if (account->islogin) {
        char *uid_label = (char *)malloc((6 + sizeof(account->uid)) * sizeof(char));
        sprintf(uid_label, "UID: \n%s", account->uid);

        label_info = gtk_label_new(uid_label);
        btn_login = gtk_button_new_with_label("重新登陆");
    } else {
        label_info = gtk_label_new("未登录\nUID: -");
        btn_login = gtk_button_new_with_label("登陆");
    }
    btn_refresh = gtk_button_new_with_label("刷新");

    gtk_box_append(GTK_BOX(box_account), label_info);
    gtk_box_append(GTK_BOX(box_btn), btn_login);
    gtk_box_append(GTK_BOX(box_btn), btn_refresh);
    gtk_box_append(GTK_BOX(box_account), box_btn);

    gtk_widget_set_margin_top(box_btn, 20);
    gtk_widget_set_margin_start(box_account, 20);

    gtk_center_box_set_start_widget(GTK_CENTER_BOX(center_box), box_account);
    gtk_box_append(GTK_BOX(box_source), center_box);

    gtk_widget_set_name(label_info, "label-info");

    provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider, "#label-info {"
                                                " font-size: 20px;"
                                                " }");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_signal_connect(btn_login, "clicked", G_CALLBACK(bili_login), app_bmg);
    g_signal_connect(btn_refresh, "clicked", G_CALLBACK(api_parse_account), NULL);

    return box_source;
}