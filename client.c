#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define _GNU_SOURCE
#define HOST "127.0.0.1" /* local host */
#define PORT 65201

ssize_t readn(int fd, void *ptr, size_t n)
{
    size_t nleft;
    ssize_t nread;

    nleft = n;
    while (nleft > 0)
    {
        if ((nread = read(fd, ptr, nleft)) < 0)
        {
            if (nleft == n)
                return -1; /* error, return -1 */
            else
                break; /* error, return amount read so far */
        }
        else if (nread == 0)
            break; /* EOF */
        nleft -= nread;
        ptr += nread;
    }
    return (n - nleft); /* return >= 0 */
}

/* Write "n" bytes to a descriptor */
ssize_t writen(int fd, void *ptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;

    nleft = n;
    while (nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) < 0)
        {
            if (nleft == n)
                return -1; /* error, return -1 */
            else
                break; /* error, return amount written so far */
        }
        else if (nwritten == 0)
            break;
        nleft -= nwritten;
        ptr += nwritten;
    }
    return (n - nleft); /* return >= 0 */
}

void comunicazione(int event, long num)
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

    if (event)
    {
        int dim;
        dim = htonl(dim);
        e = writen(fd_skt, &dim, sizeof(int));
        if (e != sizeof(int))
        {
            termina("Errore write\n");
        }
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
