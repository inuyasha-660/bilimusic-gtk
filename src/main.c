#include "include/ui.h"
#include <adwaita.h>
#include <gtk/gtk.h>

const char *APP_ID = "com.croyoux.bili.music";

int main(int argc, char **argv)
{
    g_autoptr(AdwApplication) app_bmg = NULL;

    app_bmg = adw_application_new(APP_ID, G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app_bmg, "activate", G_CALLBACK(ui_main), NULL);

    return g_application_run(G_APPLICATION(app_bmg), argc, argv);
}
