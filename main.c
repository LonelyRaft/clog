
#include "clog.h"

CLog *g_log = 0;

int main(int argc, char* argv[])
{
    // create clog object
    if(g_log == 0)
        g_log = clog_create(1024);

//    clog_set_logname(g_log, "clogtest");

    clog_error(g_log, "%d %s", argc, argv[0]);
    clog_warn(g_log, "%d %s", argc, argv[0]);
    clog_info(g_log, "%d %s", argc, argv[0]);
    clog_debug(g_log, "%d %s", argc, argv[0]);

    // free clog object
     clog_desrtroy(g_log);
    return 0;
}
