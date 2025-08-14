#include "glib-object.h"
#include "include/api.h"
#include "include/ui.h"
#include <gtk/gtk.h>
#include <stdio.h>

const int WIN_WIDTH = 1550;
const int WIN_HEIGHT = 900;
const char *APP_NAME = "Bili Music";

Account *account;
Favo *favo_s;
FavoJson *favo_json;
List *list_default;
GtkWidget *Box_list_music, *CenBox_search;
GtkWidget *Medeia_control, *Label_name;
char *Title;
int Quality = Q192K;

static void init_account_favo()
{
    account = malloc(sizeof(Account));
    puts("INFO: Parse bilibili/account.json");
    api_parse_account();

    favo_s = malloc(sizeof(Favo));
    favo_s->id = NULL;
    favo_s->title = NULL;
    favo_s->media_count = NULL;
    favo_s->inx = 0;

    favo_json = malloc(sizeof(FavoJson));
    favo_json->bvid = NULL;
    favo_json->title = NULL;
    favo_json->upper_name = NULL;
    favo_json->num = 0;

    puts("INFO: Initialize curl");
    api_init();
}

static void init_music()
{
    list_default = malloc(sizeof(List));
    list_default->inx = 0;
    list_default->video = malloc(sizeof(Video));
    puts("INFO: thread[read-musiclist]: Read music.json");
    g_thread_new("read-musiclist", (GThreadFunc)api_read_music_list, NULL);
}

void switch_revealer_status(GtkWidget *btn, gpointer revealer)
{
    GtkWidget *revealer_part = (GtkWidget *)revealer;
    gboolean status = gtk_revealer_get_reveal_child(GTK_REVEALER(revealer_part));
    gtk_revealer_set_reveal_child(GTK_REVEALER(revealer_part), !status);
}

void api_play_from_filename(gpointer get_music_p)
{
    GetMusic *get_music = (GetMusic *)get_music_p;
    GtkMediaStream *media_stream = gtk_media_file_new_for_filename(get_music->filename);
    gtk_media_stream_play(media_stream);
    gtk_media_controls_set_media_stream(GTK_MEDIA_CONTROLS(Medeia_control), media_stream);
}

static gboolean api_get_music_init(GtkWidget *btn_get, gpointer get_music_p)
{

    GetMusic *get_music = (GetMusic *)get_music_p;
    GtkMediaStream *stream_old = gtk_media_controls_get_media_stream(GTK_MEDIA_CONTROLS(Medeia_control));
    if (stream_old != NULL) {
        gtk_media_stream_set_playing(stream_old, FALSE);
        g_object_unref(stream_old);
    }
    gtk_label_set_text(GTK_LABEL(Label_name), get_music->title);

    size_t len_filename = strlen(PATH_CACHE) + strlen(list_default->video[get_music->i_bvid]->bvid) + 6 +
                          strlen(list_default->video[get_music->i_bvid]->pages->cid[get_music->j_part]);
    char *filename = (char *)malloc((len_filename + 1) * sizeof(char));
    sprintf(filename, "%s%s-%s-%d.m4s", PATH_CACHE, list_default->video[get_music->i_bvid]->bvid,
            list_default->video[get_music->i_bvid]->pages->cid[get_music->j_part], Quality);

    get_music->filename = strdup(filename);
    if (is_file_exists(filename)) {
        g_idle_add((GSourceFunc)api_play_from_filename, get_music);
        return FALSE;
    }

    get_music->filename = strdup(filename);
    free(filename);
    g_thread_new("get-music", (GThreadFunc)api_get_music, get_music);
    return FALSE;
}

void api_update_music_list()
{
    int total_main = list_default->inx;
    for (int i = 0; i < total_main; i++) {
        size_t len_title = strlen(list_default->video[i]->title);
        char *info_title = (char *)malloc((len_title + 1) * sizeof(char));
        sprintf(info_title, "%s", list_default->video[i]->title);

        size_t len_uppername = strlen(list_default->video[i]->upper_name);
        char *info_uppername = (char *)malloc((len_uppername + 1) * sizeof(char));
        sprintf(info_uppername, "%s", list_default->video[i]->upper_name);

        GtkWidget *cbox_music_main = gtk_center_box_new();
        GtkWidget *btn_music_main = gtk_button_new();
        GtkWidget *label_title = gtk_label_new(info_title);
        GtkWidget *label_uppername = gtk_label_new(info_uppername);
        GtkWidget *img_show_part = gtk_image_new_from_icon_name("pan-down-symbolic");

        gtk_label_set_xalign(GTK_LABEL(label_title), 0.01);
        gtk_center_box_set_start_widget(GTK_CENTER_BOX(cbox_music_main), label_title);
        gtk_center_box_set_center_widget(GTK_CENTER_BOX(cbox_music_main), label_uppername);
        gtk_center_box_set_end_widget(GTK_CENTER_BOX(cbox_music_main), img_show_part);
        gtk_button_set_child(GTK_BUTTON(btn_music_main), cbox_music_main);
        gtk_widget_set_name(btn_music_main, "btn-musiclist");
        gtk_widget_set_name(label_title, "label-musictitle");
        gtk_widget_set_name(label_uppername, "label-musicuppername");

        GtkWidget *revealer_part = gtk_revealer_new();
        gtk_revealer_set_reveal_child(GTK_REVEALER(revealer_part), FALSE);
        GtkWidget *box_music_part_main = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        int total_part = list_default->video[i]->pages->inx;
        for (int j = 0; j < total_part; j++) {
            size_t len_part = strlen(list_default->video[i]->pages->part[j]);
            char *info_part = (char *)malloc((len_part + 1) * sizeof(char));
            sprintf(info_part, "%s", list_default->video[i]->pages->part[j]);

            GtkWidget *cbox_music_part = gtk_center_box_new();
            GtkWidget *btn_music_part = gtk_button_new();
            GtkWidget *label_part = gtk_label_new(info_part);
            GtkWidget *img_play = gtk_image_new_from_icon_name("media-playback-start");
            gtk_label_set_xalign(GTK_LABEL(label_part), 0.02);

            gtk_center_box_set_start_widget(GTK_CENTER_BOX(cbox_music_part), label_part);
            gtk_center_box_set_end_widget(GTK_CENTER_BOX(cbox_music_part), img_play);
            gtk_button_set_child(GTK_BUTTON(btn_music_part), cbox_music_part);

            GetMusic *get_music = malloc(sizeof(GetMusic));
            get_music->filename = NULL;
            get_music->title = NULL;
            get_music->i_bvid = i;
            get_music->j_part = j;

            size_t len_music_info = strlen(info_part) + strlen(info_uppername);
            char *music_info = (char *)malloc((len_music_info + 4) * sizeof(char));
            sprintf(music_info, "%s - %s", info_part, info_uppername);

            get_music->title = strdup(music_info);
            g_signal_connect(btn_music_part, "clicked", G_CALLBACK(api_get_music_init), get_music);

            gtk_box_append(GTK_BOX(box_music_part_main), btn_music_part);
            if (j != (total_part - 1)) {
                GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
                gtk_box_append(GTK_BOX(box_music_part_main), separator);
            }

            free(music_info);
            free(info_part);
        }
        gtk_revealer_set_child(GTK_REVEALER(revealer_part), box_music_part_main);

        g_signal_connect(btn_music_main, "clicked", G_CALLBACK(switch_revealer_status), revealer_part);

        gtk_box_append(GTK_BOX(Box_list_music), btn_music_main);
        gtk_box_append(GTK_BOX(Box_list_music), revealer_part);

        if (i != (total_main - 1)) {
            GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
            gtk_box_append(GTK_BOX(Box_list_music), separator);
        }
        free(info_title);
        free(info_uppername);
    }
}

// target_p: 1 显示 0 隐藏
static void update_search_entry(GtkWidget *btn_close_search, gpointer target_p)
{
    int target = GPOINTER_TO_INT(target_p);
    if (target) {
        gtk_widget_set_visible(CenBox_search, TRUE);
    } else {
        gtk_widget_set_visible(CenBox_search, FALSE);
    }
}

static void change_quality(GtkWidget *ckbox, gpointer quality_p) { Quality = GPOINTER_TO_INT(quality_p); }

void ui_main(GtkApplication *app_bmg)
{
    GtkWidget *win_main;
    GtkWidget *box_main, *paned, *stack, *stack_sidebar;
    GtkWidget *header_bar, *btn_search, *btn_menu_setting, *popover_setting, *box_setting;
    GtkWidget *box_tq, *ckbox_tq_64K, *ckbox_tq_132K, *ckbox_tq_192K, *ckbox_tq_dolby, *ckbox_tq_HiRes;
    GtkWidget *box_home;
    GtkWidget *box_ckbox_s;
    GtkWidget *ckbox_s_title, *ckbox_s_uppername, *ckbox_s_favo, *entry_home, *btn_close_search;
    GtkWidget *notebook_media_switch;
    GtkWidget *box_music, *box_favo;
    GtkWidget *scrolled_music;
    GtkWidget *box_music_control;

    init_account_favo();

    win_main = gtk_application_window_new(app_bmg);
    gtk_window_set_title(GTK_WINDOW(win_main), APP_NAME);
    gtk_window_set_default_size(GTK_WINDOW(win_main), WIN_WIDTH, WIN_HEIGHT);

    CenBox_search = gtk_center_box_new();

    // headerbar 组件
    header_bar = gtk_header_bar_new();
    entry_home = gtk_search_entry_new();
    popover_setting = gtk_popover_new();
    box_tq = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    box_setting = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    ckbox_tq_64K = gtk_check_button_new_with_label("64K");
    ckbox_tq_132K = gtk_check_button_new_with_label("132K");
    ckbox_tq_192K = gtk_check_button_new_with_label("192K");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(ckbox_tq_192K), TRUE);
    ckbox_tq_dolby = gtk_check_button_new_with_label("杜比全景声");
    ckbox_tq_HiRes = gtk_check_button_new_with_label("Hi-Res无损");
    gtk_check_button_set_group(GTK_CHECK_BUTTON(ckbox_tq_132K), GTK_CHECK_BUTTON(ckbox_tq_64K));
    gtk_check_button_set_group(GTK_CHECK_BUTTON(ckbox_tq_192K), GTK_CHECK_BUTTON(ckbox_tq_64K));
    gtk_check_button_set_group(GTK_CHECK_BUTTON(ckbox_tq_dolby), GTK_CHECK_BUTTON(ckbox_tq_64K));
    gtk_check_button_set_group(GTK_CHECK_BUTTON(ckbox_tq_HiRes), GTK_CHECK_BUTTON(ckbox_tq_64K));

    g_signal_connect(ckbox_tq_64K, "toggled", G_CALLBACK(change_quality), GINT_TO_POINTER(Q64K));
    g_signal_connect(ckbox_tq_132K, "toggled", G_CALLBACK(change_quality), GINT_TO_POINTER(Q132K));
    g_signal_connect(ckbox_tq_192K, "toggled", G_CALLBACK(change_quality), GINT_TO_POINTER(Q192K));
    g_signal_connect(ckbox_tq_dolby, "toggled", G_CALLBACK(change_quality), GINT_TO_POINTER(QDOLBY));
    g_signal_connect(ckbox_tq_HiRes, "toggled", G_CALLBACK(change_quality), GINT_TO_POINTER(QHIRES));

    gtk_box_append(GTK_BOX(box_tq), ckbox_tq_64K);
    gtk_box_append(GTK_BOX(box_tq), ckbox_tq_132K);
    gtk_box_append(GTK_BOX(box_tq), ckbox_tq_192K);
    gtk_box_append(GTK_BOX(box_tq), ckbox_tq_dolby);
    gtk_box_append(GTK_BOX(box_tq), ckbox_tq_HiRes);

    gtk_box_append(GTK_BOX(box_setting), box_tq);

    gtk_popover_set_child(GTK_POPOVER(popover_setting), box_setting);
    btn_menu_setting = gtk_menu_button_new();
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(btn_menu_setting), popover_setting);
    btn_search = gtk_button_new_from_icon_name("edit-find");

    gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(btn_menu_setting), "open-menu-symbolic");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), btn_search);
    gtk_widget_set_visible(CenBox_search, FALSE);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar), btn_menu_setting);
    gtk_window_set_titlebar(GTK_WINDOW(win_main), header_bar);

    g_signal_connect(btn_search, "clicked", G_CALLBACK(update_search_entry), GINT_TO_POINTER(1));

    box_main = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    box_home = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    box_music = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    box_favo = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    Box_list_music = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    // 侧边栏
    paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_UP_DOWN);
    gtk_widget_set_hexpand(stack, FALSE);
    gtk_widget_set_vexpand(stack, TRUE);
    stack_sidebar = gtk_stack_sidebar_new();

    // music/favo 切换
    notebook_media_switch = gtk_notebook_new();
    gtk_widget_set_hexpand(notebook_media_switch, TRUE);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook_media_switch), TRUE);
    GtkWidget *label_music = gtk_label_new("默认列表");
    GtkWidget *label_favo = gtk_label_new("收藏夹");

    // 搜索栏
    ckbox_s_title = gtk_check_button_new_with_label("标题");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(ckbox_s_title), TRUE);
    ckbox_s_favo = gtk_check_button_new_with_label("收藏夹");
    ckbox_s_uppername = gtk_check_button_new_with_label("作者");
    box_ckbox_s = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_append(GTK_BOX(box_ckbox_s), ckbox_s_title);
    gtk_box_append(GTK_BOX(box_ckbox_s), ckbox_s_uppername);
    gtk_box_append(GTK_BOX(box_ckbox_s), ckbox_s_favo);
    btn_close_search = gtk_button_new_from_icon_name("window-close");
    gtk_center_box_set_start_widget(GTK_CENTER_BOX(CenBox_search), box_ckbox_s);
    gtk_center_box_set_center_widget(GTK_CENTER_BOX(CenBox_search), entry_home);
    gtk_center_box_set_end_widget(GTK_CENTER_BOX(CenBox_search), btn_close_search);
    gtk_widget_set_margin_top(CenBox_search, 10);
    gtk_widget_set_margin_start(box_ckbox_s, 10);
    gtk_widget_set_margin_end(btn_close_search, 10);
    g_signal_connect(btn_close_search, "clicked", G_CALLBACK(update_search_entry), GINT_TO_POINTER(0));

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook_media_switch), box_music, label_music);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook_media_switch), box_favo, label_favo);
    gtk_box_append(GTK_BOX(box_home), CenBox_search);
    gtk_box_append(GTK_BOX(box_home), notebook_media_switch);

    // music 下的滚动窗口
    scrolled_music = gtk_scrolled_window_new();
    gtk_widget_set_margin_top(Box_list_music, 10);
    gtk_widget_set_margin_start(Box_list_music, 20);
    gtk_widget_set_margin_end(Box_list_music, 20);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_music), Box_list_music);
    gtk_widget_set_vexpand(scrolled_music, TRUE);
    gtk_box_append(GTK_BOX(box_music), scrolled_music);
    Medeia_control = gtk_media_controls_new(NULL);
    Label_name = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(Label_name), 0.0);
    gtk_widget_set_margin_end(Label_name, 10);
    gtk_widget_set_margin_start(Label_name, 10);
    gtk_widget_set_margin_bottom(Medeia_control, 10);
    gtk_widget_set_margin_end(Medeia_control, 10);
    gtk_widget_set_margin_start(Medeia_control, 10);
    box_music_control = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(box_music_control), Label_name);
    gtk_box_append(GTK_BOX(box_music_control), Medeia_control);
    gtk_box_append(GTK_BOX(box_home), box_music_control);

    init_music();

    gtk_stack_add_titled(GTK_STACK(stack), box_home, "box-home", "曲目");
    gtk_stack_add_titled(GTK_STACK(stack), ui_source(app_bmg), "box-source", "音乐源");

    gtk_stack_sidebar_set_stack(GTK_STACK_SIDEBAR(stack_sidebar), GTK_STACK(stack));

    gtk_paned_set_start_child(GTK_PANED(paned), stack_sidebar);
    gtk_paned_set_resize_start_child(GTK_PANED(paned), TRUE);
    gtk_paned_set_shrink_start_child(GTK_PANED(paned), FALSE);
    gtk_paned_set_end_child(GTK_PANED(paned), stack);
    gtk_paned_set_resize_end_child(GTK_PANED(paned), TRUE);
    gtk_paned_set_shrink_end_child(GTK_PANED(paned), FALSE);
    gtk_paned_set_position(GTK_PANED(paned), 100);

    gtk_box_append(GTK_BOX(box_main), paned);

    gtk_window_set_child(GTK_WINDOW(win_main), box_main);
    gtk_window_present(GTK_WINDOW(win_main));
}
