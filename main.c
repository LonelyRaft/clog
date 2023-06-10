
#include "clog.h"
#include <unistd.h>

CLog *glog = 0;

int main(int argc, char* argv[])
{
    // create clog object
    if(glog == 0)
        glog = clog_read_cfg("./clogcfg.ini");
    if(glog == 0)
        glog = clog_create(1024);

    clog_error(glog, "clog_error test message!");
    clog_error(glog, "%d %s", argc, argv[0]);
    clog_warn(glog, "clog_warn test message!");
    clog_warn(glog, "%d %s", argc, argv[0]);
    clog_info(glog, "clog_info test message!");
    clog_info(glog, "%d %s", argc, argv[0]);
    clog_debug(glog, "clog_debug test message!");
    clog_debug(glog, "%d %s", argc, argv[0]);

    // free clog object
    clog_desrtroy(glog);
    return 0;
}
