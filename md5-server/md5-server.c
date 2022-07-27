#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <signal.h>
#include <errno.h>
#include <string.h>

#include "trace.h"
#include "algos.h"

#define DEF_PORT htons(8896)
#define DEF_ADDR htonl(INADDR_ANY)

static void child_handle(int cfd);
static void signal_handle(int sig);

int main(int argc, char *argv[])
{
    struct sockaddr_in saddr = { 0 };
    struct sockaddr_in caddr = { 0 };

    socklen_t slen = sizeof(saddr);

    int sfd = 0;
    int cfd = 0;

    pid_t child_id = 0;

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) {
        LOG_PRINT("创建文件描述符失败！\n");
        return -1;
    }

    saddr.sin_family = AF_INET;
    saddr.sin_port = DEF_PORT;
    saddr.sin_addr.s_addr = DEF_ADDR;

    if (bind(sfd, (const struct sockaddr *)&saddr, slen) < 0) {
        ERR_PRINT("绑定失败！\n");
        return -1;
    }

    if (listen(sfd, 5) < 0) {
        ERR_PRINT("监听失败！\n");
        return -1;
    }

    LOG_PRINT("当前进程 ID = %d\n", (int)getpid());

    // 注册信号，用于处理僵尸进程
    signal(SIGCHLD, signal_handle);

    while (1) {
        cfd = accept(sfd, (struct sockaddr *)&caddr, &slen);
        if (cfd < 0) {
            if (errno == EINTR) {
                LOG_PRINT("系统中断, 重新等待连接！\n");
                continue;
            } else {
                ERR_PRINT("连接失败，停止服务器！\n");
                break;
            }
        }

        LOG_PRINT("有一个新的连接，来自 %s : %d\n", 
            inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));

        if ((child_id = fork()) == 0) {
            LOG_PRINT("本进程 ID = %d, 父进程 ID = %d\n", (int)getpid(), (int)getppid());

            close(sfd);

            child_handle(cfd);

            close(cfd);
            _exit(0);
        }

        LOG_PRINT("创建了一个子进程，进程 ID = %d\n", (int)child_id);

        // 父进程关闭不需要的文件描述符,
        close(cfd);
    }

    close(sfd);
    return 0;
}

// 关闭当前会话
static void close_session(int cfd)
{
    char buff[16] = "quit session";
    size_t len = strlen(buff) + 1;

    write(cfd, buff, len);
}

static void child_handle(int cfd)
{
    char buff[64] = "请传送文件数据，我们将对其进行 MD5 摘要";

    uint8_t  md5ret[16] = { 0 };
    uint32_t md5len = sizeof(md5ret);

    size_t  len = strlen(buff) + 1;
    ssize_t rdlen = 0;

    MD5_CTX ctx;

    // 首次建立连接后推送一条消息
    if (write(cfd, buff, len) != len)
    {
        ERR_PRINT("推送消息失败，尝试关闭会话...\n");
        close_session(cfd);
        return;
    }

    len = sizeof(buff);

    // 推送成功后等待客户端数据
    while ((rdlen = read(cfd, buff, len)) > 0) {
        md5_init(&ctx);

        if (!memcmp(buff, "quit session", 12)) {
            LOG_PRINT("断开连接\n");
            close_session(cfd);
            return;
        }

        // 读取客户端的数据并且进行摘要运算
        md5_update(&ctx, (const uint8_t *)buff, rdlen);

        // 获取 md5 结果
        md5_final(&ctx, md5ret, &md5len);

        // 将结果发送给客户端
        if (write(cfd, md5ret, md5len) != md5len)
        {
            ERR_PRINT("推送摘要结果失败，尝试关闭会话...\n");
            close_session(cfd);
            return;
        }
    }
}

static void signal_handle(int sig)
{
    pid_t id = 0;
    int stat = 0;

    if (sig != SIGCHLD) {
        return;
    }

    while ((id = waitpid(-1, &stat, WNOHANG)) > 0);
}
