
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

// 守护进程启动器(通用) - 将指定的程序以后台守护进程的方式启动

/* Close all file descriptors >= fd */
static void close_all_fds(int fd)
{
    int fd_limit = sysconf(_SC_OPEN_MAX);
    if (fd_limit > 128)
        fd_limit = 128;
    while (fd < fd_limit)
        close(fd++);
}

/**
 * @brief Create a daemon process
 * 
 * @param nochdir  1 = do not change working directory to "/"
 * @param noclose  1 = do not close stdin/stdout/stderr
 * @param asroot   1 = keep root privileges, 0 = drop to UID 1
 * @return int     0 = success, -1 = failure
 */
static int create_daemon(int nochdir, int noclose, int asroot)
{
    // 第一次fork，脱离终端
    pid_t pid;
    pid = fork();
    if (pid < 0) {
        perror("Failed to first fork");
        return -1;
    } else if (pid > 0) {
        _exit(EXIT_SUCCESS); // 父进程退出
    }

    // 子进程成为新会话组长，脱离终端
    if (setsid() < 0) {
        perror("Failed to setsid");
        return -1;
    }
    
    // 可选降权限
    if (!asroot && setuid(1) < 0) {
        perror("Failed to setuid");
        return -1;
    }

    // 第二次fork，禁止进程重新打开控制终端
    pid = fork();
    if (pid < 0) {
        perror("Failed to second fork");
        return -1;
    } else if (pid > 0) {
        _exit(EXIT_SUCCESS);
    }   

    // 切换目录
    if (!nochdir)
        chdir("/");
    
    // 关闭并重定向标准文件描述符
    if (!noclose) {
        close_all_fds(0);
        int fd = open("/dev/null", O_RDWR, 0);
        if (fd < 0) {
            printf("Failed to open /dev/null, errno=%d\n", errno);
            return -1;
        }
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO)
            close(fd);
    }
    return 0;
}

static void PrintUsage(const char* prog_name)
{
    fprintf(stdout,
        "\n-----\n\n"
        "Usage:\n"
        "    %s program_name [args...]\n\n"
        "Where:\n"
        "    %s - This daemon loader program.\n"
        "    program_name - Program (with path) to run as daemon.\n\n"
        "Example:\n"
        "    %s ./myprog - Run 'myprog' as daemon.\n\n",
        prog_name, prog_name, prog_name
    );
}

// ./daemon_loader <program_path> [program_args...]
int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("Error: Missing program name to run as daemon!\n");
        PrintUsage(argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* loader_name = argv[0];  // 守护进程加载器
    const char* target_path = argv[1];  // 要启动的目标程序路径
    char **target_argv = argv + 1;      // 要传给目标程序的参数列表

    fprintf(stdout, "Daemon loader: Launching '%s' as daemon...\n", target_path);
    
    if (create_daemon(1, 0, 1) < 0) {
        perror("Error: Failed to create daemon process.\n");
        return EXIT_FAILURE;
    }
    
    signal(SIGCHLD, SIG_IGN);// 忽略子进程终止信号，避免僵尸进程
    
    execv(target_path, target_argv);// 目标程序 替换当前进程镜像
    //execl(argv[1], argv[1], NULL);

    // execv 失败
    fprintf(stdout, "Error: Failed to execute '%s', errno=%d\n", target_path, errno);
    return EXIT_FAILURE;
}
