#include <gtk/gtk.h>

typedef struct {
    char **bvid;
    char **title;
    int num; // 视频数量
    char **page; // 视频分p
    char **upper_name;
} FavoJson ;

int api_parse_account();
void api_init();
gboolean api_get_favo();
int api_get_basic_info_net();
gboolean api_import_favo(gpointer data);
gboolean api_import_manually(gpointer method_p);