#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/* Struttura per contenere file name e parametri */
struct Pack
{
    char **files;
    int num_of_files;
    int *params;
};

void *worker(void *arg)
{

    return (void *)0;
}

void *consumer(void *arg)
{

    return (void *)0;
}

void thread_make(pthread_t *th, int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        if (n == 0)
        {
            if (pthread_create(&th[i], NULL, &worker, NULL) != 0)
            {
                perror("Failed thread creation\n");
            }
        }
        else
        {
            if (pthread_create(&th[i], NULL, &consumer, NULL) != 0)
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
}

int main(int argc, char **argv)
{
    int *params = malloc(sizeof(int) * 3);
    int nthread = 4;
    int qlen = 8;
    int delay = 0;
    int c;

    while ((c = getopt(argc, argv, ":n:q:t:")) != -1)
    {
        switch (c)
        {
        case 'n':
            nthread = atoi(optarg);
            break;
        case 'q':
            qlen = atoi(optarg);
            break;
        case 't':
            delay = atoi(optarg);
            break;
        case '?':
            if (isprint(optopt))
                fprintf(stderr, "Opzione sconosciuta `-%c'.\n", optopt);
            return (int *)1;
        default:
            abort();
        }
    }
    params[0] = nthread;
    params[1] = qlen;
    params[2] = delay;
    int i;
    int j = 0;
    int num_of_file = argc - optind;
    char **files = malloc(sizeof(char *) * num_of_file);
    for (i = optind; i < argc; i++)
    {
        files[j] = argv[i];
        j++;
    }
    struct Pack p;
    p.files = files;
    p.params = params;
    free(files);
    free(params);
    return 0;
}
