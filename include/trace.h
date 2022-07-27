#ifndef __TRACE_H__
#define __TRACE_H__

#include <stdint.h>

#define LOG_PRINT(str, ...) log_print((str), ##__VA_ARGS__)
#define ERR_PRINT(str, ...) err_print((str), ##__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

// 日志打印
void log_print(const char *str, ...);

// 系统错误打印
void err_print(const char *str, ...);

// 日志格式化
const char * log_format(const char *str, ...);

// 打印二进制数据
void log_printb(const char *title, const uint8_t *data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif // __TRACE_H__
