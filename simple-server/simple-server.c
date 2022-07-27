#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "trace.h"

#define DEF_PORT htons(8895)
#define DEF_ADDR htonl(INADDR_ANY)

int main(int argc, char *argv[])
{
    printf("-- 默认端口号 -- 8895\n");

    struct sockaddr_in saddr = { 0 };
    struct sockaddr_in caddr = { 0 };

    socklen_t caddr_len = sizeof(caddr);

    int sfd = 0;
    int cfd = 0;

    // 打开文件描述符
    sfd = socket(AF_INET, SOCK_STREAM, 0);

    saddr.sin_family = AF_INET;
    saddr.sin_port = DEF_PORT;
    saddr.sin_addr.s_addr = DEF_ADDR;

    if (bind(sfd, (const struct sockaddr *)&saddr, sizeof(saddr))) {
        ERR_PRINT("绑定失败！\n");
        return -1;
    }

    if (listen(sfd, 10)) {
        ERR_PRINT("监听失败！\n");
        return -1;
    }

    char buff[64] = "欢迎访问本机，祝您生活愉快！";

    while (1) {
        LOG_PRINT("等待连接...\n");
        cfd = accept(sfd, (struct sockaddr *)&caddr, &caddr_len);

        char *saddr = inet_ntoa(caddr.sin_addr);

        LOG_PRINT("有一个客户端已经连接：ip:%s:%d\n", 
            saddr, ntohs(caddr.sin_port));

        sprintf(buff, "%s %s", saddr, "欢迎访问本机，祝您生活愉快！");

        // 发送欢迎语
        if (write(cfd, buff, (size_t)strlen(buff) + 1) < 0) {
            ERR_PRINT("服务器数据发送错误！\n");
        }

        // 发送数据后等待客户端的指令
        size_t  bf_size = sizeof(buff);
        ssize_t rd_size = 0;

        while (1) {
            rd_size = read(cfd, buff, bf_size);
            if (rd_size < 0 && rd_size > bf_size) {
                ERR_PRINT("服务器数据接收错误！\n");
            }

            buff[bf_size - 1] = '\0';
            LOG_PRINT("%s\n", buff);

            if (!strncmp(buff, "quit", bf_size)) {
                break;
            } else {

                // 除退出指令外，增加一个服务标签以作区别
                strcat(buff, "[服务标签]");

                if (write(cfd, buff, (size_t)strlen(buff) + 1) < 0) {
                    ERR_PRINT("服务器数据发送错误！\n");
                }
            }
        }

        close(cfd);
    }

    LOG_PRINT("Start Server!\n");

    close(sfd);
    return 0;
}
