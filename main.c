
#include <unistd.h>
#include <stdlib.h>
#include "xlog.h"

int main(int argc, char* argv[])
{
    (void)argc;

    xlog_init("clogtest_name");

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
    sleep(1);
    xlogStatus("%s", argv[0]);

    xlog_deinit();
    return 0;
}
