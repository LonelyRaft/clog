
#ifndef CLOG_H
#define CLOG_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

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
     * create a CLog object by config file
     * @_cfgpath: config file path
     * @return: a CLog object pointer or NULL
     ************************************/
    CLog *clog_read_cfg(
        const char *_cfgpath);

    /************************************
     * free a CLog object
     * @_log: a CLog object pointer created by clog_create
     ************************************/
    void clog_desrtroy(CLog *_log);

    /************************************
     * show date and time in log
     * @_log: a CLog object pointer
     * @_show:  0 is not
     * @return: 0 is success
     ************************************/
    int clog_use_datetime(
        CLog *_log, int _show);

    /************************************
     * show log level in log
     * @_log: a CLog object pointer
     * @_show:  0 is not
     * @return: 0 is success
     ************************************/
    int clog_use_level(
        CLog *_log, int _show);

    /************************************
     * show position in log
     * @_log: a CLog object pointer
     * @_show:  0 is not
     * @return: 0 is success
     ************************************/
    int clog_use_position(
        CLog *_log, int _show);

    /************************************
     * show function in log
     * @_log: a CLog object pointer
     * @_show:  0 is not
     * @return: 0 is success
     ************************************/
    int clog_use_function(
        CLog *_log, int _show);

    /************************************
     * print logs on stdout at the same time
     * @_log: a CLog object pointer
     * @_show:  0 is not
     * @return: 0 is success
     ************************************/
    int clog_use_stdout(
        CLog *_log, int _show);

    /************************************
     * show log name in log
     * @_log: a CLog object pointer
     * @_show:  0 is not
     * @return: 0 is success
     ************************************/
    int clog_use_name(
        CLog *_log, int _show);

    /************************************
     * set name for log file
     * @_log: a CLog object pointer
     * @_name:  new file name
     * @return: 0 is success
     ************************************/
    int clog_set_name(
        CLog *_log, const char *_name);

    /************************************
     * set path for log file
     * @_log: a CLog object pointer
     * @_dire:  new file directory
     * @return: 0 is success
     ************************************/
    int clog_set_dir(
        CLog *_log, const char *_dire);

    /************************************
     * set path for log file with env var
     * @_log: a CLog object pointer
     * @_envvar:  env var name
     * @return: 0 is success
     ************************************/
    int clog_set_dir_envvar(
        CLog *_log, const char *_envvar);

    /************************************
     * set level for log
     * @_log: a CLog object pointer
     * @_level:  log level
     * @return: 0 is success
     ************************************/
    int clog_set_level(
        CLog *_log, int _level);

    /************************************
     * get size of all current logs
     * @_log: a CLog object pointer
     * @return: size of all current logs
     ************************************/
    int clog_get_size(CLog *_log);

    /************************************
     * clear all current logs
     * @_log: a CLog object pointer
     * @return: 0 is sucess
     ************************************/
    int clog_clear(CLog *_log);

    int _clog_error(
        CLog *_log, const char *_file, int _line,
        const char *_func, const char *_fmt, ...);

    int _clog_warn(
        CLog *_log, const char *_file, int _line,
        const char *_func, const char *_fmt, ...);

    int _clog_info(
        CLog *_log, const char *_file, int _line,
        const char *_func, const char *_fmt, ...);

    int _clog_debug(
        CLog *_log, const char *_file, int _line,
        const char *_func, const char *_fmt, ...);

/************************************
 * output error message
 * @log:    CLog object pointer
 * @fmt:    message format
 * @...:    message data
 * return:   message length
 *************************************/
#define clog_error(log, fmt, ...) _clog_error( \
    log, __FILE__, __LINE__, __FUNCTION__,     \
    fmt, ##__VA_ARGS__)

/************************************
 * output warning message
 * @log:    CLog object pointer
 * @fmt:    message format
 * @...:    message data
 * @return:  message length
 *************************************/
#define clog_warn(log, fmt, ...) _clog_warn( \
    log, __FILE__, __LINE__, __FUNCTION__,   \
    fmt, ##__VA_ARGS__)

/************************************
 * output information message
 * @log:    CLog object pointer
 * @fmt:    message format
 * @...:    message data
 * @return:  message length
 *************************************/
#define clog_info(log, fmt, ...) _clog_info( \
    log, __FILE__, __LINE__, __FUNCTION__,   \
    fmt, ##__VA_ARGS__)

/************************************
 * output debug message
 * @log:    CLog object pointer
 * @fmt:    message format
 * @...:    message data
 * @return:  message length
 *************************************/
#define clog_debug(log, fmt, ...) _clog_debug( \
    log, __FILE__, __LINE__, __FUNCTION__,     \
    fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // CLOG_H
