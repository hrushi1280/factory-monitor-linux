#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[])
{
    int pipe_fd = atoi(argv[1]);

    srand(time(NULL) ^ getpid());

    while(1)
    {
        int temp = rand()%100;
        int vib = rand()%10;
        int load = rand()%100;

        dprintf(pipe_fd,"%d %d %d\n",temp,vib,load);

        sleep(1);
    }

    return 0;
}
