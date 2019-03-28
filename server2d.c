#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    // 守护进程（父进程）  
    int status;
    pid_t pid;
    int sig;
    for ( ; ; )  {  
        if ( 0 == ( pid = fork()) ) {  
            // 工作进程（子进程）  
            system("./server2");  
            //信号处理函数signal_handler  
            /*if (sig == SIGTERM || sig == SIGINT) {  
                //destroy();  
                return 0;  
            }  */
        }  
        waitpid(-1, &status, 0);  
        if (WIFEXITED(status) && (WEXITSTATUS(status) == 0)) exit(0);  
    }  
}
