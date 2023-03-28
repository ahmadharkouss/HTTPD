#ifndef DAEMON_H_
#define DAEMON_H_
#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <unistd.h>

#include "parsing.h"
#include "server.h"
#include "utils_server.h"

int daemon_start(struct config *conf);
int daemon_quit(struct config *conf);
int daemon_reload(struct config *conf, const char *config_file);
int daemon_restart(struct config *conf);

#endif