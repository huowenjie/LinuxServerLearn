#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "test.h"

void simple_client()
{
    struct sockaddr_in saddr = { 0 };
    int     sfd  = 0;
    int     port = 0;
    char    *str_addr = NULL;

    str_addr = INPUTSTR("请输入服务端IP地址:");

    if (!str_addr || !str_addr[0]) {
        return;
    }

    port = INPUTINT("请输入端口号:");

    if (!port) {
        return;
    }

    sfd = socket(AF_INET, SOCK_STREAM, 0);

    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(str_addr);
    saddr.sin_port = htons((uint16_t)port);

    do {
        if (connect(sfd, (const struct sockaddr *)&saddr, sizeof(saddr))) {
            ERR_PRINT("连接失败！\n");
            break;
        }

        char buff[64] = { 0 };
        size_t buf_len = sizeof(buff);

        ssize_t rlen = read(sfd, buff, buf_len);
        if (rlen < 0 || rlen > buf_len) {
            ERR_PRINT("接收消息错误！\n");
            break;
        }

        LOG_PRINT("%s：%s\n", str_addr, buff);

        while (1) {
            char *send = INPUTSTR("请输入指令：");
            if (write(sfd, send, strlen(send) + 1) < 0) {
                ERR_PRINT("发送消息错误！\n");
            }

            if (!strcmp(send, "quit")) {
                break;
            }

            // 发送指令之后等待服务器的指令
            ssize_t rd_size = read(sfd, buff, buf_len);
            if (rd_size < 0 || rd_size > buf_len) {
                ERR_PRINT("接收消息错误！\n");
            }

            buff[buf_len - 1] = '\0';
            LOG_PRINT("%s\n", buff);
        }
    } while (0);

    close(sfd);
}
