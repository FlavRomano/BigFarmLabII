#include "xerrori.h"

// collezione di chiamate a funzioni di sistema con controllo output
// i prototipi sono in xerrori.h

// termina un processo con eventuale messaggio d'errore
void termina(const char *messaggio)
{
    if (errno == 0)
        fprintf(stderr, "== %d == %s\n", getpid(), messaggio);
    else
        fprintf(stderr, "== %d == %s: %s\n", getpid(), messaggio, strerror(errno));
    exit(1);
}

// termina un processo con eventuale messaggio d'errore + linea e file
void xtermina(const char *messaggio, int linea, char *file)
{
    if (errno == 0)
        fprintf(stderr, "== %d == %s\n", getpid(), messaggio);
    else
        fprintf(stderr, "== %d == %s: %s\n", getpid(), messaggio,
                strerror(errno));
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);

    exit(1);
}