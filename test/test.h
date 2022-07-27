#ifndef __CLIENT_TEST_H__
#define __CLIENT_TEST_H__

#include <stddef.h>
#include <string.h>
#include "trace.h"

// 输入函数
int inputi(const char *hint, int def);
char * inputa(const char *hint, const char *def);

#define INPUTINT(hint) inputi((hint), 0)
#define INPUTSTR(hint) inputa((hint), "")

// 显示菜单
void show_menu(const char * const * menu, int count, const char *title);

// 选择菜单
int select_menu(const char * const * menu, int count, const char *title);

// ----------------------------------------------------------------------------

// 简单客户端
void simple_client();

// MD5客户端
void md5_client();

// io 复用客户端
void io_client();

// UDP 连接
void udp_client();

#endif // __CLIENT_TEST_H__
