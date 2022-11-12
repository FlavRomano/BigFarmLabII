#include "apilab.h"
#include <stdbool.h>

#define _GNU_SOURCE
#define HOST "127.0.0.1" /* local host */
#define PORT 65201

void ricezione(int fd_skt, int s_request);
void comunicazione(long l, bool request_all_pairs);