#ifndef SERVER_H_
#define SERVER_H_

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "parsing.h"
#include "syntaxerr.h"
#include "utils_server.h"

void respond(int client_sock, struct config *conf, struct sockaddr_in client);
int run_server(struct config *conf);
int start_server(struct config *test);

#endif