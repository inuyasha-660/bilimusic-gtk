#include "include/api.h"
#include "include/ui.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

extern Account *account;

const char *PATH_AVATAR = "bilimusic/avatar.jpg";

GtkWidget *ui_source(GtkApplication *app_bmg)
{
    GtkWidget *box_source, *box_account, *box_btn, *center_box;
    GtkWidget *img_avatar, *label_info, *btn_login, *btn_refresh;
    GtkWidget *label_method, *box_ckbox, *ckbox_favo, *ckbox_bvid;
    GtkWidget *box_load, *label_load, *box_entry, *entry_id, *entry_p, *box_btn_load, *btn_load, *label_path;
    GtkCssProvider *provider;

    box_source = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    center_box = gtk_center_box_new();
    box_account = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    box_btn = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    if (account->islogin) {
        api_get_favo();
    }

    if (account->islogin) {
        img_avatar = gtk_image_new_from_file(PATH_AVATAR);
        char *uid_label = (char *)malloc((6 + sizeof(account->mid)) * sizeof(char));
        sprintf(uid_label, "UID: \n%s", account->mid);

        label_info = gtk_label_new(uid_label);
        btn_login = gtk_button_new_with_label("重新登陆");
    } else {
        img_avatar = gtk_image_new_from_icon_name("avatar-default");
        label_info = gtk_label_new("未登录\nUID: -");
        btn_login = gtk_button_new_with_label("登陆");
    }
    btn_refresh = gtk_button_new_with_label("刷新");

    gtk_widget_set_size_request(img_avatar, 60, 60);
    gtk_box_append(GTK_BOX(box_account), img_avatar);
    gtk_box_append(GTK_BOX(box_account), label_info);
    gtk_box_append(GTK_BOX(box_btn), btn_login);
    gtk_box_append(GTK_BOX(box_btn), btn_refresh);
    gtk_box_append(GTK_BOX(box_account), box_btn);

    gtk_widget_set_margin_top(box_btn, 20);
    gtk_widget_set_margin_start(box_account, 20);

    label_method = gtk_label_new("选择导入方式\n(Bvid/收藏夹)");
    box_ckbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    ckbox_favo = gtk_check_button_new_with_label("收藏夹");
    ckbox_bvid = gtk_check_button_new_with_label("Bvid");
    gtk_check_button_set_group(GTK_CHECK_BUTTON(ckbox_favo), GTK_CHECK_BUTTON(ckbox_bvid));
    gtk_box_append(GTK_BOX(box_ckbox), label_method);
    gtk_box_append(GTK_BOX(box_ckbox), ckbox_bvid);
    gtk_box_append(GTK_BOX(box_ckbox), ckbox_favo);

    box_load = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    box_btn_load = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    box_entry = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    label_load = gtk_label_new("收藏夹/Bvid: \n分p(默认全选): ");
    entry_id = gtk_entry_new();
    entry_p = gtk_entry_new();
    btn_load = gtk_button_new_with_label("导入");
    label_path = gtk_label_new("(默认列表)");
    gtk_widget_set_margin_end(btn_load, 8);
    gtk_widget_set_margin_top(btn_load, 8);
    gtk_widget_set_size_request(btn_load, 60, 30);
    gtk_box_append(GTK_BOX(box_entry), entry_id);
    gtk_box_append(GTK_BOX(box_entry), entry_p);
    gtk_box_append(GTK_BOX(box_load), label_load);
    gtk_widget_set_margin_top(box_entry, 10);
    gtk_box_append(GTK_BOX(box_load), box_entry);
    gtk_box_append(GTK_BOX(box_btn_load), btn_load);
    gtk_box_append(GTK_BOX(box_btn_load), label_path);
    gtk_box_append(GTK_BOX(box_load), box_btn_load);

    gtk_center_box_set_start_widget(GTK_CENTER_BOX(center_box), box_account);
    gtk_center_box_set_center_widget(GTK_CENTER_BOX(center_box), box_ckbox);
    gtk_center_box_set_end_widget(GTK_CENTER_BOX(center_box), box_load);
    gtk_box_append(GTK_BOX(box_source), center_box);

    gtk_widget_set_name(label_info, "label-info");

    provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider, "#label-info {"
                                                " font-size: 20px;"
                                                " }");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_signal_connect(btn_login, "clicked", G_CALLBACK(bili_login), app_bmg);
    g_signal_connect(btn_refresh, "clicked", G_CALLBACK(api_get_basic_info_net), NULL);

    return box_source;
}