#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include "test.h"

void io_client()
{
    int  sfd  = 0;
    int  port = 0;
    char buff[64] = { 0 };
    size_t buf_len = sizeof(buff);

    struct sockaddr_in saddr = { 0 };
    socklen_t addr_size = sizeof(struct sockaddr_in);
    char *str_addr = NULL;

    str_addr = INPUTSTR("请输入服务端IP地址:");

    if (!str_addr || !str_addr[0]) {
        return;
    }

    port = INPUTINT("请输入端口号:");

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) {
        ERR_PRINT("创建 socket 失败！\n");
        return;
    }

    saddr.sin_addr.s_addr = inet_addr(str_addr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);

    if (connect(sfd, (const struct sockaddr *)&saddr, addr_size) < 0) {
        ERR_PRINT("连接服务器失败！\n");
        goto err;
    }

    // 读取服务端发送的结果
    if (read(sfd, buff, sizeof(buff)) < 0) {
        ERR_PRINT("读取服务器消息错误!\n");
        goto err;
    }

    LOG_PRINT("连接成功！%s\n", buff);

    while (1) {
        char *send = INPUTSTR("请输入指令：");
        if (write(sfd, send, strlen(send) + 1) < 0) {
            ERR_PRINT("发送消息错误！\n");
        }

        // 发送指令之后等待服务器的指令
        ssize_t rd_size = read(sfd, buff, buf_len);
        if (rd_size < 0 || rd_size > buf_len) {
            ERR_PRINT("接收消息错误！\n");
        }

        buff[buf_len - 1] = '\0';
        LOG_PRINT("%s\n", buff);

        if (!strncmp(buff, "server quit", 11)) {
            LOG_PRINT("断开连接！\n");
            break;
        }
    }

err:
    close(sfd);
}
