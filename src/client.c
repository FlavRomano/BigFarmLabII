#include "/home/local/ADUNIPI/f.romano39/BigFarmLabII/include/apilab.h"
#include <stdbool.h>

#define _GNU_SOURCE
#define HOST "127.0.0.1" /* local host */
#define PORT 65201

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

    int request;
    if (richiesta_singola)
    {
        size_t send_long_len = snprintf(NULL, 0, "%ld", l);
        char send_long[send_long_len];
        sprintf(send_long, "%ld", l);
        request = htonl(send_long_len);

        e = writen(fd_skt, &request, sizeof(int));
        if (e != sizeof(int))
        {
            termina("Errore write");
        }
        for (int i = 0; i < send_long_len; i++)
        {
            int c = htonl(send_long[i]);
            e = writen(fd_skt, &c, sizeof(int));
            if (e != sizeof(int))
            {
                termina("Errore write");
            }
        }

        e = readn(fd_skt, &request, sizeof(int));
        if (e != sizeof(int))
        {
            termina("Errore read");
        }
        int n = ntohl(request);
        char *buffer = malloc(n);
        int i;
        for (i = 0; i < n; i++)
        {
            e = readn(fd_skt, &request, sizeof(int));
            if (e != sizeof(int))
            {
                termina("Errore read");
            }
            char c = ntohl(request);
            buffer[i] = c;
        }
        buffer[i] = '\0';
        printf("%s\n", buffer);
        free(buffer);
    }
    else /* stampa di tutte le coppie "somma:file" */
    {
        request = htonl(-1);

        e = writen(fd_skt, &request, sizeof(int));
        if (e != sizeof(int))
        {
            termina("Errore write");
        }

        e = readn(fd_skt, &request, sizeof(int));
        if (e != sizeof(int))
        {
            termina("Errore read");
        }

        int n = ntohl(request);
        char *s;
        s = malloc(n + 1);
        int i;
        for (i = 0; i < n; i++)
        {
            e = readn(fd_skt, &request, sizeof(int));
            if (e != sizeof(int))
            {
                termina("Errore read");
            }
            char c = ntohl(request);
            s[i] = c;
        }
        s[i] = '\0';
        printf("%s", s);
        free(s);
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