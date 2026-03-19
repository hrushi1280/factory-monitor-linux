#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int fd[2];
    pipe(fd);

    pid_t monitor = fork();

    if(monitor == 0)
    {
        close(fd[1]);

        char arg[10];
        sprintf(arg,"%d",fd[0]);

        execl("./monitor","monitor",arg,NULL);
    }

    for(int i=0;i<3;i++)
    {
        pid_t pid = fork();

        if(pid == 0)
        {
            close(fd[0]);

            char arg[10];
            sprintf(arg,"%d",fd[1]);

            execl("./sensor","sensor",arg,NULL);
        }
    }

    close(fd[0]);
    close(fd[1]);

    waitpid(monitor, NULL, 0); // REQUIRED

    return 0;
}
