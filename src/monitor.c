#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <string.h>
#include "../include/shared.h"

int pipe_fd;
struct sensor_data *data;

pthread_mutex_t mutex;
int running = 1;

int semid;

// semaphore operations
void sem_lock()
{
    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
}

void sem_unlock()
{
    struct sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
}

// signal handler
void handler(int sig)
{
    if(sig == SIGINT)
    {
        printf("\nShutting down...\n");
        running = 0;
    }

    if(sig == SIGUSR1)
    {
        printf("\n--- SNAPSHOT ---\n");
        printf("Temp: %d\n", data->temperature);
        printf("Vibration: %d\n", data->vibration);
        printf("Load: %d\n", data->load);
    }
}

// read config using read()
void read_config()
{
    int fd = open("config.txt", O_RDONLY);
    if(fd < 0) return;

    char buffer[100];
    read(fd, buffer, sizeof(buffer));
    printf("Config Loaded: %s\n", buffer);

    close(fd);
}

// thread 1: read pipe
void* reader_thread(void *arg)
{
    FILE *fp = fdopen(pipe_fd, "r");

    while(running)
    {
        int t,v,l;

        if(fscanf(fp,"%d %d %d",&t,&v,&l)==3)
        {
            pthread_mutex_lock(&mutex);
            sem_lock();

            data->temperature = t;
            data->vibration = v;
            data->load = l;

            sem_unlock();
            pthread_mutex_unlock(&mutex);
        }
    }
    return NULL;
}

// thread 2: logger with lseek
void* logger_thread(void *arg)
{
    int fd = open("logs.txt", O_CREAT | O_RDWR, 0666);

    while(running)
    {
        pthread_mutex_lock(&mutex);

        char buffer[100];

        sprintf(buffer,"Temp:%d Vib:%d Load:%d\n",
        data->temperature,
        data->vibration,
        data->load);

        lseek(fd, 0, SEEK_END); // REQUIRED
        write(fd, buffer, strlen(buffer));

        pthread_mutex_unlock(&mutex);

        sleep(2);
    }

    close(fd);
    return NULL;
}

// thread 3: alerts
void* alert_thread(void *arg)
{
    while(running)
    {
        pthread_mutex_lock(&mutex);

        if(data->temperature > 70)
            printf("ALERT: High Temperature\n");

        if(data->vibration > 5)
            printf("ALERT: High Vibration\n");

        if(data->load > 90)
            printf("ALERT: High Load\n");

        pthread_mutex_unlock(&mutex);

        sleep(1);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    pipe_fd = atoi(argv[1]);

    signal(SIGINT, handler);
    signal(SIGUSR1, handler);

    read_config(); // uses read()

    // shared memory
    int shmid = shmget(SHM_KEY, sizeof(struct sensor_data), IPC_CREAT | 0666);
    data = shmat(shmid, NULL, 0);

    // semaphore
    semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, 1);

    pthread_mutex_init(&mutex, NULL);

    pthread_t t1, t2, t3;

    pthread_create(&t1, NULL, reader_thread, NULL);
    pthread_create(&t2, NULL, logger_thread, NULL);
    pthread_create(&t3, NULL, alert_thread, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    shmdt(data);

    return 0;
}
