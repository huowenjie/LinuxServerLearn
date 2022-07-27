#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "test.h"

void close_session(int sfd)
{

}

void md5_client()
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

        char    buff[64] = { 0 };
        size_t  buf_len = sizeof(buff);
        ssize_t rdlen = 0;

        rdlen = read(sfd, buff, buf_len);
        if (rdlen < 0 || rdlen > buf_len) {
            ERR_PRINT("读取消息失败！\n");
            break;
        }
        LOG_PRINT("%s\n", buff);

        while (1) {
            char *origin = INPUTSTR("请输入要做摘要的内容:");
            if (origin) {
                write(sfd, origin, strlen(origin));
            }

            rdlen = read(sfd, buff, buf_len);
            if (rdlen < 0) {
                ERR_PRINT("读取消息失败！\n");
                break;
            }

            if (!memcmp(buff, "quit session", 12)) {
                LOG_PRINT("断开连接！\n");
                break;
            }

            // 打印摘要数据
            log_printb("MD5", (const uint8_t *)buff, rdlen);
        }
    } while (0);

    close(sfd);
}
