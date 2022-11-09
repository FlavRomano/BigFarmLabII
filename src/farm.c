#include "apilab.h"

#define _GNU_SOURCE
#define __HERE__ __LINE__, __FILE__
#define HOST "127.0.0.1" /* local host */
#define PORT 65203

typedef struct
{
    int *cindex;
    char **buffer;
    int *buf_len;
    pthread_mutex_t *mutex;
    sem_t *sem_free_slots;
    sem_t *sem_data_items;
} t_args;

volatile sig_atomic_t sign = 0;

void handler()
{
    sign = 1;
}

long sum_file(char *f_name)
{
    FILE *f = xfopen(f_name, "rb", __HERE__);
    long x, res = 0;
    int i = 0;
    if (f == NULL)
    {
        termina("Errore apertura file\n");
    }
    while (fread(&x, sizeof(long), 1, f))
    {
        res += (i++ * x);
    }

    return res;
}

void send_to_collector(char *s)
{
    int fd_skt, res_len;
    struct sockaddr_in serv;
    size_t e;

    fd_skt = 0;
    res_len = strlen(s);
    int package[strlen(s)];
    for (int i = 0; i < res_len; i++)
    {
        package[i] = (int)s[i];
    }

    if ((fd_skt = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        termina("Errore creazione socket\n");
    }

    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    serv.sin_addr.s_addr = inet_addr(HOST);

    if (connect(fd_skt, (struct sockaddr *)&serv, sizeof(serv)) < 0)
    {
        termina("Errore apertura connessione\n");
    }

    int dim_host_to_network = htonl(res_len);
    e = writen(fd_skt, &dim_host_to_network, sizeof(int));
    if (e != sizeof(int))
    {
        termina("Errore invio dati al server\n");
    }
    for (int i = 0; i < res_len; i++)
    {
        int c = htonl(package[i]);
        e = writen(fd_skt, &c, sizeof(int));
        if (e != sizeof(int))
        {
            termina("Errore write\n");
        }
    }
    if (close(fd_skt) < 0)
    {
        termina("Errore chiusura socket\n");
    }
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
        int size_buffer = *(args->buf_len);
        strcpy(file_name, args->buffer[i % size_buffer]);
        *(args->cindex) += 1;
        xpthread_mutex_unlock(args->mutex, __HERE__);
        xsem_post(args->sem_free_slots, __HERE__);
        if (!strcmp(file_name, "_"))
            break;
        long sum = sum_file(file_name);
        char res[276];
        sprintf(res, "%s:%ld", file_name, sum);
        send_to_collector(res);
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
            if (x < 1)
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
    int params[3]; /* 0. nthread ; 1. qlen ; 2. delay */
    if (argc < 2)
    {
        printf("Uso: %s {-n | -q | -d} file [file ...]\n", argv[0]);
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
    for (int i = 0; i < params[1]; i++)
    {
        buffer[i] = malloc(255);
    }
    int pindex = 0, cindex = 0;

    t_args *args = malloc(sizeof(t_args));
    args->cindex = &cindex;
    args->buffer = buffer;
    args->buf_len = &params[1];
    args->mutex = &cmutex;
    args->sem_data_items = &sem_data_items;
    args->sem_free_slots = &sem_free_slots;

    /* thread worker */
    pthread_t th[params[0]];
    for (int i = 0; i < params[0]; i++)
    {
        xpthread_create(&th[i], NULL, worker_body, args, __HERE__);
    }

    /* master thread */
    for (int i = 0; sign == 0 && i < num_of_files; i++)
    {
        usleep(params[2] * 1000);
        xsem_wait(&sem_free_slots, __HERE__);
        strcpy(buffer[pindex++ % params[1]], files[i]);
        xsem_post(&sem_data_items, __HERE__);
    }
    /* terminazione threads con un dummy char */
    for (int i = 0; i < params[0]; i++)
    {
        xsem_wait(&sem_free_slots, __HERE__);
        strcpy(buffer[pindex++ % params[1]], "_");
        xsem_post(&sem_data_items, __HERE__);
    }

    for (int i = 0; i < params[0]; i++)
    {
        xpthread_join(th[i], NULL, __HERE__);
    }

    free(files);
    for (int i = 0; i < params[1]; i++)
    {
        free(buffer[i]);
    }
    sem_destroy(&sem_data_items);
    sem_destroy(&sem_free_slots);
    xpthread_mutex_destroy(&cmutex, __HERE__);

    return 0;
}