#include "daemon.h"

struct config *base_conf;
const char *base_file;

int daemon_start(struct config *conf)
{
    FILE *fr = fopen(conf->pid_file, "r+");
    if (fr)
    {
        char c;
        int val = 0;
        while ((c = fgetc(fr)) != EOF)
        {
            c -= '0';
            if (c >= 0 && c <= 9)
                val = val * 10 + c;
            else
            {
                val = -1;
                break;
            }
        }
        fclose(fr);
        if (!kill(val, 0))
            return 1;
    }
    pid_t cpid = fork();
    if (cpid == -1)
        return -1;
    else if (!cpid)
        start_server(conf);
    else
    {
        FILE *fp = fopen(conf->pid_file, "w");
        if (fp == NULL)
            return -1;
        fprintf(fp, "%d", cpid);
        fclose(fp);
    }
    return 0;
}

int daemon_quit(struct config *conf)
{
    FILE *fp = fopen(conf->pid_file, "r");
    if (fp)
    {
        char c;
        int val = 0;
        while ((c = fgetc(fp)) != EOF)
        {
            c -= '0';
            if (c >= 0 && c <= 9)
                val = val * 10 + c;
            else
            {
                val = -1;
                break;
            }
        }
        kill(val, SIGTERM);
        fclose(fp);
        remove(conf->pid_file);
    }
    return 0;
}

void reload(int sig)
{
    printf("sig: %d\n",sig);
    struct config *new = file_parse(base_file);
    if (new->error == 0)
    {
        struct vhost *pstto = base_conf->vh;
        struct vhost *tocpy = new->vh;
        while (tocpy)
        {
            if (tocpy->server_name)
                strcpy(pstto->server_name, tocpy->server_name);
            if (tocpy->ip)
                strcpy(pstto->ip, tocpy->ip);
            if (tocpy->port)
                strcpy(pstto->port, tocpy->port);
            if (tocpy->root_dir)
                strcpy(pstto->root_dir, tocpy->root_dir);
            if (tocpy->default_file)
                strcpy(pstto->default_file, tocpy->default_file);
            tocpy = tocpy->next;
            if (tocpy && !pstto->next)
            {
                pstto->next = malloc(sizeof(struct vhost));
                memset(pstto->next->default_file, 0,
                    sizeof(pstto->next->default_file));
            }
            pstto = pstto->next;
        }
    }
    free_config(new);
}


int daemon_reload(struct config *conf, const char *config_file)
{
    base_conf = conf;
    base_file = config_file;
    signal(SIGUSR1, reload);
    FILE *fp = fopen(conf->pid_file, "r");
    if (fp)
    {
        char c;
        int val = 0;
        while ((c = fgetc(fp)) != EOF)
        {
            c -= '0';
            if (c >= 0 && c <= 9)
                val = val * 10 + c;
            else
            {
                val = -1;
                break;
            }
        }
        kill(val, SIGUSR1);
        fclose(fp);
    }
    return 0;
}

int daemon_restart(struct config *conf)
{
    daemon_quit(conf);
    daemon_start(conf);
    return 0;
}