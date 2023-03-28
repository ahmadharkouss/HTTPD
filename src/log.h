#ifndef LOG_H_
#define LOG_H_

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

#include "daemon.h"
#include "parsing.h"
#include "server.h"
#include "utils_server.h"

char *get_ip(struct sockaddr_in client);
int log_or_not(struct config *conf);
// int log_file_or_stdout(struct config *conf);
int log_on_file(struct config *conf, char *ip, char *reqline[3],
                char *status_code, int flag);
void log_on_stdout(struct config *conf, char *ip, char *reqline[3],
                   char *status_code, int flag);
#endif