
#include "xlog.h"
#include <stdlib.h>
#include <string.h>

clog_t *xlog  =NULL;

void xlog_init(const char *_name)
{
    if(xlog != NULL)
        return;
    char* buff = NULL;
    do
    {
        const char* home =
                getenv("PRJHOME1");
        if(home == NULL)
            break;
        int home_len = strlen(home);
        const char* logcfg =
                "/resource/data/conf/clogcfg.ini";
        int logcfg_len = strlen(logcfg);
        const char* logdir =
                "/resource/data/log/";
        int logdir_len = strlen(logdir);
        int length = home_len + logcfg_len;
        if(logcfg_len < logdir_len)
            length = home_len + logdir_len;
        buff = (char*)malloc(length + 4);
        if(buff == NULL)
            break;
        memcpy(buff, home, home_len);
        memcpy(buff + home_len, logcfg, logcfg_len);
        buff[home_len + logcfg_len] = 0;
        xlog = clog_read_cfg(buff);
        if(xlog == NULL)
            break;
        memcpy(buff + home_len, logdir, logdir_len);
        buff[home_len + logdir_len] = 0;
        clog_set_dir(xlog, buff);
    }while(0);
     if(xlog == NULL)
         xlog = clog_create(1024);
     if(buff != NULL)
         free(buff);
     clog_set_name(xlog, _name);
}

void xlog_deinit()
{
    if(xlog == NULL)
        return;
    clog_desrtroy(xlog);
}
