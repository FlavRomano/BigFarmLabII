#include "/home/local/ADUNIPI/f.romano39/BigFarmLabII/include/apilab.h"
#include <stdbool.h>

#define _GNU_SOURCE
#define HOST "127.0.0.1" /* local host */
#define PORT 65201

void ricezione(int fd_skt, int s_request)
{
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
    server_response[i] = '\0';
    printf("%s\n", server_response);
    free(server_response);
}

void comunicazione(long l, bool richiesta_singola)
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

        e = readn(fd_skt, &s_request, sizeof(int));
        if (e != sizeof(int))
        {
            termina("Errore read");
        }
        ricezione(fd_skt, s_request);
    }
    else /* stampa di tutte le coppie "somma:file" */
    {
        s_request = htonl(-1);

        e = writen(fd_skt, &s_request, sizeof(int));
        if (e != sizeof(int))
        {
            termina("Errore write");
        }

        e = readn(fd_skt, &s_request, sizeof(int));
        if (e != sizeof(int))
        {
            termina("Errore read");
        }
        ricezione(fd_skt, s_request);
    }
    if (close(fd_skt) < 0)
    {
        termina("Errore chiusura socket");
    }
}

int main(int argc, char const *argv[])
{
    if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {
            comunicazione(atol(argv[i]), true);
        }
    }
    else /* richiesta speciale al collector */
        comunicazione(0, false);
    return 0;
}