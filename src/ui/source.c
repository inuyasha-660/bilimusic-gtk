#include "glib.h"
#include "glibconfig.h"
#include "include/api.h"
#include "include/ui.h"
#include <gtk/gtk.h>
#include <string.h>

extern Account *account;
extern Favo *favo_s;
GtkWidget *Box_favo;

const char *PATH_AVATAR = "bilimusic/avatar.jpg";

gboolean api_import_favo_init(GtkWidget *btn_add, gpointer data)
{
    g_thread_new("import-favo", (GThreadFunc)api_import_favo, data);
    return FALSE;
}

gboolean api_get_favo_update_widget()
{
    for (int i = 0; i < favo_s->inx; i++) {
        GtkWidget *box_signal_favo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
        GtkWidget *btn_add = gtk_button_new_from_icon_name("list-add");
        char *favo_info =
            (char *)malloc(strlen((favo_s->title[i]) + 3 + strlen(favo_s->media_count[i])) * sizeof(char));
        sprintf(favo_info, "%s [%s]", favo_s->title[i], favo_s->media_count[i]);
        GtkWidget *label_info = gtk_label_new(favo_info);

        g_signal_connect(btn_add, "clicked", G_CALLBACK(api_import_favo_init), GINT_TO_POINTER(i));

        gtk_box_append(GTK_BOX(box_signal_favo), btn_add);
        gtk_box_append(GTK_BOX(box_signal_favo), label_info);
        gtk_widget_set_name(box_signal_favo, "sinal-favo");
        gtk_box_append(GTK_BOX(Box_favo), box_signal_favo);
    }
    return FALSE;
}

GtkWidget *ui_source(GtkApplication *app_bmg)
{
    GtkWidget *box_source, *box_account, *box_btn, *center_box;
    GtkWidget *img_avatar, *label_info, *btn_login, *btn_refresh;
    GtkWidget *label_method, *box_ckbox, *ckbox_favo, *ckbox_bvid;
    GtkWidget *box_load, *label_load, *box_entry, *entry_id, *entry_p, *box_btn_load, *btn_load, *label_path;
    GtkWidget *scroll_favo;
    GtkCssProvider *provider;

    box_source = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    center_box = gtk_center_box_new();
    box_account = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    box_btn = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    Box_favo = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    scroll_favo = gtk_scrolled_window_new();

    gtk_widget_set_margin_top(scroll_favo, 20);
    gtk_widget_set_margin_end(scroll_favo, 20);
    gtk_widget_set_margin_start(scroll_favo, 20);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_favo), Box_favo);
    gtk_widget_set_vexpand(scroll_favo, TRUE);

    puts("INFO: Create a new thread to get favorites");
    g_thread_new("get-favo", (GThreadFunc)api_get_favo, NULL);

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
    gtk_box_append(GTK_BOX(box_source), scroll_favo);

    gtk_widget_set_name(label_info, "label-info");

    provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider, "#label-info {"
                                                " font-size: 20px;"
                                                " }"
                                                " #sinal-favo {"
                                                " font-size: 20px;"
                                                " min-height: 45px;"
                                                " }");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_signal_connect(btn_login, "clicked", G_CALLBACK(bili_login), app_bmg);
    g_signal_connect(btn_refresh, "clicked", G_CALLBACK(api_get_basic_info_net), NULL);

    return box_source;
}