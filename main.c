
#include "xlog.h"

CLog *xlog = 0;

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
    xlogError("%s", argv[0]);
    xlogWarn("warn message!");
    xlogWarn("%s", argv[0]);
    xlogInfo("info message!");
    xlogInfo("%s", argv[0]);
    xlogDebug("debug message!");
    xlogDebug("%s", argv[0]);

    // free clog object
    clog_desrtroy(xlog);
    return 0;
}
