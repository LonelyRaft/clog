
#ifndef XLOG_H
#define XLOG_H

#include "clog.h"

extern CLog *xlog;

#define xlogError(fmt, ...) clog_error(xlog, fmt, ##__VA_ARGS__)
#define xlogWarn(fmt, ...) clog_warn(xlog, fmt, ##__VA_ARGS__)
#define xlogInfo(fmt, ...) clog_info(xlog, fmt, ##__VA_ARGS__)
#define xlogDebug(fmt, ...) clog_debug(xlog, fmt, ##__VA_ARGS__)

#endif // XLOG_H
