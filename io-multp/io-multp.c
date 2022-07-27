#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/select.h>
#include <poll.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "global.h"
#include "trace.h"

#define MAX_CONN 5
#define DEF_PORT htons(8897)

static void select_close(int cfd, fd_set *fds);
static void poll_close(int cfd);

static int select_server();
static int poll_server();

static const char *str_start = "欢迎使用本机!\n";

int main(int argc, char *argv[])
{
    if (argc < 2 || (argc == 2 && !strncmp(argv[1], "1", 1))) {
        return select_server();
    }
    
    if (argc == 2 && !strncmp(argv[1], "2", 1)) {
        return poll_server();
    }

    return 0;
}

void select_close(int cfd, fd_set *fds)
{
    // 关闭时候给客户端发送一条消息
    poll_close(cfd);
    FD_CLR(cfd, fds);
}

void poll_close(int cfd)
{
    // 关闭时候给客户端发送一条消息
    const char *info = "server quit";
    write(cfd, info, strlen(info) + 1);

    close(cfd);
}

static int select_server()
{
    char buff[64]  = { 0 };
    size_t buf_len = sizeof(buff);

    int maxfd = 0;
    int sfd = 0;
    int cfd = 0;
    int nfd = 0;
    int ret = SUCCESS;
    fd_set fds;
    fd_set allfds;

    int cfds[MAX_CONN] = { 0 };
    int i = 0;

    struct sockaddr_in saddr = { 0 };
    struct sockaddr_in caddr = { 0 };
    socklen_t addr_size = sizeof(struct sockaddr_in);

    printf("select server start!\n");

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) {
        ERR_PRINT("创建socket失败!\n");
        ret = FAILED;
        goto err;
    }

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8897);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sfd, (const struct sockaddr *)&saddr, addr_size) < 0) {
        ERR_PRINT("绑定失败！\n");
        ret = FAILED;
        goto err;
    }

    if (listen(sfd, 5) < 0) {
        ERR_PRINT("监听失败!\n");
        ret = FAILED;
        goto err;
    }

    // 初始化 fd_set
    FD_ZERO(&allfds);

    // 初始化客户端socket队列
    for (; i < MAX_CONN; ++i) {
        cfds[i] = -1;
    }

    maxfd = sfd;
    FD_SET(sfd, &allfds);

    while (1) {
        fds = allfds;

        // 只测试可读
        LOG_PRINT("等待 socket 描述符变化！\n");

        if ((nfd = select(maxfd + 1, &fds, NULL, NULL, NULL)) < 0) {
            ERR_PRINT("等待描述符状态失败\n");
            ret = FAILED;
            break;
        }

        // 如果测试文件描述符是否可读
        if (FD_ISSET(sfd, &fds)) {
            cfd = accept(sfd, (struct sockaddr *)&caddr, &addr_size);
            if (cfd < 0) {
                if (errno == EINTR) {
                    LOG_PRINT("系统中断重新监听\n");
                    continue;
                } else {
                    ERR_PRINT("建立连接失败\n");
                    ret = FAILED;
                    break;
                }
            }

            if (cfd > maxfd) {
                maxfd = cfd;
            }

            // 从队列中查询是否有位置可以保存描述符
            for (i = 0; i < MAX_CONN; ++i) {
                if (cfds[i] < 0) {
                    cfds[i] = cfd;
                    FD_SET(cfd, &allfds);

                    LOG_PRINT("有一个客户端已经连接：ip:%s:%d\n", 
                        inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));

                    // 连接成功后向客户端发送一条消息
                    write(cfd, str_start, strlen(str_start) + 1);
                    break;
                }
            }

            if (i == MAX_CONN) {
                LOG_PRINT("最大连接数已满，请稍后...\n");
                close(cfd);
                continue;
            }
        }

        // 依次处理所有的客户端文件描述符
        for (i = 0; i < MAX_CONN; ++i) {
            if ((cfd = cfds[i]) < 0) {
                continue;
            }

            // 如果 cfds[i] 可读，则处理该描述符
            if (FD_ISSET(cfd, &fds)) {
                if (read(cfd, buff, buf_len) < 0) {
                    ERR_PRINT("读取数据错误，断开连接!\n");
                    select_close(cfd, &allfds);
                    cfds[i] = -1;
                    continue;
                }

                // 客户端输入 quit 断开连接
                if (!memcmp(buff, "quit", 4)) {
                    select_close(cfd, &allfds);
                    cfds[i] = -1;
                    LOG_PRINT("连接任务已处理完毕！处理下一个连接！\n");
                    continue;
                }

                // 除退出指令外，增加一个服务标签以作区别
                strcat(buff, "[服务标签]");

                if (write(cfd, buff, (size_t)strlen(buff) + 1) < 0) {
                    ERR_PRINT("服务器数据发送错误！\n");
                }
            }
        }
    }

err:
    close(sfd);
    return ret;
}

static int poll_server()
{
    int ret = FAILED;
    int sfd = 0;
    int cfd = 0;

    int i = 0;
    int maxi = 0;
    int pret = 0;

    char buff[64] = { 0 };
    size_t buff_len = sizeof(buff);
    size_t rd_len = 0;

    struct sockaddr_in saddr;
    struct sockaddr_in caddr;
    struct pollfd pfds[MAX_CONN];
    socklen_t addr_len = sizeof(struct sockaddr_in);

    printf("poll 服务启动!\n");
    
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) {
        ERR_PRINT("创建socket失败!\n");
        return FAILED;
    }

    saddr.sin_family = AF_INET;
    saddr.sin_port = DEF_PORT;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sfd, (const struct sockaddr *)&saddr, addr_len)) {
        ERR_PRINT("绑定socket失败!\n");
        goto err;
    }

    if (listen(sfd, MAX_CONN) < 0) {
        ERR_PRINT("监听socket失败！\n");
        goto err;
    }

    pfds[0].fd = sfd;
    pfds[0].events = POLLIN;
    pfds[0].revents = 0;
    maxi = 0;

    // 初始化链表
    for (i = 1; i < MAX_CONN; ++i) {
        pfds[i].fd = -1;
        pfds[i].events = 0;
        pfds[i].revents = 0;
    }

    while (1) {
        LOG_PRINT("等待描述如变化！\n");

        if ((pret = poll(pfds, maxi + 1, -1)) < 0) {
            ERR_PRINT("等待描述符变化失败!\n");
            break;
        }

        LOG_PRINT("当前可操作的描述符数量%d\n", pret);

        if (pfds[0].revents & POLLIN) {
            cfd = accept(sfd, (struct sockaddr *)&caddr, &addr_len);
            if (cfd < 0) {
                if (errno == EINTR) {
                    LOG_PRINT("系统中断，重新等待！\n");
                    continue;
                } else {
                    ERR_PRINT("等待连接错误！\n");
                    break;
                }
            }

            LOG_PRINT("有一个客户端已经连接：ip:%s:%d\n", 
                inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));

            // 连接成功后向客户端发送一条消息
            write(cfd, str_start, strlen(str_start) + 1);

            for (i = 1; i < MAX_CONN; ++i) {
                if (pfds[i].fd < 0) {
                    pfds[i].fd = cfd;
                    pfds[i].events = POLLIN;
                    break;
                }
            }

            if (i > maxi) {
                maxi = i;
            }
        }

        for (i = 1; i < maxi; ++i) {
            if (pfds[i].fd < 0) {
                continue;
            }

            if (pfds[i].events & (POLLIN | POLLERR)) {
                if ((rd_len = read(pfds[i].fd, buff, buff_len)) < 0) {
                    ERR_PRINT("读取错误!\n");
                    poll_close(cfd);
                    pfds[i].fd = -1;
                    pfds[i].events = 0;
                    pfds[i].revents = 0;
                } else {
                    LOG_PRINT("%s\n", buff);

                    if (!strncmp(buff, "quit", 4)) {
                        poll_close(cfd);
                        pfds[i].fd = -1;
                        pfds[i].events = 0;
                        pfds[i].revents = 0;
                    }

                    // 除退出指令外，增加一个服务标签以作区别
                    strcat(buff, "[服务标签]");

                    if (write(cfd, buff, (size_t)strlen(buff) + 1) < 0) {
                        ERR_PRINT("服务器数据发送错误！\n");
                    }
                }
            }
        }
    }

    ret = SUCCESS;

err:
    close(sfd);
    return ret;
}
