#ifndef PROBE_H
#define PROBE_H

#include "../core/errors.h"
#include "../core/matrix.h"

CallmStatusCode Probe_init(const char *host, int server_port);

CallmStatusCode Probe_send_matrix(Matrix *M, const char *msg);

#endif  // !#ifndef PROBE_H
