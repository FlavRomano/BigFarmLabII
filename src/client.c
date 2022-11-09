#include "/home/local/ADUNIPI/f.romano39/BigFarmLabII/include/apilab.h"

#define _GNU_SOURCE
#define HOST "127.0.0.1" /* local host */
#define PORT 65203

void comunicazione(long l, int event)
{
    int fd_skt;
    struct sockaddr_in serv;
    size_t e;
    fd_skt = 0;

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
    int tmp;
    if (event)
    {
        char s[1024];
        int s_len = sprintf(s, "%ld", l);
        tmp = htonl(s_len);

        e = writen(fd_skt, &tmp, sizeof(int));
        if (e != sizeof(int))
        {
            termina("Errore write\n");
        }
        for (int i = 0; i < s_len; i++)
        {
            int c = htonl(s[i]);
            e = writen(fd_skt, &c, sizeof(int));
            if (e != sizeof(int))
            {
                termina("Errore write\n");
            }
        }

        e = readn(fd_skt, &tmp, sizeof(int));
        if (e != sizeof(int))
        {
            termina("Errore read\n");
        }
        int n = ntohl(tmp);
        int i;
        for (i = 0; i < n; i++)
        {
            e = readn(fd_skt, &tmp, sizeof(int));
            if (e != sizeof(int))
            {
                termina("Errore read\n");
            }
            char c = ntohl(tmp);
            s[i] = c;
        }
        s[i] = '\0';
        printf("%s\n", s);
    }
    else /* stampa di tutte le coppie "somma:file" */
    {
        int dim = -1;
        tmp = htonl(dim);

        e = writen(fd_skt, &tmp, sizeof(int));
        if (e != sizeof(int))
        {
            termina("Errore write\n");
        }

        e = readn(fd_skt, &tmp, sizeof(int));
        if (e != sizeof(int))
        {
            termina("Errore read\n");
        }

        int n = ntohl(tmp);
        char *s;
        s = malloc(n + 1);
        int i;
        for (i = 0; i < n; i++)
        {
            e = readn(fd_skt, &tmp, sizeof(int));
            if (e != sizeof(int))
            {
                termina("Errore read");
            }
            char c = ntohl(tmp);
            s[i] = c;
        }
        s[i] = '\0';
        printf("%s", s);
        free(s);
    }
    if (close(fd_skt) < 0)
    {
        termina("Errore chiusura socket\n");
    }
}

int main(int argc, char const *argv[])
{
    if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {
            comunicazione(atol(argv[i]), 1);
        }
    }
    else /* richiesta speciale al collector */
        comunicazione(0, 0);
    return 0;
}