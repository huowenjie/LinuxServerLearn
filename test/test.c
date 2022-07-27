#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "test.h"

// 主菜单
static const char *main_menu[] = {
    "1.simple-client 连接",
    "2.md5-client 连接",
    "3.io-client 连接",
    "4.udp-client 连接",
    "0.退出"
};

int main(int argc, char *argv[])
{
    int run = 1;

    while (run) {
        int sel = select_menu(main_menu, 
            sizeof(main_menu) / sizeof(const char *), "主菜单");

        switch (sel) {
            case 1: simple_client(); break;
            case 2: md5_client();    break;
            case 3: io_client();     break;
            case 4: udp_client();    break;
            case 0:
            default: run = 0; break;
        }
    }

    LOG_PRINT("Exiting...\n");
    getchar();
    return 0;
}

int inputi(const char *hint, int def)
{
    char buff[32] = { 0 };
    char *rch = NULL;

    if (hint && hint[0]) {
        LOG_PRINT("%s\n", hint);
    }

    if (!fgets(buff, sizeof(buff), stdin)) {
        return def;
    }

    rch = strrchr(buff, '\n');
    if (rch) {
        *rch = '\0';
    }

    return atoi(buff);
}

char * inputa(const char *hint, const char *def)
{
    static char buff[128] = { 0 };
    char *rch = NULL;

    if (hint && hint[0]) {
        LOG_PRINT("%s\n", hint);
    }

    if (!fgets(buff, sizeof(buff), stdin)) {
        return NULL;
    }

    rch = strrchr(buff, '\n');
    if (rch) {
        *rch = '\0';
    }

    return buff;
}

void show_menu(const char * const * menu, int count, const char *title)
{
    int i = 0;

    if (!menu || count <= 0) {
        return;
    }

    LOG_PRINT("\n========================================\n");

    if (title && title[0]) {
        log_print("%s\n", title);
    }

    LOG_PRINT("========================================\n");

    for (; i < count; ++i) {
        LOG_PRINT("%s\n", menu[i]);
    }

    LOG_PRINT("%s\n\n", "----------------------------------------");
}

int select_menu(const char * const * menu, int count, const char *title)
{
    if (!menu || count < 1)
    {
        return -1;
    }

    show_menu(menu, count, title);
    return INPUTINT("请选择:");
}
