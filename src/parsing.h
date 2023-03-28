#ifndef PARSING_H_
#define PARSING_H_
#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct vhost
{
    char server_name[64];
    char ip[32];
    char root_dir[64];
    char port[64];
    char default_file[64];
    struct vhost *next;
};

struct config
{
    int error;
    char pid_file[64];
    char log_file[64];
    char log[64];
    struct vhost *vh;
};

void free_config(struct config *config);
struct config *file_parse(const char *config_file);

#endif