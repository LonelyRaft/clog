
#ifndef CLOG_H
#define CLOG_H

#define CLOG_LEVEL_ERROR 0x04
#define CLOG_LEVEL_WARN 0x03
#define CLOG_LEVEL_INFO 0x02
#define CLOG_LEVEL_DEBUG 0x01

typedef struct CLog CLog;

/************************************
 * create a CLog object
 * @_msgsz_max: max length of a piece of message
 * @return: a CLog object pointer or NULL
 ************************************/
CLog *clog_create(unsigned int _msgsz_max);

/************************************
 * free a CLog object
 * @_log: a CLog object pointer created by clog_create
 ************************************/
void clog_desrtroy(CLog* _log);

/************************************
 * show date and time in log
 * @_log: a CLog object pointer
 * @_show:  0 is not
 * @return: 0 is success
 ************************************/
int clog_show_datetime(
    CLog* _log, int _show);

/************************************
 * show log level in log
 * @_log: a CLog object pointer
 * @_show:  0 is not
 * @return: 0 is success
 ************************************/
int clog_show_loglevel(
    CLog* _log, int _show);

/************************************
 * show position in log
 * @_log: a CLog object pointer
 * @_show:  0 is not
 * @return: 0 is success
 ************************************/
int clog_show_postion(
    CLog* _log, int _show);

/************************************
 * show function in log
 * @_log: a CLog object pointer
 * @_show:  0 is not
 * @return: 0 is success
 ************************************/
int clog_show_function(
    CLog* _log, int _show);

/************************************
 * print logs on stdout at the same time
 * @_log: a CLog object pointer
 * @_show:  0 is not
 * @return: 0 is success
 ************************************/
int clog_show_stdout(
    CLog* _log, int _show);

/************************************
 * set name for log file
 * @_log: a CLog object pointer
 * @_name:  new file name
 * @return: 0 is success
 ************************************/
int clog_set_logname(
    CLog* _log, const char* _name);

/************************************
 * set path for log file
 * @_log: a CLog object pointer
 * @_dire:  new file directory
 * @return: 0 is success
 ************************************/
int clog_set_logdir(
    CLog* _log, const char* _dire);

/************************************
 * set level for log
 * @_log: a CLog object pointer
 * @_level:  log level
 * @return: 0 is success
 ************************************/
int clog_set_loglevel(
    CLog* _log, int _level);

/************************************
 * get size of all current logs
 * @_log: a CLog object pointer
 * @return: size of all current logs
 ************************************/
int clog_get_logsize(CLog * _log);

/************************************
 * clear all current logs
 * @_log: a CLog object pointer
 * @return: 0 is sucess
 ************************************/
int clog_clear(CLog * _log);

int _clog_error(CLog* _log,
    const char* _file, int _line,
    const char* _func,
    const char* _fmt, ...);

int _clog_warn(CLog* _log,
    const char* _file, int _line,
    const char* _func,
    const char* _fmt, ...);

int _clog_info(CLog* _log,
               const char* _file, int _line,
               const char* _func,
               const char* _fmt, ...);

int _clog_debug(CLog* _log,
    const char* _file, int _line,
    const char* _func,
    const char* _fmt, ...);

/************************************
 * output error message
 * @log:    CLog object pointer
 * @fmt:    message format
 * @...:    message data
 * return:   message length
*************************************/
#define clog_error(log, fmt, ...) _clog_error(\
log, __FILE__, __LINE__, __FUNCTION__, \
fmt, __VA_ARGS__)

/************************************
 * output warning message
 * @log:    CLog object pointer
 * @fmt:    message format
 * @...:    message data
 * @return:  message length
*************************************/
#define clog_warn(log, fmt, ...) _clog_warn(\
log, __FILE__, __LINE__, __FUNCTION__, \
fmt, __VA_ARGS__)

/************************************
 * output information message
 * @log:    CLog object pointer
 * @fmt:    message format
 * @...:    message data
 * @return:  message length
*************************************/
#define clog_info(log, fmt, ...) _clog_info(\
log, __FILE__, __LINE__, __FUNCTION__, \
fmt, __VA_ARGS__)

/************************************
 * output debug message
 * @log:    CLog object pointer
 * @fmt:    message format
 * @...:    message data
 * @return:  message length
*************************************/
#define clog_debug(log, fmt, ...) _clog_debug(\
log, __FILE__, __LINE__, __FUNCTION__, \
fmt, __VA_ARGS__)

#endif // CLOG_H
