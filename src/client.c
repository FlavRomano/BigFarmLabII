#include "/home/local/ADUNIPI/f.romano39/BigFarmLabII/include/apilab.h"
#include <stdbool.h>

#define _GNU_SOURCE
#define HOST "127.0.0.1" /* local host */
#define PORT 65201

typedef struct
{
    int fd_skt;
    int type_server_request;
} segment;

segment *comunicazione(long l, bool richiesta_singola)
{
    int fd_skt;
    struct sockaddr_in serv;
    size_t e;
    fd_skt = 0;

    if ((fd_skt = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        termina("Errore creazione socket");
    }

    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    serv.sin_addr.s_addr = inet_addr(HOST);

    if (connect(fd_skt, (struct sockaddr *)&serv, sizeof(serv)) < 0)
    {
        termina("Errore apertura connessione");
    }

    int s_request;
    if (richiesta_singola)
    {
        size_t client_long_len = snprintf(NULL, 0, "%ld", l);
        char client_long[client_long_len];
        sprintf(client_long, "%ld", l);
        s_request = htonl(client_long_len);
        e = writen(fd_skt, &s_request, sizeof(int));
        if (e != sizeof(int))
        {
            termina("Errore write");
        }
        for (int i = 0; i < client_long_len; i++)
        {
            int c = htonl(client_long[i]);
            e = writen(fd_skt, &c, sizeof(int));
            if (e != sizeof(int))
            {
                termina("Errore write");
            }
        }
    }
    else /* stampa di tutte le coppie "somma:file" */
    {
        s_request = htonl(-1);
        e = writen(fd_skt, &s_request, sizeof(int));
        if (e != sizeof(int))
        {
            termina("Errore write");
        }
    }
    e = readn(fd_skt, &s_request, sizeof(int));
    if (e != sizeof(int))
    {
        termina("Errore read");
    }
    segment *s = malloc(sizeof(segment));
    s->fd_skt = fd_skt;
    s->type_server_request = s_request;
    return s;
}

void ricezione(segment *s)
{
    int fd_skt = s->fd_skt;
    int s_request = s->type_server_request;
    int n = ntohl(s_request);
    char *server_response = malloc(n + 1);
    int i, e;
    for (i = 0; i < n; i++)
    {
        e = readn(fd_skt, &s_request, sizeof(int));
        if (e != sizeof(int))
        {
            termina("Errore read");
        }
        char c = ntohl(s_request);
        server_response[i] = c;
    }
    if (close(fd_skt) < 0)
    {
        termina("Errore chiusura socket");
    }
    server_response[i] = '\0';
    printf("%s", server_response);
    free(server_response);
    free(s);
}

int main(int argc, char const *argv[])
{
    bool richiesta_singola = argc > 1;
    if (richiesta_singola)
    {
        for (int i = 1; i < argc; i++)
        {
            segment *s = comunicazione(atol(argv[i]), true);
            ricezione(s);
        }
    }
    else
    {
        segment *s = comunicazione(0, false);
        ricezione(s);
    }
    return 0;
}