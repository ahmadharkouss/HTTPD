#include "parsing.h"

void free_vhost(struct vhost *vh)
{
    if (vh || vh != NULL)
    {
        free_vhost(vh->next);
        free(vh);
    }
}

void free_config(struct config *config)
{
    if (config)
    {
        if (config->vh)
            free_vhost(config->vh);
        free(config);
    }
}

struct config *file_parse(const char *config_file)
{
    struct config *config = malloc(sizeof(struct config));
    if (!config)
        return NULL;
    config->vh = NULL;
    FILE *fp = fopen(config_file, "r");
    if (fp == NULL)
    {
        config->error = 2;
        return config; // no file
    }
    ssize_t read;
    char *buf = NULL;
    size_t size = 0;
    read = getline(&buf, &size, fp);
    while ((read = getline(&buf, &size, fp)) != -1)
    {
        if (buf[0] != '\n')
        {
            char *name = strtok(buf, " =\n");
            char *des = strtok(NULL, " =\n");
            if (strcmp(name, "pid_file") == 0)
                strcpy(config->pid_file, des);
            else if (strcmp(name, "log_file") == 0)
                strcpy(config->log_file, des);
            else if (strcmp(name, "log") == 0)
                strcpy(config->log, des);
            else if (strcmp(name, "[[vhosts]]") == 0)
                break;
        }
    }
    if (!config->pid_file)
    {
        config->error = 2;
        return config;
    }
    struct vhost *vhost = malloc(sizeof(struct vhost));
    memset(vhost->default_file, 0, sizeof(vhost->default_file));
    config->vh = vhost;
    while ((read = getline(&buf, &size, fp)) != -1)
    {
        if (buf[0] != '\n')
        {
            char *name = strtok(buf, " =\n");
            if (strcmp(name, "[[vhosts]]") == 0)
            {
                if (!vhost->server_name || !vhost->port || !vhost->ip
                    || !vhost->root_dir)
                {
                    config->error = 2;
                    free(buf);
                    return config;
                }
                else
                {
                    vhost->next = malloc(sizeof(struct vhost));
                    vhost = vhost->next;
                    memset(vhost->default_file, 0, sizeof(vhost->default_file));
                }
            }
            else
            {
                char *des = strtok(NULL, " =\n");
                if (strcmp(name, "server_name") == 0)
                    strcpy(vhost->server_name, des);
                else if (strcmp(name, "ip") == 0)
                    strcpy(vhost->ip, des);
                else if (strcmp(name, "port") == 0)
                    strcpy(vhost->port, des);
                else if (strcmp(name, "root_dir") == 0)
                    strcpy(vhost->root_dir, des);
                else if (strcmp(name, "default_file") == 0)
                    strcpy(vhost->default_file, des);
            }
        }
    }
    vhost->next = NULL;
    if (!vhost->server_name || !vhost->port || !vhost->ip || !vhost->root_dir)
        config->error = 2;
    else
        config->error = 0;
    free(buf);
    fclose(fp);
    return config;
}