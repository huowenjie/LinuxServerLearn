#include <stddef.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "trace.h"

// 日志打印
void log_print(const char *str, ...)
{
    va_list args;

    va_start(args, str);
    vprintf(str, args);
    va_end(args);
}

void err_print(const char *str, ...)
{
    va_list args;

    va_start(args, str);
    vprintf(str, args);
    va_end(args);

    printf("%s\n", strerror(errno));
}

// 日志格式化
const char * log_format(const char *str, ...)
{
    return NULL;
}

void log_printb(const char *title, const uint8_t *data, uint32_t len)
{
    if (!data || len <= 0) {
        return;
    }

    printf("name:%s size:%d\n", (!title || !title[0]) ? "" : title, len);
    printf("------------------------+------------------------\n");

    uint32_t j = 0;
    for (uint32_t i = 0; i < len; i++) {
        if (i > 0) {
            if (i % 16 == 0) {
                printf("\n");
                j++;
                if (j % 8 == 0) {
                    printf("------------------------+------------------------\n");
                }
            }
            else if (i % 8 == 0) {
                printf("| ");
            }
        }

        if ((i + 1) % 16 == 0) {
            printf("%02x", data[i]);
        }
        else {
            printf("%02x ", data[i]);
        }
    }

    printf("\n");
    printf("------------------------+------------------------\n");
}
