#define HOST "127.0.0.1" /* local host */
#define PORT 65201
#include "apilab.h"
#include <libgen.h>

typedef struct
{
    int *cindex;
    char **buffer;
    int *buf_len;
    pthread_mutex_t *mutex;
    sem_t *sem_free_slots;
    sem_t *sem_data_items;
} t_args;

void handler();
long sum_file(char *f_name);
void send_to_collector(char *mess, size_t mess_len);
void *worker_body(void *arg);
void gen_params(int argc, char **argv, int params[]);