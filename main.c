#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define _GNU_SOURCE
#define NUM_OF_PARAM 3
#define TRUE 1

/*
    ; RICORDATI DI LEVARLI
    ; SERVONO SOLO A NON
    ; ROMPERE IL LINTER DEL MAC
*/

extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;

struct Pack
{
    char **files;
    int num_of_files;
    int next_file;
    int *params; /* params[0] = nthread; params[1] = qlen; params[2] = delay */
} typedef Pack;

struct Args_t
{
    Pack *p;
    char **file_buffer;
    int in;
    int out;
} typedef Args_t;

pthread_mutex_t mutex_buffer;
sem_t sem_full, sem_empty;

Pack *make_pack(char **files, int n, int params[])
{
    Pack *p;
    p = (Pack *)malloc(sizeof(Pack));
    p->files = files;
    p->num_of_files = n;
    p->next_file = 0;
    p->params = params;
    return p;
}

Args_t *make_args_t(Pack *p, int buffer_len)
{
    Args_t *args;
    args = (Args_t *)malloc(sizeof(Args_t));
    args->p = p;
    char **file_buffer = malloc(sizeof(char *) * buffer_len);
    args->file_buffer = file_buffer;
    args->in = 0;
    args->out = 0;
    return args;
}

void destroy_pack(Pack *p)
{
    free(p);
    p = NULL;
}

void destroy_args_t(Args_t *args)
{
    int i;
    for (i = 0; i < args->p->next_file; i++)
    {
        free(args->file_buffer[i]);
        args->file_buffer[i] = NULL;
    }
    free(args->file_buffer);
    free(args);
    args = NULL;
}

void print_pack(Pack *p)
{
    int i;
    for (i = 0; i < p->num_of_files; i++)
    {
        printf("%d) %s\n", i, p->files[i]);
    }
    for (i = 0; i < 3; i++)
    {
        printf("%d\n", p->params[i]);
    }
}

void print_buffer(Args_t *args)
{
    int i;
    for (i = 0; i < args->p->next_file; i++)
    {
        printf("%s\n", args->file_buffer[i]);
    }
}

void *worker(void *arg)
{
    Args_t *args = arg;
    int delay = args->p->params[2];
    while (TRUE)
    {
        sleep(delay);
        sem_wait(&sem_empty);
        pthread_mutex_lock(&mutex_buffer);
        args->file_buffer[args->in] = strdup(args->p->files[args->p->next_file]);
        printf("Producer: Insert Item %s at %d\n", args->p->files[args->p->next_file], args->in);
        args->p->next_file = (args->p->next_file + 1) % args->p->num_of_files;
        args->in = (args->in + 1) % args->p->params[1];
        pthread_mutex_unlock(&mutex_buffer);
        sem_post(&sem_full);
    }
    // print_buffer(args);
    return (void *)0;
}

void *consumer(void *arg)
{
    Args_t *args = arg;
    int delay = args->p->params[2];
    while (TRUE)
    {
        sem_wait(&sem_full);
        pthread_mutex_lock(&mutex_buffer);
        printf("Consumer: Consume %s\n", args->file_buffer[args->out]);
        args->out = (args->out + 1) % args->p->params[1];
        pthread_mutex_unlock(&mutex_buffer);
        sem_post(&sem_empty);
        sleep(delay);
    }
    return (void *)0;
}

void thread_make(pthread_t *th, int n, Pack *p)
{
    int i;
    Args_t *args = make_args_t(p, p->params[1]);
    pthread_mutex_init(&mutex_buffer, NULL);
    sem_init(&sem_empty, 0, p->params[1]);
    sem_init(&sem_full, 0, 0);
    for (i = 0; i < n + 1; i++)
    {
        if (i == 0) /* Worker */
        {
            if (pthread_create(&th[i], NULL, &worker, args) != 0)
            {
                perror("Failed thread creation\n");
            }
        }
        else /* Consumer */
        {
            if (pthread_create(&th[i], NULL, &consumer, args) != 0)
            {
                perror("Failed thread creation\n");
            }
        }
    }
    for (i = 0; i < n; i++)
    {
        if (pthread_join(th[i], NULL) != 0)
        {
            perror("Failed thread join\n");
        }
    }
    pthread_mutex_destroy(&mutex_buffer);
    sem_destroy(&sem_empty);
    sem_destroy(&sem_full);
    destroy_args_t(args);
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
            if (x == 0)
            {
                fprintf(stderr, "Valore non valido '%d', deve essere > 0.\n", x);
                exit(1);
            }
            nthread = atoi(optarg);
            break;
        case 'q':
            x = atoi(optarg);
            if (x == 0)
            {
                fprintf(stderr, "Valore non valido '%d', deve essere > 0.\n", x);
                exit(1);
            }
            qlen = atoi(optarg);
            break;
        case 't':
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
    int params[NUM_OF_PARAM];
    gen_params(argc, argv, params);
    int j = 0;
    int num_of_files = argc - optind;
    char **files = malloc(sizeof(char *) * num_of_files);
    for (int i = optind; i < argc; i++)
    {
        files[j] = argv[i];
        j++;
    }
    Pack *p = make_pack(files, num_of_files, params);
    pthread_t th[p->params[0]];
    thread_make(th, p->params[0], p);
    destroy_pack(p);
    free(files);
    return 0;
}
