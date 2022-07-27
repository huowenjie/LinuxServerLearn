#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>

#include "trace.h"
#include "global.h"

#define MAX_CONN 5

// 简单 UDP 服务
static int simple_server();

int main(int argc, char *argv[])
{
    if (argc < 2 || (argc == 2 && !strncmp(argv[1], "1", 1))) {
        return simple_server();
    }

    // if (argc == 2 && !strncmp(argv[1], "2", 1)) {
    //     return simple_server();
    // }

    return 0;
}

int simple_server()
{
    int sfd = 0;
    int ret = FAILED;
    
    struct sockaddr_in saddr = { 0 };
    struct sockaddr_in caddr = { 0 };
    
    socklen_t addr_len = sizeof(saddr);
    
    char buff[64] = { 0 };
    size_t buf_len = sizeof(buff);
    ssize_t rd_len = 0;

    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd < 0) {
        ERR_PRINT("创建socket失败!\n");
        return ret;
    }

    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(8898);

    if (bind(sfd, (const struct sockaddr *)&saddr, addr_len) < 0) {
        ERR_PRINT("绑定失败！\n");
        goto err;
    }

    while (1) {
        rd_len = recvfrom(
            sfd, buff, buf_len, 0, (struct sockaddr *)&caddr, &addr_len);
        if (rd_len < 0) {
            ERR_PRINT("收取信息失败！\n");
            continue;
        }

        LOG_PRINT("%s->127.0.0.1:%s\n", inet_ntoa(caddr.sin_addr), buff);

        // 增加一个服务标签然后回射回去
        strcat(buff, "[服务标签]");
        sendto(sfd, buff, strlen(buff) + 1, 0, (const struct sockaddr *)&caddr, addr_len);
    }

    ret = SUCCESS;
err:
    close(sfd);
    return ret;
}
