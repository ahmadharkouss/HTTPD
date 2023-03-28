#ifndef UTILS_SERVER_H_
#define UTILS_SERVER_H_

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

char *my_itoa(int value, char *s);
void write_headers(int client_sock, struct config *conf, char *path, int flag);
int check_file_path(int client_sock, struct config *conf, char *path,
                    struct sockaddr_in client, char *reqline[3], int log);
int after_synt_valid(int client_sock, struct config *conf, char *reqline[3],
                     struct sockaddr_in client, int log);
int check_request_host(int client_sock, Request *rq, struct config *conf,
                       struct sockaddr_in client, char *reqline[3], int log);

#endif
