
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
static int daemon(int nochdir, int noclose, int asroot)
{
    // fork
    pid_t pid;
    pid = fork();
    if (pid < 0)
        return -1;
    if (pid > 0)
        _exit(EXIT_SUCCESS); // 父进程退出

    // 创建新会话
    if (setsid() < 0)
        return -1;
    
    // 可选降权限
    if (!asroot && setuid(1) < 0)
        return -1;

    // fork
    pid = fork();
    if (pid < 0)
        return -1;
    if (pid > 0)
        _exit(EXIT_SUCCESS);

    // 切换目录
    if (!nochdir)
        chdir("/");
    
    // 关闭并重定向标准文件描述符
    if (!noclose) {
        close_all_fds(0);
        int fd = open("/dev/null", O_RDWR, 0);
        if (fd < 0) {
            fprintf(stderr, "Failed to open /dev/null, errno=%d\n", errno);
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

#define TEXT(a) a
void PrintUsage(char* name)
{
    printf (
            TEXT("\n ----- \n\n")
            TEXT("Usage:\n")
            TEXT("   	%s program_name \n\n")
            TEXT("Where:\n")
            TEXT("   	%s - Name of this Daemon loader.\n")
            TEXT("   	program_name - Name (including path) of the program you want to load as daemon.\n\n")
            TEXT("Example:\n")
            TEXT("   	%s ./atprcmgr - Launch program 'atprcmgr' in current directory as daemon. \n\n\n\n"),
            name, name, name
            );
}

int main(int argc, char* argv[])
{
    printf(
           TEXT("\n")
           TEXT("Daemon loader\n")
           TEXT("- Launch specified program as daemon.\n")
           //TEXT("- Require root privilege to launch successfully.\n\n\n")
           );
    
    if (argc < 2)
    {
        printf("* Missing parameter : daemon program name not specified!\n");
        PrintUsage(argv[0]);
        exit(0);
    }
    
    printf("- Loading %s as daemon, please wait ......\n\n\n", argv[1]);
    
    if (daemon(1, 0, 1) >= 0)
    {
        signal(SIGCHLD, SIG_IGN);
        
        //execl(argv[1], argv[1], NULL);
        execv(argv[1], argv + 1);
        printf("! Excute daemon programm %s failed. \n", argv[1]);
        
        exit(0);
    }
    
    printf("! Create daemon error. Please check if you have 'root' privilege. \n");
    return 0;
}