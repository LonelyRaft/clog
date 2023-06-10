
#include "clog.h"

CLog *g_log = 0;

int main(int argc, char* argv[])
{
    // create clog object
    if(g_log == 0)
        g_log = clog_read_cfg("./clogcfg.ini");
//    if(g_log == 0)
//        g_log = clog_create(1024);

    clog_error(g_log, "%d %s", argc, argv[0]);
    clog_warn(g_log, "%d %s", argc, argv[0]);
    clog_info(g_log, "%d %s", argc, argv[0]);
    clog_debug(g_log, "%d %s", argc, argv[0]);
    clog_error(g_log, "clog_error test message!");
    clog_warn(g_log, "clog_warn test message!");
    clog_info(g_log, "clog_info test message!");
    clog_debug(g_log, "clog_debug test message!");

    // free clog object
     clog_desrtroy(g_log);
    return 0;
}
