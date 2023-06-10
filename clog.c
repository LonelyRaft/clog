#include "clog.h"
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <limits.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif // O_BINARY

static const char* stdfmt[]={
    "\e[0m", // clear format
    "\e[1;34m", // bold and blue
    "\e[1;32m", // bold and green
    "\e[1;33m", // bold and yellow
    "\e[1;31m", // bold and red
};

typedef struct CLog{
    unsigned char m_show_datetime;
    unsigned char m_show_position;
    unsigned char m_show_function;
    unsigned char m_show_loglevel;
    unsigned char m_show_stdout;
    unsigned char m_log_level;
    unsigned char m_name_len;
    unsigned char m_dir_len;
    unsigned int m_bufsz;
    char *m_msgbuf;
    char *m_log_name;
    char *m_log_dir;
    char *m_log_path;
    pthread_mutex_t m_lock;
} CLog;

typedef struct CLogHead
{
    const char* file;
    int line;
    const char* func;
    int level;
}CLogHead;

typedef struct CLogConfig
{
    unsigned int msgsize;
    unsigned char datetime;
    unsigned char loglevel;
    unsigned char function;
    unsigned char position;
    unsigned char _stdout;
    unsigned char level;
    unsigned char dir_len;
    unsigned char name_len;
    char* directory;
    char* name;
}CLogConfig;

static int clog_datetime(
    char* _datetime)
{
    time_t seconds = time(0);
    struct tm curr = {0};
    localtime_r(&seconds, &curr);
    int result = snprintf(
        _datetime, 24,
        "[%04d-%02d-%02d %02d:%02d:%02d] ",
        curr.tm_year + 1900, curr.tm_mon + 1, curr.tm_mday,
        curr.tm_hour, curr.tm_min, curr.tm_sec);
    return result;
}

static int clog_loglevel(
    char* _log_level, int _level)
{
    if(_log_level == NULL)
        return 0;
    switch(_level)
    {
    case CLOG_LEVEL_INFO:
        strcpy(_log_level, "[Info] ");
        break;
    case CLOG_LEVEL_DEBUG:
        strcpy(_log_level, "[Debug] ");
        break;
    case CLOG_LEVEL_WARN:
        strcpy(_log_level, "[Warn] ");
        break;
    case CLOG_LEVEL_ERROR:
        strcpy(_log_level, "[Error] ");
        break;
    default:
        _log_level[0] = 0;
        break;
    }
    return strlen(_log_level);
}

static const char* clog_log_path(
    CLog * _log)
{
    if(_log == NULL)
        return NULL;
    if(_log->m_log_name == NULL)
    {
        _log->m_log_name =
            (char*)malloc(64);
        if(_log->m_log_name == NULL)
            return NULL;
        srand((unsigned int)time(NULL));
        _log->m_name_len = snprintf(
            _log->m_log_name, 63,
            "clog%d", rand()%1001);
        if(_log->m_name_len < 0)
        {
            _log->m_name_len = 0;
            free(_log->m_log_name);
            return NULL;
        }
    }
    if(_log->m_log_dir == NULL)
    {
#ifdef _WIN32
        const char* dir = "./";
#endif // _WIN32
#ifdef __linux__
        const char* dir = "/tmp/";
#endif // __linux__
        _log->m_dir_len = strlen(dir);
        _log->m_log_dir =
            (char*)malloc(_log->m_dir_len + 1);
        if(_log->m_log_dir == NULL)
        {
            _log->m_dir_len = 0;
            return NULL;
        }
        strcpy(_log->m_log_dir, dir);
    }
    if(_log->m_log_path == NULL)
    {
        time_t secs = time(NULL);
        struct tm curr = {0};
        char suffix[32]  ={0};
        localtime_r(&secs, &curr);
        int length = snprintf(
            suffix, 32, "_%04d%02d%02d.log",
            curr.tm_year + 1900,
            curr.tm_mon + 1, curr.tm_mday);
        length += _log->m_dir_len;
        length += _log->m_name_len;
        _log->m_log_path =
            (char*)malloc(length + 1);
        if(_log->m_log_path == NULL)
            return NULL;
        char*  path = _log->m_log_path;
        strcpy(path, _log->m_log_dir);
        path = path + _log->m_dir_len;
        strcpy(path, _log->m_log_name);
        path = path + _log->m_name_len;
        strcpy(path, suffix);
    }
    return _log->m_log_path;
}

static int clog_write(
    CLog * _log, const CLogHead *_head,
    const char* _message, int _length)
{
    int result = 0;
    char header[512] = {0};
    int idx = 0;
    int length = 0;
    if(_log->m_show_datetime)
    {
        length = clog_datetime(header + idx);
        if(length > 0)
            idx += length;
    }
    if(_log->m_show_loglevel)
    {
        length = clog_loglevel(
            header + idx, _head->level);
        if(length > 0)
            idx += length;
    }
    if(_log->m_show_function)
    {
        header[idx++] = '[';
        length = 0;
        while(_head->func[length])
            header[idx++] =
                _head->func[length++];
        header[idx++] = ']';
        header[idx++] = ' ';
    }
    if(_log->m_show_position)
    {
        length = snprintf(
            header + idx, 255 - idx, "[%s:%d]",
            _head->file, _head->line);
        if(length > 0)
            idx += length;
    }
    do {
        const char* path = clog_log_path(_log);
        if(path == NULL)
            break;
        int fd = open(path,
                      O_CREAT | O_BINARY | O_WRONLY | O_APPEND,
                      S_IWUSR | S_IRUSR);
        if(fd < 0)
            break;
        header[idx++] = '\n';
        write(fd, header, idx);
        write(fd, _message, _length);
        close(fd);
        result = idx + _length;
    }while(0);
    if(_log->m_show_stdout)
    {
        printf("%s%s\n%s",
               stdfmt[_head->level],
               header, stdfmt[0]);
        printf("%s\n",_message);
    }
    return result;
}

static int clog_readline(
    int _fd, char* _buff, int size)
{
    int idx = 0;
    char ret = 0;
    char tmp = '\n';
    if(_buff == NULL || size < 1)
        return 0;
    do {
         ret = read(_fd, &tmp, 1);
        if(0 > ret)
            return ret;
        _buff[idx++] = tmp;
        if(idx == size)
            return idx;
    }while(tmp != '\n');
    return idx;
}

CLog *clog_create(
    unsigned int _msgsz_max)
{
    CLog *log = NULL;
    unsigned int size =
        UINT_MAX - sizeof(CLog) - 1;
    if(_msgsz_max == 0 || _msgsz_max > size)
        return log;
    size = _msgsz_max + sizeof(CLog) + 1;
    log = (CLog*)malloc(size);
    if(log == NULL)
        return log;
    memset(log, 0 ,sizeof(CLog));
    log->m_show_datetime  =1;
    log->m_show_loglevel = 1;
    log->m_log_level =
        CLOG_LEVEL_WARN;
    log->m_bufsz = _msgsz_max;
    log->m_msgbuf = (char*)(log + 1);
    pthread_mutex_init(
        &log->m_lock, NULL);
    return log;
}

CLog *clog_read_cfg(
    const char* _cfgpath)
{
    if(_cfgpath == NULL ||
        _cfgpath[0] == 0)
        return NULL;
    int fd = open(_cfgpath, O_RDONLY);
    if(fd < 0)
        return NULL;
    char buffer[512];
    int length = 0;
    unsigned char start = 0;
    unsigned char comment = 0;
    unsigned char cfgcnt = 0;
    CLogConfig config =
        {1024, 1, 1, 0, 0, 0, CLOG_LEVEL_WARN, 0, 0, NULL, NULL};
    do {
        if(cfgcnt >= 9)
            break;
        length = clog_readline(fd, buffer, 511);
        if(length < 2)
            break;
        if(buffer[0] == '#')
        {
            if(buffer[length - 1] != '\n')
                comment = 1;
            continue;
        }
        if(comment)
        {
            if(buffer[length - 1] == '\n')
                comment = 0;
            continue;
        }
        if(length >= 6 && strncmp(buffer, "[clog]", 6) == 0)
        {
            start = 1;
            continue;
        }
        if(start == 0)
            continue;
        if(buffer[0] == '[')
            break;
        buffer[length] = 0;
        char* data = strchr(buffer, '#') ;
        if(data != NULL)
        {
            if(buffer[length - 1] != '\n')
                comment = 1;
            length = data - buffer;
            *data  =0;
        }
        data = strchr(buffer, '=');
        if(data == NULL)
            continue;
        data++;
        cfgcnt++; // assuming this is a valid configuration
        if(length > 8 && strncmp(buffer, "msgsize", 7) == 0)
        {
            while((*data) && !isdigit(*data))
                data++;
            if((*data) == 0)
                continue;
            const char * p = data;
            while((*data) && isdigit(*data))
                data++;
            *(data) = 0;
            config.msgsize = atoi(p);
        }
        else if(length > 9 && strncmp(buffer, "datetime", 8) == 0)
        {
            const char* p =
                strstr(data, "true");
            if(p != NULL) {
                config.datetime = 1;
                continue;
            }
            p = strstr(data, "false");
            if(p != NULL)
                config.datetime = 0;
        }
        else if(length > 8 && strncmp(buffer, "loglevel", 7) == 0)
        {
            const char* p =
                strstr(data, "true");
            if(p != NULL) {
                config.loglevel = 1;
                continue;
            }
            p = strstr(data, "false");
            if(p != NULL)
                config.loglevel = 0;
        }
        else if(length > 9 && strncmp(buffer, "function", 8) == 0)
        {
            const char* p =
                strstr(data, "true");
            if(p != NULL) {
                config.function = 1;
                continue;
            }
            p = strstr(data, "false");
            if(p != NULL)
                config.function = 0;
        }
        else if(length > 9 && strncmp(buffer, "position", 8) == 0)
        {
            const char* p =
                strstr(data, "true");
            if(p != NULL) {
                config.position = 1;
                continue;
            }
            p = strstr(data, "false");
            if(p != NULL)
                config.position = 0;
        }
        else if(length > 7 && strncmp(buffer, "stdout", 6) == 0)
        {
            const char* p =
                strstr(data, "true");
            if(p != NULL) {
                config._stdout = 1;
                continue;
            }
            p = strstr(data, "false");
            if(p != NULL)
                config._stdout = 0;
        }
        else if(length > 6 && strncmp(buffer, "level", 5) == 0)
        {
            const char* p =
                strstr(data, "DEBUG");
            if(p != NULL) {
                config.level = CLOG_LEVEL_DEBUG;
                continue;
            }
            p = strstr(data, "INFO");
            if(p != NULL) {
                config.level = CLOG_LEVEL_INFO;
                continue;
            }
            p = strstr(data, "WARN");
            if(p != NULL) {
                config.level = CLOG_LEVEL_WARN;
                continue;
            }
            p = strstr(data, "ERROR");
            if(p != NULL)
                config.level = CLOG_LEVEL_ERROR;
        }
        else if(length > 4 && strncmp(buffer, "dir", 3) == 0)
        {
            if(config.directory != NULL)
                continue;
            const char* dir = data;
            while((*data) && !isspace(*data))
                data++;
            *data = 0;
            int dirlen = strlen(dir);
            if(dirlen <= 0)
                continue;
            config.directory =
                (char*)malloc(dirlen + 4);
            if(config.directory == NULL)
                continue;
            strcpy(config.directory, dir);
            if(config.directory[dirlen - 1] != '/' &&
                config.directory[dirlen - 1] != '\\')
            {
                config.directory[dirlen++] = '/';
                config.directory[dirlen] = 0;
            }
            config.dir_len = dirlen;
        }
        else if(length > 5 && strncmp(buffer, "name", 4) == 0)
        {
            if(config.name != NULL)
                continue;
            const char* name = data;
            while((*data) && !isspace(*data))
                data++;
            *data = 0;
            int nmlen = strlen(name);
            if(nmlen <= 0)
                continue;
            config.name = (char*)malloc(nmlen + 1);
            if(config.name == NULL)
                continue;
            strcpy(config.name, name);
            config.name_len = nmlen;
        }
        else
        {
            cfgcnt--; // assumption is invalid
        }
    }while(1);
    close(fd);
    if(config.msgsize <= 0)
        config.msgsize = 1024;
    CLog *log  = (CLog*)clog_create(config.msgsize);
    if(log != NULL)
    {
        log->m_show_datetime = config.datetime;
        log->m_show_loglevel = config.loglevel;
        log->m_show_function = config.function;
        log->m_show_position = config.position;
        log->m_show_stdout = config._stdout;
        log->m_log_level = config.level;
        log->m_log_dir = config.directory;
        log->m_dir_len = config.dir_len;
        log->m_log_name =config.name;
        log->m_name_len = config.name_len;
    }
    else
    {
        if(config.name != NULL)
            free(config.name);
        if(config.directory != NULL)
            free(config.directory);
    }
    config.directory = config.name = NULL;
    return log;
}

void clog_desrtroy(CLog* _log)
{
    if(_log == NULL)
        return;
    if(_log->m_log_dir)
        free(_log->m_log_dir);
    if(_log->m_log_name)
        free(_log->m_log_name);
    if(_log->m_log_path)
        free(_log->m_log_path);
    free(_log);
}

int clog_show_datetime(
    CLog* _log, int _show)
{
    if(_log == NULL)
        return -1;
    pthread_mutex_lock(&_log->m_lock);
    _log->m_show_datetime = _show;
    pthread_mutex_unlock(&_log->m_lock);
    return 0;
}

int clog_show_loglevel(
    CLog* _log, int _show)
{
    if(_log == NULL)
        return -1;
    pthread_mutex_lock(&_log->m_lock);
    _log->m_show_loglevel = _show;
    pthread_mutex_unlock(&_log->m_lock);
    return 0;
}

int clog_show_position(
    CLog* _log, int _show)
{
    if(_log == NULL)
        return -1;
    pthread_mutex_lock(&_log->m_lock);
    _log->m_show_position = _show;
    pthread_mutex_unlock(&_log->m_lock);
    return 0;
}

int clog_show_function(
    CLog* _log, int _show)
{
    if(_log == NULL)
        return -1;
    pthread_mutex_lock(&_log->m_lock);
    _log->m_show_function = _show;
    pthread_mutex_unlock(&_log->m_lock);
    return 0;
}

int clog_show_stdout(CLog *_log, int _show)
{
    if(_log == NULL)
        return -1;
    pthread_mutex_lock(&_log->m_lock);
    _log->m_show_stdout = _show;
    pthread_mutex_unlock(&_log->m_lock);
    return 0;
}

int clog_set_logname(
    CLog* _log, const char* _name)
{
    if(_log == NULL || _name == NULL
        || _name[0] == 0)
        return -1;
    size_t length = strlen(_name);
    char *name_buf =
        (char*)malloc(length + 1);
    if(name_buf == NULL)
        return -2;
    strcpy(name_buf, _name);
    pthread_mutex_lock(&_log->m_lock);
    if(_log->m_log_name)
        free(_log->m_log_name);
    _log->m_log_name = name_buf;
    _log->m_name_len = length;
    if(_log->m_log_path)
    {
        free(_log->m_log_path);
        _log->m_log_path = NULL;
    }
    pthread_mutex_unlock(&_log->m_lock);
    return 0;
}

int clog_set_logdir(
    CLog* _log, const char* _dir)
{
    if(_log == NULL || _dir == NULL
        || _dir[0] == 0)
        return -1;
    size_t length = strlen(_dir);
    char* dir_buf = (char*)malloc(length + 4);
    if(dir_buf == NULL)
        return -2;
    strcpy(dir_buf, _dir);
    if(dir_buf[length - 1] != '/' &&
        dir_buf[length - 1] != '\\')
    {
        dir_buf[length++] = '/';
        dir_buf[length] = 0;
    }
    if(access(dir_buf, F_OK))
    {
        free(dir_buf);
        return -3;
    }
    pthread_mutex_lock(&_log->m_lock);
    if(_log->m_log_dir)
        free(_log->m_log_dir);
    _log->m_log_dir = dir_buf;
    _log->m_dir_len = length;
    if(_log->m_log_path)
    {
        free(_log->m_log_path);
        _log->m_log_path = NULL;
    }
    pthread_mutex_unlock(&_log->m_lock);
    return 0;
}

int clog_set_logdir_envvar(
    CLog* _log, const char* _envvar)
{
    if(_envvar == NULL)
        return -1;
    return clog_set_logdir(_log, getenv(_envvar));
}

int clog_set_loglevel(
    CLog *_log, int _level)
{
    if(_log == NULL)
        return -1;
    if(_level < CLOG_LEVEL_DEBUG ||
        _level > CLOG_LEVEL_ERROR)
        return -2;
    pthread_mutex_lock(&_log->m_lock);
    _log->m_log_level = _level;
    pthread_mutex_unlock(&_log->m_lock);
    return 0;
}

int clog_get_logsize(CLog * _log)
{
    struct stat file_state = {0};
    if(_log == NULL)
        return file_state.st_size;
    do {
        pthread_mutex_lock(&_log->m_lock);
        if(_log->m_log_path == NULL)
            break;
        int fd = open(
            _log->m_log_path, O_RDONLY);
        if(fd < 0)
            break;
        fstat(fd, &file_state);
        close(fd);
    }while(0);
    pthread_mutex_unlock(&_log->m_lock);
    return file_state.st_size;
}

int clog_clear(CLog * _log)
{
    int result = 0;
    if(_log == NULL)
        return result;
    do {
        pthread_mutex_lock(&_log->m_lock);
        if(_log->m_log_path == NULL)
            break;
        int fd = open(
            _log->m_log_path,
            O_TRUNC | O_WRONLY);
        if(fd < 0)
            break;
        close(fd);
    }while(0);
    pthread_mutex_unlock(&_log->m_lock);
    return result;
}

int _clog_error(
    CLog* _log, const char* _file, int _line,
    const char* _func, const char* _fmt, ...)
{
    int result = 0;
    if(_log == NULL || _file == NULL || _line <= 0
        || _func == NULL || _fmt==NULL)
        return result;
    do {
        pthread_mutex_lock(&_log->m_lock);
        if(_log->m_log_level > CLOG_LEVEL_ERROR)
            break;
        va_list args;
        va_start(args, _fmt);
        result = vsnprintf(
            _log->m_msgbuf,
            _log->m_bufsz, _fmt, args);
        va_end(args);
        if(result < 0)
            break;
        _log->m_msgbuf[result++] = '\n';
        _log->m_msgbuf[result] = 0;
        CLogHead header = {0};
        header.file = _file;
        header.line = _line;
        header.func = _func;
        header.level = CLOG_LEVEL_ERROR;
        result = clog_write(
            _log, &header,
            _log->m_msgbuf, result);
    }while(0);
    pthread_mutex_unlock(&_log->m_lock);
    return result;
}

int _clog_warn(
    CLog* _log, const char* _file, int _line,
    const char* _func, const char* _fmt, ...)
{
    int result = 0;
    if(_log == NULL || _file == NULL || _line <= 0
        || _func == NULL || _fmt==NULL)
        return result;
    do {
        pthread_mutex_lock(&_log->m_lock);
        if(_log->m_log_level > CLOG_LEVEL_WARN)
            break;
        va_list args;
        va_start(args, _fmt);
        result = vsnprintf(
            _log->m_msgbuf,
            _log->m_bufsz, _fmt, args);
        va_end(args);
        if(result < 0)
            break;
        _log->m_msgbuf[result++] = '\n';
        _log->m_msgbuf[result] = 0;
        CLogHead header = {0};
        header.file = _file;
        header.line = _line;
        header.func = _func;
        header.level = CLOG_LEVEL_WARN;
        result = clog_write(
            _log, &header,
            _log->m_msgbuf, result);
    }while(0);
    pthread_mutex_unlock(&_log->m_lock);
    return result;
}

int _clog_info(
    CLog* _log, const char* _file, int _line,
    const char* _func, const char* _fmt, ...)
{
    int result = 0;
    if(_log == NULL || _file == NULL || _line <= 0
        || _func == NULL || _fmt==NULL)
        return result;
    do {
        pthread_mutex_lock(&_log->m_lock);
        if(_log->m_log_level > CLOG_LEVEL_INFO)
            break;
        va_list args;
        va_start(args, _fmt);
        result = vsnprintf(
            _log->m_msgbuf,
            _log->m_bufsz, _fmt, args);
        va_end(args);
        if(result < 0)
            break;
        _log->m_msgbuf[result++] = '\n';
        _log->m_msgbuf[result] = 0;
        CLogHead header = {0};
        header.file = _file;
        header.line = _line;
        header.func = _func;
        header.level = CLOG_LEVEL_INFO;
        result = clog_write(
            _log, &header,
            _log->m_msgbuf, result);
    }while(0);
    pthread_mutex_unlock(&_log->m_lock);
    return result;
}

int _clog_debug(
    CLog* _log, const char* _file, int _line,
    const char* _func, const char* _fmt, ...)
{
    int result = 0;
    if(_log == NULL || _file == NULL || _line <= 0
        || _func == NULL || _fmt==NULL)
        return result;
    do {
        pthread_mutex_lock(&_log->m_lock);
        if(_log->m_log_level > CLOG_LEVEL_DEBUG)
            break;
        va_list args;
        va_start(args, _fmt);
        result = vsnprintf(
            _log->m_msgbuf,
            _log->m_bufsz, _fmt, args);
        va_end(args);
        if(result < 0)
            break;
        _log->m_msgbuf[result++] = '\n';
        _log->m_msgbuf[result] = 0;
        CLogHead header = {0};
        header.file = _file;
        header.line = _line;
        header.func = _func;
        header.level = CLOG_LEVEL_DEBUG;
        result = clog_write(
            _log, &header,
            _log->m_msgbuf, result);
    }while(0);
    pthread_mutex_unlock(&_log->m_lock);
    return result;
}


