
#include "clog.h"

clog_t *_clog;

int main(int argc, char const *argv[])
{
    _clog = clog_create(1024);
    clog_use_stdout(_clog, 1);
    clog_error(_clog, "%s", argv[0]);
    clog_desrtroy(_clog);
    return 0;
}

