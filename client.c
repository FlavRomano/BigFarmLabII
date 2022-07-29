#include "apilab.h"

#define _GNU_SOURCE
#define HOST "127.0.0.1" /* local host */
#define PORT 65201

void comunicazione(int event, long l)
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
            comunicazione(0, atol(argv[i]));
        }
    }
    else /* richiesta speciale al collector */
        comunicazione(1, 0);
    return 0;
}
