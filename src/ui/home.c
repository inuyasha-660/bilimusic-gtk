#include "include/api.h"
#include "include/ui.h"
#include <gtk/gtk.h>

const int WIN_WIDTH = 1550;
const int WIN_HEIGHT = 900;
const char *APP_NAME = "Bili Music";

Account *account;
Favo *favo_s;

void ui_main(GtkApplication *app_bmg)
{
    GtkWidget *win_main;
    GtkWidget *header_bar, *btn_menu_setting;
    GtkWidget *box_main, *paned, *stack, *stack_sidebar;
    GtkWidget *box_home;

    account = malloc(sizeof(Account));
    favo_s = malloc(sizeof(Favo));
    favo_s->id = NULL;
    favo_s->title = NULL;
    favo_s->media_count = NULL;
    favo_s->inx = 0;
    puts("INFO: Parse bilibili/account.json");
    api_parse_account();
    puts("INFO: Initialize curl");
    api_init();

    win_main = gtk_application_window_new(app_bmg);
    gtk_window_set_title(GTK_WINDOW(win_main), APP_NAME);
    gtk_window_set_default_size(GTK_WINDOW(win_main), WIN_WIDTH, WIN_HEIGHT);

    header_bar = gtk_header_bar_new();
    btn_menu_setting = gtk_menu_button_new();
    gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(btn_menu_setting), "open-menu-symbolic");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar), btn_menu_setting);

    gtk_window_set_titlebar(GTK_WINDOW(win_main), header_bar);

    box_main = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    box_home = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_UP_DOWN);
    gtk_widget_set_hexpand(stack, FALSE);
    gtk_widget_set_vexpand(stack, TRUE);
    stack_sidebar = gtk_stack_sidebar_new();

    GtkWidget *label_home = gtk_label_new("Home");
    gtk_box_append(GTK_BOX(box_home), label_home);

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
