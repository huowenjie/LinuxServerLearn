#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "test.h"
#include "trace.h"

void udp_client()
{
    int sfd   = 0;
    int port  = 0;
    int rdlen = 0;
    char *str = NULL;
    char buff[64] = { 0 };
    char ip[32] = { 0 };

    struct sockaddr_in saddr = { 0 };
    struct sockaddr_in raddr = { 0 };

    socklen_t addr_len = sizeof(saddr);

    str = INPUTSTR("请输入 IP 地址:");
    if (!str || !str[0]) {
        return;
    }

    strncpy(ip, str, sizeof(ip));

    port = INPUTINT("请输入端口号:");
    if (port <= 0) {
        return;
    }

    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd < 0) {
        ERR_PRINT("创建socket失败!\n");
        return;
    }

    saddr.sin_addr.s_addr = inet_addr(ip);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8898);

    while (1) {
        str = INPUTSTR("请输入内容:");
        if (!strncmp(str, "quit", 4)) {
            break;
        }

        sendto(sfd, str, strlen(str) + 1, 0, 
            (const struct sockaddr *)&saddr, addr_len);

        // 接收从服务端返回的消息
        if ((rdlen = recvfrom(sfd, buff, sizeof(buff), 0, (struct sockaddr *)&raddr, &addr_len)) < 0) {
            ERR_PRINT("接收消息失败!\n");
            break;
        }

        LOG_PRINT("%s->%s:%s\n", inet_ntoa(raddr.sin_addr), ip, buff);
    }

    close(sfd);
}
