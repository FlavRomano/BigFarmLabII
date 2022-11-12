#define HOST "127.0.0.1" /* local host */
#define PORT 65201
#include "apilab.h"
#include <stdbool.h>

void ricezione(int fd_skt, int s_request);
void comunicazione(long l, bool request_all_pairs);