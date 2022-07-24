#include "xerrori.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define _GNU_SOURCE
#define __HERE__ __LINE__, __FILE__
#define HOST "127.0.0.1" /* local host */
#define PORT 65432

typedef struct
{
    int *params;
    int *cindex;
    char **buffer;
    pthread_mutex_t *mutex;
    sem_t *sem_free_slots;
    sem_t *sem_data_items;
    int *num_of_files;
    int *fd_skt;
    struct sockaddr_in *s_addr;
} t_args;

volatile sig_atomic_t sign = 0;

void handler(int s)
{
    sign = 1;
}

long sum_file(char *f_name)
{
    FILE *f = fopen(f_name, "rb");
    long res = 0;
    int c = 0;
    if (f == NULL)
        fprintf(stderr, "Errore apertura file\n");
    else
    {
        long x;
        while (true)
        {
            int e = fread(&x, sizeof(long), 1, f);
            if (e == -1)
                termina("Errore fread\n");
            if (e != -1)
                break;
            res += (c++ * x);
        }
    }
    return res;
}

void *worker_body(void *arg)
{
    t_args *args = (t_args *)arg;
    char file_name[255];
    do
    {
        xsem_wait(args->sem_data_items, __HERE__);
        xpthread_mutex_lock(args->mutex, __HERE__);
        int i = *(args->cindex);
        int size_buffer = args->params[1];
        strcpy(file_name, args->buffer[i % size_buffer]);
        *(args->cindex) += 1;
        xpthread_mutex_unlock(args->mutex, __HERE__);
        xsem_post(args->sem_free_slots, __HERE__);
        if (strcmp(file_name, "/") == 0)
            break;
        long sum = sum_file(file_name);
        printf("Il thread %d ha restituito %ld come somma\n", gettid(), sum);
    } while (true);
    pthread_exit(NULL);
}

void gen_params(int argc, char **argv, int params[])
{
    int nthread = 4;
    int qlen = 8;
    int delay = 0;
    int c;
    int x;

    while ((c = getopt(argc, argv, ":n:q:t:")) != -1)
    {
        switch (c)
        {
        case 'n':
            x = atoi(optarg);
            if (x <= 1)
            {
                fprintf(stderr, "Valore non valido '%d', -n deve essere > 0.\n", x);
                exit(1);
            }
            nthread = atoi(optarg);
            break;
        case 'q':
            x = atoi(optarg);
            if (x <= 1)
            {
                fprintf(stderr, "Valore non valido '%d', -q deve essere > 0.\n", x);
                exit(1);
            }
            qlen = atoi(optarg);
            break;
        case 't':
            x = atoi(optarg);
            if (x <= 0)
            {
                fprintf(stderr, "Valore non valido '%d', -t deve essere > 0.\n", x);
                exit(1);
            }
            delay = atoi(optarg);
            break;
        case '?':
            if (isprint(optopt))
                fprintf(stderr, "Opzione sconosciuta '-%c'.\n", optopt);
            exit(1);
        default:
            abort();
        }
    }
    params[0] = nthread;
    params[1] = qlen;
    params[2] = delay;
}

int main(int argc, char **argv)
{
    int params[3]; // 0. nthread ; 1. qlen ; 2. delay
    if (argc < 2)
    {
        printf("Uso: %s file [file ...] \n", argv[0]);
        return 1;
    }
    gen_params(argc, argv, params);
    int j = 0;
    int num_of_files = argc - optind;
    char **files = malloc(sizeof(char *) * num_of_files);
    for (int i = optind; i < argc; i++)
    {
        files[j] = argv[i];
        j++;
    }

    struct sigaction sa;
    sa.sa_handler = handler;
    sigaction(SIGINT, NULL, &sa);
    sigaction(SIGINT, &sa, NULL);

    pthread_mutex_t cmutex = PTHREAD_MUTEX_INITIALIZER;
    sem_t sem_free_slots, sem_data_items;
    xsem_init(&sem_free_slots, 0, params[1], __HERE__);
    xsem_init(&sem_data_items, 0, 0, __HERE__);
    char *buffer[params[1]];
    int pindex = 0, cindex = 0;

    t_args *args = malloc(sizeof(t_args));
    args->buffer = buffer;
    args->params = params;
    args->mutex = &cmutex;
    args->sem_data_items = &sem_data_items;
    args->sem_free_slots = &sem_free_slots;
    args->num_of_files = &num_of_files;

    pthread_t th[params[0]];
    for (int i = 0; i < params[0]; i++)
    {
        xpthread_create(&th[i], NULL, worker_body, args, __HERE__);
    }

    for (int i = 0; sign == 0 && i < num_of_files; i++)
    {
        usleep(params[2] * 1000);
        xsem_wait(&sem_free_slots, __HERE__);
        strcpy(buffer[pindex++ % params[1]], files[i]);
        xsem_post(&sem_data_items, __HERE__);
    }

    // terminazione threads
    for (int i = 0; i < params[0]; i++)
    {
        xsem_wait(&sem_free_slots, __HERE__);
        strcpy(buffer[pindex++ % params[1]], "/");
        xsem_post(&sem_data_items, __HERE__);
    }

    for (int i = 0; i < params[0]; i++)
    {
        xpthread_join(th[i], NULL, __HERE__);
    }

    return 0;
}