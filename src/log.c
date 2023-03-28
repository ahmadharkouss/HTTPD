#include "log.h"

#include <arpa/inet.h>

// get ip address

char *get_ip(struct sockaddr_in client)
{
    struct sockaddr_in *pV4Addr = (struct sockaddr_in *)&client;
    struct in_addr ipAddr = pV4Addr->sin_addr;
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);
    char *res = str;
    return res;
}

// check if there is error when it's null
int log_or_not(struct config *conf)
{
    if (strncmp(conf->log, "false", strlen(conf->log)) == 0)
    {
        return 0;
    }
    // default and yes
    return 1;
}

/*
int log_file_or_stdout(struct config *conf)
{
    if(log_or_not(conf)==1)
    {
        int d=check_file(conf->log_file);
        if(d==1)
        {
            return 1;//log on file//file exists
        }
        return 2;//log on stdout//there is no file
    }
    return -1;

}
*/
// test if there is no log file or value

// add char *ip to function respond

int log_on_file(struct config *conf, char *ip, char *reqline[3],
                char *status_code, int flag)
{
    FILE *fp;
    fp = fopen(conf->log_file, "w");
    if (fp == NULL)
    {
        return -1;
    }
    fseek(fp, 100, SEEK_SET);

    // get time
    time_t t;
    struct tm *tmp;
    char MY_TIME[100];
    time(&t);
    tmp = gmtime(&t);
    strftime(MY_TIME, sizeof(MY_TIME), "%a, %e %b %Y %H:%M:%S GMT", tmp);

    if (flag == 0)
    {
        fputs(MY_TIME, fp);
        // add server name
        fputs(" [", fp);
        fputs(conf->vh->server_name, fp);
        fputs("] received ", fp);
        fputs(reqline[0], fp);
        fputs(" on ", fp);
        fputs("'", fp);
        fputs(reqline[1], fp);
        fputs("'", fp);
        fputs(" from ", fp);
        fputs(ip, fp);
        fputs("\n", fp);
        fclose(fp);
        return 0;
    }
    if (strncmp(status_code, "400", strlen(status_code)) == 0)
    {
        fputs(MY_TIME, fp);
        // add server name
        fputs(" [", fp);
        fputs(conf->vh->server_name, fp);
        fputs("] responding with", fp);
        fputs(" %s ", fp);
        fputs("to ", fp);
        fputs(ip, fp);
        fputs("\n", fp);
    }
    else if (strncmp(status_code, "405", strlen(status_code)) == 0)
    {
        fputs(MY_TIME, fp);
        // add server name
        fputs(" [", fp);
        fputs(conf->vh->server_name, fp);
        fputs("] responding with", fp);
        fputs(" %s ", fp);
        fputs("to ", fp);
        fputs(ip, fp);
        fputs(" for ", fp);
        fputs("UNKNOWN", fp);
        fputs(" on ", fp);
        fputs(reqline[1], fp);
        fputs("\n", fp);
    }
    else
    {
        fputs(MY_TIME, fp);
        // add server name
        fputs(" [", fp);
        fputs(conf->vh->server_name, fp);
        fputs("] responding with", fp);
        fputs(" %s ", fp);
        fputs("to ", fp);
        fputs(ip, fp);
        fputs(" for ", fp);
        fputs(reqline[0], fp);
        fputs(" on ", fp);
        fputs(reqline[1], fp);
        fputs("\n", fp);
    }
    fclose(fp);
    return 0;
}

// 0 for request
// 1 for response

void log_on_stdout(struct config *conf, char *ip, char *reqline[3],
                   char *status_code, int flag)
{
    // get date
    time_t t;
    struct tm *tmp;
    char MY_TIME[100];
    time(&t);
    tmp = gmtime(&t);
    strftime(MY_TIME, sizeof(MY_TIME), "%a, %e %b %Y %H:%M:%S GMT", tmp);
    // request
    if (flag == 0)
    {
        printf("%s [%s] received %s on '%s' from %s\n", MY_TIME,
               conf->vh->server_name, reqline[0], reqline[1], ip);
        return;
    }
    // response
    // depends opn status code
    if (strncmp(status_code, "400", strlen(status_code)) == 0)
    {
        printf("%s [%s] responding with %s to %s\n", MY_TIME,
               conf->vh->server_name, status_code, ip);
    }
    else if (strncmp(status_code, "405", strlen(status_code)) == 0)
    {
        printf("%s [%s] responding with %s to %s for UNKNOWN on '%s'\n",
               MY_TIME, conf->vh->server_name, status_code, ip, reqline[1]);
    }
    else
    {
        printf("%s [%s] responding with %s to %s for %s on '%s'\n", MY_TIME,
               conf->vh->server_name, status_code, ip, reqline[0], reqline[1]);
    }
}

/*

int log(struct sockaddr_in client)
{

}

*/

/*
//try with empty log
int main(void)
{
    struct config *test=file_parse("tests/server.conf");
    int d=log_or_not(test->log);

}
*/