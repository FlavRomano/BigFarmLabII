#include "apilab.h"

#define _GNU_SOURCE
#define HOST "127.0.0.1" /* local host */
#define PORT 65201

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

    if (connect(fd_skt, (struct sockaddr *)&serv, sizeof(serv) < 0))
    {
        termina("Errore apertura connessione\n");
    }
    int dim;
    if (event)
    {
        ;
    }
    else
    {
        ;
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
            comunicazione(atol(argv[i]), 0);
        }
    }
    else /* richiesta speciale al collector */
        comunicazione(0, 1);
    return 0;
}
