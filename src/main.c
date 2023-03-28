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

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Invalid number of arguments\n");
        return 1;
    }
    // parse file
    struct config *test = file_parse(argv[argc - 1]);
    if (test->error != 0)
    {
        fprintf(stderr, "Invalid config file\n");
        int err = test->error;
        free_config(test);
        return err;
    }
    if (strcmp(argv[1], "--dry-run") == 0)
        return 0;
    // test daemon;
    if (strcmp(argv[1], "-a") == 0)
    {
        int i = 2;
        if (strcmp(argv[2], "start") == 0)
            i = daemon_start(test);
        else if (strcmp(argv[2], "stop") == 0)
            i = daemon_quit(test);
        else if (strcmp(argv[2], "reload") == 0)
            i = daemon_reload(test, argv[argc - 1]);
        else if (strcmp(argv[2], "restart") == 0)
            i = daemon_restart(test);
        if (i == 2)
            fprintf(stderr, "Invalid arguments\n");
        else if (i == -1)
            fprintf(stderr, "Error\n");
        free_config(test);
        return i;
    }
    else if (argc > 2)
    {
        fprintf(stderr, "Invalid number of arguments\n");
        return 1;
    }
    else
        return start_server(test);
}