#define HOST "127.0.0.1"
#define PORT 65201
#include "apilab.h"
#include <stdbool.h>

void ricezione(int fd_skt);
void comunicazione(long l, bool request_all_pairs);