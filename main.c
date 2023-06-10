
#include "clog.h"

CLog *g_log = 0;

int main(int argc, char* argv[])
{
    // create clog object
    if(g_log == 0)
        g_log = clog_create(1024);

    clog_set_logname(g_log, "clogtest");
    clog_show_stdout(g_log, 1);
    clog_show_function(g_log, 1);
    clog_show_position(g_log, 1);
    clog_set_loglevel(g_log, CLOG_LEVEL_DEBUG);

    clog_error(g_log, "%d %s", argc, argv[0]);
    clog_warn(g_log, "%d %s", argc, argv[0]);
    clog_info(g_log, "%d %s", argc, argv[0]);
    clog_debug(g_log, "%d %s", argc, argv[0]);

    // free clog object
     clog_desrtroy(g_log);
    return 0;
}
