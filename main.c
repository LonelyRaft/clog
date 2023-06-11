
#include <unistd.h>
#include "xlog.h"

clog_t *xlog = 0;

int main(int argc, char* argv[])
{
    // create clog object
    if(xlog == 0)
        xlog = clog_read_cfg("./clogcfg.ini");

//    clog_error(glog, "clog_error test message!");
//    clog_error(glog, "%d %s", argc, argv[0]);
//    clog_warn(glog, "clog_warn test message!");
//    clog_warn(glog, "%d %s", argc, argv[0]);
//    clog_info(glog, "clog_info test message!");
//    clog_info(glog, "%d %s", argc, argv[0]);
//    clog_debug(glog, "clog_debug test message!");
//    clog_debug(glog, "%d %s", argc, argv[0]);

    xlogError("error message!");
    sleep(1);
    xlogError("%s", argv[0]);
    sleep(1);
    xlogWarn("warn message!");
    sleep(1);
    xlogWarn("%s", argv[0]);
    sleep(1);
    xlogInfo("info message!");
    sleep(1);
    xlogInfo("%s", argv[0]);
    sleep(1);
    xlogDebug("debug message!");
    sleep(1);
    xlogDebug("%s", argv[0]);

    // free clog object
    clog_desrtroy(xlog);
    return 0;
}
