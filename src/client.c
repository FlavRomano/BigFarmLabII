#include "client.h"

void comunicazione(long l, bool request_all_pairs)
{
    int fd_skt = 0;
    struct sockaddr_in serv;

    if ((fd_skt = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        termina("Errore creazione socket");

    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    serv.sin_addr.s_addr = inet_addr(HOST);
    if (connect(fd_skt, (struct sockaddr *)&serv, sizeof(serv)) < 0)
        termina("Errore apertura connessione");

    size_t e;
    int request;
    if (request_all_pairs)
    {
        request = htonl(-1);
        e = writen(fd_skt, &request, sizeof(int));
        if (e != sizeof(int))
            termina("Errore invio richiesta di tutte le coppie");
    }
    else
    {
        char *client_long;
        size_t client_long_len = asprintf(&client_long, "%ld", l);
        request = htonl(client_long_len);
        e = writen(fd_skt, &request, sizeof(int));
        if (e != sizeof(int))
            termina("Errore invio richiesta di una coppia");

        for (int i = 0; i < client_long_len; i++)
        {
            int c = htonl(client_long[i]);
            e = writen(fd_skt, &c, sizeof(int));
            if (e != sizeof(int))
                termina("Errore invio cifra del long");
        }
        free(client_long);
    }
    ricezione(fd_skt);
}

void ricezione(int fd_skt)
{
    int net_slen;
    size_t e = readn(fd_skt, &net_slen, sizeof(int));
    if (e != sizeof(int))
        termina("Errore lettura dimensione stringa");

    int len = ntohl(net_slen);
    char *server_response = malloc(len + 1);
    int i, net_char;
    for (i = 0; i < len; i++)
    {
        e = readn(fd_skt, &net_char, sizeof(int));
        if (e != sizeof(int))
            termina("Errore lettura del carattere");

        char c = ntohl(net_char);
        server_response[i] = c;
    }
    server_response[i] = '\0';

    if (close(fd_skt) < 0)
        termina("Errore chiusura socket");

    printf("%s", server_response);
    free(server_response);
}

int main(int argc, char *argv[])
{
    if (argc == 1)
        comunicazione((long)NULL, true);
    else
        for (int i = 1; i < argc; i++)
            comunicazione(atol(argv[i]), false);
    return 0;
}