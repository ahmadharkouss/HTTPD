#include "server.h"

#include <sys/stat.h>

#include "log.h"
// client connection
int isDirectoryExists(const char *path)
{
    struct stat stats;

    stat(path, &stats);

    // Check for file existence
    if (S_ISDIR(stats.st_mode))
        return 1;

    return 0;
}
int is_absolute_test1(char *path, struct config *conf)
{
    char str[100] = "http://";
    strncat(str, conf->vh->ip, strlen(conf->vh->ip));
    strncat(str, ":dd", 1);
    strncat(str, conf->vh->port, strlen(conf->vh->port));
    if (strncmp(str, path, strlen(str)) == 0)
    {
        if (strlen(str) == strlen(path))
        {
            return 2;
        }

        return 1;
    }
    return 0;
}

void respond(int client_sock, struct config *conf, struct sockaddr_in client)
{
    int log1 = log_or_not(conf);
    char mesg[99999], *reqline[3], data_to_send[1024], path[99999];
    int rcvd, fd, bytes_read;

    memset((void *)mesg, (int)'\0', 99999);

    rcvd = recv(client_sock, mesg, 99999, 0);

    if (rcvd < 0) // receive error
    {
        fprintf(stderr, ("recv() error\n"));
        return;
    }
    else if (rcvd == 0) // receive socket closed
    {
        fprintf(stderr, "Client disconnected upexpectedly.\n");
        return;
    }
    else // message received
    {
        // run first test on message(lyxical syntax)=parsing
        // do not modifie message
        int err1 = 0, err2 = 0;
        err1 = run_test1(mesg);
        if (err1 == -1)
        {
            send(client_sock, "HTTP/1.1 400 Bad Request\r\n",
                 strlen("HTTP/1.1 400 Bad Request\r\n"), MSG_NOSIGNAL);
            write_headers(client_sock, conf, NULL, 0);
            return;
        }
        // parse and run second test on lyxical error
        struct Request *rq = run_test2(&err2, mesg);
        if (err2 == 1)
        {
            send(client_sock, "HTTP/1.1 400 Bad Request\r\n",
                 strlen("HTTP/1.1 400 Bad Request\r\n"), MSG_NOSIGNAL);
            write_headers(client_sock, conf, NULL, 0);
            if (rq != NULL)
            {
                free_request(rq);
            }
            return;
        }
        // reparse , faster and no memory leaks
        reqline[0] = strtok(mesg, " "); // method
        reqline[1] = strtok(NULL, " "); // resource name
        reqline[2] = strtok(NULL, "\r\n"); // http version
        char string1[] = "0";
        if (log1 == 1)
        {
            log_on_stdout(conf, get_ip(client), reqline, string1, 0);
        }
        // check if it's an absolute form
        // check if it's an absolute form
        int isabs = is_absolute_test1(reqline[1], conf);
        if (isabs == 1)
        {
            char *token;
            token = strtok(reqline[1], ":");
            if (token != NULL)
            {
                token = strtok(NULL, ":");
            }
            if (token != NULL)
            {
                token = strtok(NULL, "/");
                if (token != NULL)
                {
                    token = strtok(NULL, "/");
                }
                if (token != NULL)
                {
                    strncat(conf->vh->root_dir, "/dd", 1);
                    strcpy(path, conf->vh->root_dir);
                    strcpy(&path[strlen(conf->vh->root_dir)], token);
                }
            }
        }

        // check for host connection errors
        if (isabs == 0)
        {
            int err3 = check_request_host(client_sock, rq, conf, client,
                                          reqline, log1);
            if (err3 == 1)
            {
                if (rq != NULL)
                {
                    free_request(rq);
                }
                return;
            }
            if (rq != NULL)
            {
                free_request(rq);
            }
        }

        // run second test on message

        int d = after_synt_valid(client_sock, conf, reqline, client, log1);
        if (d == 1)
            return;

        // Valid Get or Head method

        // METHOD = HEAD OR METHOD = GET
        // GET FILE PATH

        if (isabs == 0)
        {
            int flag2 = 0;
            if ((reqline[1][0]) != '/')
            {
                strncat(conf->vh->root_dir, "/dd", 1);
                flag2 = 1;
            }
            strcpy(path, conf->vh->root_dir);
            strcpy(&path[strlen(conf->vh->root_dir)], reqline[1]);
            // check if request is a direcotry --> default file
            if (isDirectoryExists(path) || isabs == 2)
            {
                if ((conf->vh->default_file[0]) != '/' && flag2 == 0)
                {
                    strncat(conf->vh->root_dir, "/dd", 1);
                    strcpy(path, conf->vh->root_dir);
                }

                reqline[1] = conf->vh->default_file;
                strcpy(&path[strlen(conf->vh->root_dir)], reqline[1]);
            }

            // not a directory , check if its a file
            //  check if file exists or have enough permissions
            int r =
                check_file_path(client_sock, conf, path, client, reqline, log1);
            if (r == 1)
                return;
        }

        if ((fd = open(path, O_RDONLY)) != -1) // FILE FOUND
        {
            // status line
            send(client_sock, "HTTP/1.1 200 OK\r\n",
                 strlen("HTTP/1.1 200 OK\r\n"), MSG_NOSIGNAL);
            write_headers(client_sock, conf, path, 1);
            if (log1 == 1)
            {
                char string2[] = "200";
                log_on_stdout(conf, get_ip(client), reqline, string2, 1);
            }
            // ADD content in get case
            if (strncmp(reqline[0], "GET\0", 4) == 0)
            {
                write(client_sock, "\r\n", strlen("\r\n"));
                while ((bytes_read = read(fd, data_to_send, 1024)) > 0)
                {
                    send(client_sock, data_to_send, bytes_read, MSG_NOSIGNAL);
                }
            }
        }
        close(fd);
    }
}

int run_server(struct config *conf)
{
    if (conf->error == 2)
    {
        return 2;
    }
    if (conf->vh->default_file && (conf->vh->default_file[0]) == 0)
    {
        strcpy(conf->vh->default_file, "/index.html");
    }
    struct addrinfo hints, *res, *list;

    // getaddrinfo for host
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int error = getaddrinfo(conf->vh->ip, conf->vh->port, &hints, &res);
    if (error == -1)
    {
        return -1;
        // add to consol log
    }
    // socket and bind
    int sockfd;
    for (list = res; list != NULL; list = list->ai_next)
    {
        sockfd = socket(list->ai_family, list->ai_socktype, list->ai_protocol);
        if (sockfd == -1)
        {
            continue;
        }
        if (bind(sockfd, list->ai_addr, list->ai_addrlen) != -1)
        {
            break;
            // bind success
        }
    }

    if (list == NULL) // failed to find
    {
        // add log
        return -1;
    }
    freeaddrinfo(list);
    return sockfd;
}

int start_server(struct config *test)
{
    int listening_sock = run_server(test);
    if (listening_sock == -1 || listening_sock == 2)
    {
        return 2;
    }
    if (listen(listening_sock, 1000000) == -1)
    {
        return 2; // OR 3
    }

    while (1)
    {
        // communication
        struct sockaddr_in client;
        size_t c = sizeof(struct sockaddr_in);
        int client_sock =
            accept(listening_sock, (struct sockaddr *)&client, (socklen_t *)&c);
        if (client_sock == -1)
        {
            continue;
        }
        respond(client_sock, test, client);
        close(client_sock);
    }
    if (test != NULL)
    {
        free_config(test);
    }
    close(listening_sock);
    return 0;
}