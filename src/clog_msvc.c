
#define _CRT_SECURE_NO_WARNINGS

#include "clog.h"
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

static const char *stdfmt[] = {
    "\033[0m",     // clear format
    "\033[1;34m",  // bold and blue
    "\033[1;32m",  // bold and green
    "\033[1;33m",  // bold and yellow
    "\033[1;31m",  // bold and red
    "\033[1;35m",  // bold and purple
};

typedef struct clog_config_t
{
    unsigned int msgsize;
    unsigned char b_datetime;
    unsigned char b_level;
    unsigned char b_function;
    unsigned char b_position;
    unsigned char b_stdout;
    unsigned char b_name;
    unsigned char level;
    unsigned int name_len;
    unsigned int dir_len;
    char *directory;
    char *name;
} clog_config_t;

typedef struct clog_t
{
    clog_config_t config;
    char *m_msgbuf;
    char *m_path;
    HANDLE m_lock;
} clog_t;

typedef struct clog_head_t
{
    const char *file;
    const char *func;
    size_t line;
    unsigned char level;
} clog_head_t;

static int clog_datetime(char *_datetime)
{
    time_t seconds = time(0);
    struct tm curr = {0};
    localtime_s(&curr, &seconds);
    int result = snprintf(
            _datetime, 24,
            "[%04d-%02d-%02d %02d:%02d:%02d] ",
            curr.tm_year + 1900, curr.tm_mon + 1, curr.tm_mday,
            curr.tm_hour, curr.tm_min, curr.tm_sec);
    return result;
}

static int clog_level2str(char *_log_level, int _level)
{
    if (_log_level == NULL) {
        return 0;
    }
    switch (_level) {
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
    return (int)strlen(_log_level);
}

static const char *clog_gen_path(clog_t *_log)
{
    if (_log == NULL) {
        return NULL;
    }
    clog_config_t *config = (clog_config_t *)_log;
    if (config->name == NULL) {
        config->name = (char *)malloc(64);
        if (config->name == NULL) {
            return NULL;
        }
        srand((unsigned int)time(NULL));
        int result = snprintf(
                config->name, 63,
                "clog%d", rand() % 1001);
        if (result < 0) {
            free(config->name);
            config->name = NULL;
            config->name_len = 0;
            return NULL;
        }
        config->name_len = result;
    }
    if (config->directory == NULL) {
#ifdef _WIN32
        const char *dir = "./";
#endif  // _WIN32
#ifdef __linux__
        const char *dir = "/tmp/";
#endif  // __linux__
        config->dir_len = (unsigned int)strlen(dir);
        config->directory =
            (char *)malloc(config->dir_len + 4);
        if (config->directory == NULL) {
            config->dir_len = 0;
            return NULL;
        }
        strcpy(config->directory, dir);
    }
    if (_log->m_path == NULL) {
        time_t secs = time(NULL);
        struct tm curr = {0};
        char suffix[32] = {0};
        localtime_s(&curr, &secs);
        int length = snprintf(
                suffix, 32, "_%04d%02d%02d.log",
                curr.tm_year + 1900,
                curr.tm_mon + 1, curr.tm_mday);
        length += config->dir_len;
        length += config->name_len;
        _log->m_path =
            (char *)malloc(length + 1);
        if (_log->m_path == NULL) {
            return NULL;
        }
        char *path = _log->m_path;
        strcpy(path, config->directory);
        path = path + config->dir_len;
        strcpy(path, config->name);
        path = path + config->name_len;
        strcpy(path, suffix);
    }
    return _log->m_path;
}

static int clog_write(
    clog_t *_log, const clog_head_t *_head,
    const char *_message, int _length)
{
    int result = 0, idx = 0, length = 0;
    char header[512] = {0};
    const clog_config_t *config =
        (clog_config_t *)_log;
    clog_gen_path(_log);
    if (config->b_datetime) {
        length = clog_datetime(header + idx);
        if (length > 0) {
            idx += length;
        }
    }
    if (config->b_level) {
        length = clog_level2str(
                header + idx, _head->level);
        if (length > 0) {
            idx += length;
        }
    }
    if (config->b_name && config->name) {
        // name is not null
        header[idx++] = '[';
        length = 0;
        while (config->name[length]) {
            header[idx++] =
                config->name[length++];
        }
        header[idx++] = ']';
        header[idx++] = ' ';
    }
    if (config->b_function) {
        header[idx++] = '[';
        length = 0;
        while (_head->func[length]) {
            header[idx++] =
                _head->func[length++];
        }
        header[idx++] = ']';
        header[idx++] = ' ';
    }
    if (config->b_position) {
        length = snprintf(
                header + idx, 511 - idx, "[%s:%llu]",
                _head->file, _head->line);
        if (length > 0) {
            idx += length;
        }
    }
    if (idx > 0) {
        header[idx++] = '\n';
        header[idx] = 0;
    }
    do {
        const char *path =
            clog_gen_path(_log);
        if (path == NULL) {
            break;
        }
        int fd = _open(
                path,
                O_CREAT | O_BINARY | O_WRONLY | O_APPEND,
                S_IWRITE | S_IREAD);
        if (fd < 0) {
            break;
        }
        _write(fd, header, idx);
        _write(fd, _message, _length);
        _close(fd);
        result = idx + _length;
    } while (0);
    if (config->b_stdout) {
        if (idx > 0) {
            fprintf(stderr, "%s%s%s%s",
                stdfmt[_head->level],
                header, stdfmt[0], _message);
        } else {
            fprintf(stderr, "%s%s%s",
                stdfmt[_head->level],
                _message, stdfmt[0]);
        }
    }
    return result;
}

static int clog_readline(int _fd, char *_buff, int size)
{
    int idx = 0;
    char ret = 0;
    char tmp = '\n';
    if (_buff == NULL || size < 1) {
        return 0;
    }
    do {
        ret = _read(_fd, &tmp, 1);
        if (0 > ret) {
            return ret;
        }
        _buff[idx++] = tmp;
        if (idx == size) {
            return idx;
        }
    } while (tmp != '\n');
    return idx;
}

clog_t *clog_create(unsigned int _msgsz_max)
{
    clog_t *log = NULL;
    unsigned int size =
        UINT_MAX - sizeof(clog_t) - 1;
    if (_msgsz_max == 0 ||
        _msgsz_max > size) {
        return log;
    }
    size = _msgsz_max + sizeof(clog_t) + 1;
    char *addr = (char *)malloc(size);
    if (addr == NULL) {
        return log;
    }
    memset(addr, 0, size);
    log = (clog_t *)addr;
    addr = addr + sizeof(clog_t);
    log->m_msgbuf = addr;
    log->config.b_datetime = 1;
    log->config.b_level = 1;
    log->config.level = CLOG_LEVEL_WARN;
    log->config.msgsize = _msgsz_max;
    log->m_lock = CreateMutex(NULL, FALSE, NULL);
    if (log->m_lock == NULL) {
        free(addr);
        addr  = NULL;
        log = NULL;
    }
    return log;
}

clog_t *clog_read_cfg(const char *_cfgpath)
{
    if (_cfgpath == NULL ||
        _cfgpath[0] == 0) {
        return NULL;
    }
    int fd = _open(_cfgpath, O_RDONLY);
    if (fd < 0) {
        return NULL;
    }
    char buffer[512];
    int length = 0;
    unsigned char start = 0;
    unsigned char comment = 0;
    clog_config_t config = {
        1024, 1, 1,    0,   0, 0, 0,
        CLOG_LEVEL_WARN,
        0,    0, NULL, NULL
    };
    do {
        length = clog_readline(fd, buffer, 511);
        if (length < 2) {
            break;
        }
        if (buffer[0] == '#') {
            if (buffer[length - 1] != '\n') {
                comment = 1;
            }
            continue;
        }
        if (comment) {
            if (buffer[length - 1] == '\n') {
                comment = 0;
            }
            continue;
        }
        if (length >= 6 &&
            strncmp(buffer, "[clog]", 6) == 0) {
            start = 1;
            continue;
        }
        if (start == 0) {
            continue;
        }
        if (buffer[0] == '[') {
            break;
        }
        buffer[length] = 0;
        char *data = strchr(buffer, '#');
        if (data != NULL) {
            if (buffer[length - 1] != '\n') {
                comment = 1;
            }
            length = (int)(data - buffer);
            *data = 0;
        }
        data = strchr(buffer, '=');
        if (data == NULL) {
            continue;
        }
        data++;
        if (length > 8 && strncmp(buffer, "msgsize", 7) == 0) {
            while ((*data) && !isdigit(*data)) {
                data++;
            }
            if ((*data) == 0) {
                continue;
            }
            const char *p = data;
            while ((*data) && isdigit(*data)) {
                data++;
            }
            *(data) = 0;
            config.msgsize = atoi(p);
        } else if (length > 13 && strncmp(buffer, "use_datetime", 12) == 0) {
            const char *p = strstr(data, "true");
            if (p != NULL) {
                config.b_datetime = 1;
                continue;
            }
            p = strstr(data, "false");
            if (p != NULL) {
                config.b_datetime = 0;
            }
        } else if (length > 10 && strncmp(buffer, "use_level", 9) == 0) {
            const char *p = strstr(data, "true");
            if (p != NULL) {
                config.b_level = 1;
                continue;
            }
            p = strstr(data, "false");
            if (p != NULL) {
                config.b_level = 0;
            }
        } else if (length > 13 && strncmp(buffer, "use_function", 12) == 0) {
            const char *p = strstr(data, "true");
            if (p != NULL) {
                config.b_function = 1;
                continue;
            }
            p = strstr(data, "false");
            if (p != NULL) {
                config.b_function = 0;
            }
        } else if (length > 13 && strncmp(buffer, "use_position", 12) == 0) {
            const char *p = strstr(data, "true");
            if (p != NULL) {
                config.b_position = 1;
                continue;
            }
            p = strstr(data, "false");
            if (p != NULL) {
                config.b_position = 0;
            }
        } else if (length > 11 && strncmp(buffer, "use_stdout", 10) == 0) {
            const char *p = strstr(data, "true");
            if (p != NULL) {
                config.b_stdout = 1;
                continue;
            }
            p = strstr(data, "false");
            if (p != NULL) {
                config.b_stdout = 0;
            }
        } else if (length > 9 && strncmp(buffer, "use_name", 8) == 0) {
            const char *p = strstr(data, "true");
            if (p != NULL) {
                config.b_name = 1;
                continue;
            }
            p = strstr(data, "false");
            if (p != NULL) {
                config.b_name = 0;
            }
        } else if (length > 6 && strncmp(buffer, "level", 5) == 0) {
            const char *p = strstr(data, "DEBUG");
            if (p != NULL) {
                config.level = CLOG_LEVEL_DEBUG;
                continue;
            }
            p = strstr(data, "INFO");
            if (p != NULL) {
                config.level = CLOG_LEVEL_INFO;
                continue;
            }
            p = strstr(data, "WARN");
            if (p != NULL) {
                config.level = CLOG_LEVEL_WARN;
                continue;
            }
            p = strstr(data, "ERROR");
            if (p != NULL) {
                config.level = CLOG_LEVEL_ERROR;
                continue;
            }
            p = strstr(data, "CLOSE");
            if (p != NULL) {
                config.level = CLOG_LEVEL_CLOSE;
            }
        } else if (length > 4 && strncmp(buffer, "dir", 3) == 0) {
            if (config.directory != NULL) {
                continue;
            }
            const char *dir = data;
            while ((*data) && !isspace(*data)) {
                data++;
            }
            *data = 0;
            int dirlen = (int)strlen(dir);
            if (dirlen <= 0) {
                continue;
            }
            config.directory = (char *)malloc(dirlen + 4);
            if (config.directory == NULL) {
                continue;
            }
            strcpy(config.directory, dir);
            if (config.directory[dirlen - 1] != '/' &&
                config.directory[dirlen - 1] != '\\') {
                config.directory[dirlen++] = '/';
                config.directory[dirlen] = 0;
            }
            config.dir_len = dirlen;
        } else if (length > 5 && strncmp(buffer, "name", 4) == 0) {
            if (config.name != NULL) {
                continue;
            }
            const char *name = data;
            while ((*data) && !isspace(*data)) {
                data++;
            }
            *data = 0;
            int nmlen = (int)strlen(name);
            if (nmlen <= 0) {
                continue;
            }
            config.name = (char *)malloc(nmlen + 1);
            if (config.name == NULL) {
                continue;
            }
            strcpy(config.name, name);
            config.name_len = nmlen;
        }
    } while (1);
    _close(fd);
    if (config.msgsize <= 0) {
        config.msgsize = 1024;
    }
    clog_t *log = (clog_t *)clog_create(config.msgsize);
    if (log == NULL) {
        if (config.name != NULL) {
            free(config.name);
        }
        if (config.directory != NULL) {
            free(config.directory);
        }
        config.directory = config.name = NULL;
        return log;
    }
    log->config = config;
    return log;
}

void clog_desrtroy(clog_t *_log)
{
    if (_log == NULL) {
        return;
    }
    clog_config_t *config =
        (clog_config_t *)_log;
    if (config->directory) {
        free(config->directory);
    }
    if (config->name) {
        free(config->name);
    }
    config->directory =
        config->name = NULL;
    if (_log->m_path) {
        free(_log->m_path);
    }
    _log->m_path = NULL;
    if (_log->m_lock) {
        CloseHandle(_log->m_lock);
    }
    _log->m_lock = NULL;
    free(_log);
    _log = NULL;
}

int clog_use_datetime(clog_t *_log, int _show)
{
    if (_log == NULL) {
        return -1;
    }
    WaitForSingleObject(_log->m_lock, INFINITE);
    _log->config.b_datetime = _show;
    ReleaseMutex(_log->m_lock);
    return 0;
}

int clog_use_level(clog_t *_log, int _show)
{
    if (_log == NULL) {
        return -1;
    }
    WaitForSingleObject(_log->m_lock, INFINITE);
    _log->config.b_level = _show;
    ReleaseMutex(_log->m_lock);
    return 0;
}

int clog_use_position(clog_t *_log, int _show)
{
    if (_log == NULL) {
        return -1;
    }
    WaitForSingleObject(_log->m_lock, INFINITE);
    _log->config.b_position = _show;
    ReleaseMutex(_log->m_lock);
    return 0;
}

int clog_use_function(clog_t *_log, int _show)
{
    if (_log == NULL) {
        return -1;
    }
    WaitForSingleObject(_log->m_lock, INFINITE);
    _log->config.b_function = _show;
    ReleaseMutex(_log->m_lock);
    return 0;
}

int clog_use_stdout(clog_t *_log, int _show)
{
    if (_log == NULL) {
        return -1;
    }
    WaitForSingleObject(_log->m_lock, INFINITE);
    _log->config.b_stdout = _show;
    ReleaseMutex(_log->m_lock);
    return 0;
}

int clog_use_name(clog_t *_log, int _show)
{
    if (_log == NULL) {
        return -1;
    }
    WaitForSingleObject(_log->m_lock, INFINITE);
    _log->config.b_name = _show;
    ReleaseMutex(_log->m_lock);
    return 0;
}

int clog_set_name(clog_t *_log, const char *_name)
{
    if (_log == NULL ||
        _name == NULL ||
        _name[0] == 0) {
        return -1;
    }
    int length = (int)strlen(_name);
    char *name_buf = (char *)malloc(length + 1);
    if (name_buf == NULL) {
        return -2;
    }
    strcpy(name_buf, _name);
    WaitForSingleObject(_log->m_lock, INFINITE);
    if (_log->config.name) {
        free(_log->config.name);
    }
    _log->config.name = name_buf;
    _log->config.name_len = length;
    if (_log->m_path) {
        free(_log->m_path);
    }
    _log->m_path = NULL;
    ReleaseMutex(_log->m_lock);
    return 0;
}

int clog_set_dir(clog_t *_log, const char *_dir)
{
    if (_log == NULL ||
        _dir == NULL || _dir[0] == 0) {
        return -1;
    }
    int length = (int)strlen(_dir);
    char *dir_buf = (char *)malloc(length + 4);
    if (dir_buf == NULL) {
        return -2;
    }
    strcpy(dir_buf, _dir);
    if (dir_buf[length - 1] != '/' &&
        dir_buf[length - 1] != '\\') {
        dir_buf[length++] = '/';
        dir_buf[length] = 0;
    }
    if (_access(dir_buf, 0)) {
        free(dir_buf);
        dir_buf = NULL;
        return -3;
    }
    WaitForSingleObject(_log->m_lock, INFINITE);
    if (_log->config.directory) {
        free(_log->config.directory);
    }
    _log->config.directory = dir_buf;
    _log->config.dir_len = length;
    if (_log->m_path) {
        free(_log->m_path);
    }
    _log->m_path = NULL;
    ReleaseMutex(_log->m_lock);
    return 0;
}

int clog_set_dir_envvar(clog_t *_log, const char *_envvar)
{
    if (_envvar == NULL) {
        return -1;
    }
    return clog_set_dir(_log, getenv(_envvar));
}

int clog_set_level(clog_t *_log, int _level)
{
    if (_log == NULL) {
        return -1;
    }
    if (_level < CLOG_LEVEL_DEBUG ||
        _level > CLOG_LEVEL_ERROR) {
        return -2;
    }
    WaitForSingleObject(_log->m_lock, INFINITE);
    _log->config.level = _level;
    ReleaseMutex(_log->m_lock);
    return 0;
}

int clog_get_size(clog_t *_log)
{
    struct stat file_state = {0};
    if (_log == NULL) {
        return file_state.st_size;
    }
    do {
        WaitForSingleObject(_log->m_lock, INFINITE);
        if (_log->m_path == NULL) {
            break;
        }
        int fd = _open(
                _log->m_path, O_RDONLY);
        if (fd < 0) {
            break;
        }
        fstat(fd, &file_state);
        _close(fd);
    } while (0);
    ReleaseMutex(_log->m_lock);
    return file_state.st_size;
}

int clog_clear(clog_t *_log)
{
    int result = 0;
    if (_log == NULL) {
        return result;
    }
    do {
        WaitForSingleObject(_log->m_lock, INFINITE);
        clog_gen_path(_log);
        if (_log->m_path == NULL) {
            break;
        }
        int fd = _open(
                _log->m_path,
                O_TRUNC | O_WRONLY);
        if (fd < 0) {
            break;
        }
        _close(fd);
    } while (0);
    ReleaseMutex(_log->m_lock);
    return result;
}

int _clog_error(
    clog_t *_log, const char *_file, size_t _line,
    const char *_func, const char *_fmt, ...)
{
    int result = 0;
    if (_log == NULL || _file == NULL ||
        _func == NULL || _fmt == NULL) {
        return result;
    }
    do {
        WaitForSingleObject(_log->m_lock, INFINITE);
        if (_log->config.level > CLOG_LEVEL_ERROR) {
            break;
        }
        va_list args;
        va_start(args, _fmt);
        result = vsnprintf(
                _log->m_msgbuf,
                _log->config.msgsize,
                _fmt, args);
        va_end(args);
        if (result < 0) {
            break;
        }
        _log->m_msgbuf[result++] = '\n';
        _log->m_msgbuf[result] = 0;
        clog_head_t header = {0};
        header.file = _file;
        header.line = _line;
        header.func = _func;
        header.level = CLOG_LEVEL_ERROR;
        result = clog_write(
                _log, &header,
                _log->m_msgbuf, result);
    } while (0);
    ReleaseMutex(_log->m_lock);
    return result;
}

int _clog_warn(
    clog_t *_log, const char *_file, size_t _line,
    const char *_func, const char *_fmt, ...)
{
    int result = 0;
    if (_log == NULL || _file == NULL ||
        _func == NULL || _fmt == NULL) {
        return result;
    }
    do {
        WaitForSingleObject(_log->m_lock, INFINITE);
        if (_log->config.level > CLOG_LEVEL_WARN) {
            break;
        }
        va_list args;
        va_start(args, _fmt);
        result = vsnprintf(
                _log->m_msgbuf,
                _log->config.msgsize,
                _fmt, args);
        va_end(args);
        if (result < 0) {
            break;
        }
        _log->m_msgbuf[result++] = '\n';
        _log->m_msgbuf[result] = 0;
        clog_head_t header = {0};
        header.file = _file;
        header.line = _line;
        header.func = _func;
        header.level = CLOG_LEVEL_WARN;
        result = clog_write(
                _log, &header,
                _log->m_msgbuf, result);
    } while (0);
    ReleaseMutex(_log->m_lock);
    return result;
}

int _clog_info(
    clog_t *_log, const char *_file, size_t _line,
    const char *_func, const char *_fmt, ...)
{
    int result = 0;
    if (_log == NULL || _file == NULL ||
        _func == NULL || _fmt == NULL) {
        return result;
    }
    do {
        WaitForSingleObject(_log->m_lock, INFINITE);
        if (_log->config.level > CLOG_LEVEL_INFO) {
            break;
        }
        va_list args;
        va_start(args, _fmt);
        result = vsnprintf(
                _log->m_msgbuf,
                _log->config.msgsize,
                _fmt, args);
        va_end(args);
        if (result < 0) {
            break;
        }
        _log->m_msgbuf[result++] = '\n';
        _log->m_msgbuf[result] = 0;
        clog_head_t header = {0};
        header.file = _file;
        header.line = _line;
        header.func = _func;
        header.level = CLOG_LEVEL_INFO;
        result = clog_write(
                _log, &header,
                _log->m_msgbuf, result);
    } while (0);
    ReleaseMutex(_log->m_lock);
    return result;
}

int _clog_debug(
    clog_t *_log, const char *_file, size_t _line,
    const char *_func, const char *_fmt, ...)
{
    int result = 0;
    if (_log == NULL || _file == NULL ||
        _func == NULL || _fmt == NULL) {
        return result;
    }
    do {
        WaitForSingleObject(_log->m_lock, INFINITE);
        if (_log->config.level > CLOG_LEVEL_DEBUG) {
            break;
        }
        va_list args;
        va_start(args, _fmt);
        result = vsnprintf(
                _log->m_msgbuf,
                _log->config.msgsize,
                _fmt, args);
        va_end(args);
        if (result < 0) {
            break;
        }
        _log->m_msgbuf[result++] = '\n';
        _log->m_msgbuf[result] = 0;
        clog_head_t header = {0};
        header.file = _file;
        header.line = _line;
        header.func = _func;
        header.level = CLOG_LEVEL_DEBUG;
        result = clog_write(
                _log, &header,
                _log->m_msgbuf, result);
    } while (0);
    ReleaseMutex(_log->m_lock);
    return result;
}

int _clog_status(clog_t *_log, const char *_fmt, ...)
{
    int result = 0;
    if (_log == NULL || _fmt == NULL) {
        return 0;
    }
    do {
        WaitForSingleObject(_log->m_lock, INFINITE);
        va_list args;
        va_start(args, _fmt);
        result = vsnprintf(
                _log->m_msgbuf,
                _log->config.msgsize,
                _fmt, args);
        va_end(args);
        if (result < 0) {
            break;
        }
        _log->m_msgbuf[result++] = '\n';
        _log->m_msgbuf[result] = 0;
        fprintf(
            stderr, "%s%s%s",
            stdfmt[CLOG_LEVEL_CLOSE],
            _log->m_msgbuf, stdfmt[0]);
    } while (0);
    ReleaseMutex(_log->m_lock);
    return result;
}
