#include "utils_server.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
char *my_itoa(int value, char *s)
{
    if (value == 0)
    {
        s[0] = '0';
        s[1] = '\0';
        return s;
    }
    int len = 1;
    int flag = 0;
    if (value < 0)
    {
        value = -1 * value;
        len++;
        flag = 1;
    }
    int i = value;
    while (i / 10 > 0)
    {
        i = i / 10;
        len++;
    }
    s[len] = '\0';
    while (len > 0)
    {
        s[len - 1] = value % 10 + '0';
        value = value / 10;
        len--;
    }
    if (flag == 1)
        s[0] = '-';
    return s;
}

void write_headers(int client_sock, struct config *conf, char *path, int flag)
{
    // server name
    send(client_sock, "Server: ", strlen("Server: "), MSG_NOSIGNAL);
    send(client_sock, conf->vh->server_name, strlen(conf->vh->server_name),
         MSG_NOSIGNAL);
    send(client_sock, "\r\n", strlen("\r\n"), MSG_NOSIGNAL);
    // Date
    send(client_sock, "Date: ", strlen("Date: "), MSG_NOSIGNAL);
    time_t t;
    struct tm *tmp;
    char MY_TIME[100];
    time(&t);
    tmp = gmtime(&t);
    strftime(MY_TIME, sizeof(MY_TIME), "%a, %e %b %Y %H:%M:%S GMT", tmp);
    send(client_sock, MY_TIME, strlen(MY_TIME), MSG_NOSIGNAL);
    send(client_sock, "\r\n", strlen("\r\n"), MSG_NOSIGNAL);
    // content length //default = 0
    // add content length
    send(client_sock, "Content-Length: ", strlen("Content-Length: "),
         MSG_NOSIGNAL);
    // Get case
    if (flag == 1)
    {
        // Get content length
        FILE *fdi;
        fdi = fopen(path, "r");
        fseek(fdi, 0L, SEEK_END);
        int length = ftell(fdi);
        fclose(fdi);
        char slength[100];
        sprintf(slength, "%d", length);
        send(client_sock, slength, strlen(slength), MSG_NOSIGNAL);
    }
    // Other cases
    else
    {
        send(client_sock, "0", strlen("0"), MSG_NOSIGNAL);
    }
    send(client_sock, "\r\n", strlen("\r\n"), MSG_NOSIGNAL);
    // Connection close
    send(client_sock, "Connection: close\r\n", strlen("Connection: close\r\n"),
         MSG_NOSIGNAL);
}

int check_file_path(int client_sock, struct config *conf, char *path,
                    struct sockaddr_in client, char *reqline[3], int log)
{
    FILE *fdi;
    fdi = fopen(path, "r");
    if (fdi == NULL)
    {
        // file do not exist
        if (errno == ENOENT)
        {
            send(client_sock, "HTTP/1.1 404 Not Found\r\n",
                 strlen("HTTP/1.1 404 Not Found\r\n"), MSG_NOSIGNAL);
            write_headers(client_sock, conf, path, 0);
            if (log == 1)
            {
                char string2[] = "404";
                log_on_stdout(conf, get_ip(client), reqline, string2, 1);
            }
        }
        // not enough permissions
        if (errno == EACCES)
        {
            send(client_sock, "HTTP/1.1 403 Forbidden\r\n",
                 strlen("HTTP/1.1 403 Forbidden\r\n"), MSG_NOSIGNAL);
            write_headers(client_sock, conf, path, 0);
            if (log == 1)
            {
                char string2[] = "403";
                log_on_stdout(conf, get_ip(client), reqline, string2, 1);
            }
        }
        return 1;
    }
    fclose(fdi);
    return 0;
}
// Funtion removing spaces from string
char *removeSpacesFromStr(char *string)
{
    // non_space_count to keep the frequency of non space characters
    int non_space_count = 0;

    // Traverse a string and if it is non space character then, place it at
    // index non_space_count
    for (int i = 0; string[i] != '\0'; i++)
    {
        if (string[i] != ' ')
        {
            string[non_space_count] = string[i];
            non_space_count++; // non_space_count incremented
        }
    }

    // Finally placing final character at the string end
    string[non_space_count] = '\0';
    return string;
}

int check_request_host(int client_sock, Request *rq, struct config *conf,
                       struct sockaddr_in client, char *reqline[3], int log)
{
    struct Header *r = get_header(rq, "Host");

    if (r != NULL)
    {
        char *str = r->value;
        str = removeSpacesFromStr(str);
        const char s[2] = ":";
        char *token;
        // get ip address
        token = strtok(str, s);
        int flag = 0;
        if (token != NULL)
        {
            if (strncmp(token, "localhost\0", 11) == 0)
            {
                if (strncmp(conf->vh->ip, "127.0.0.1\0", 11) != 0)
                {
                    send(client_sock, "HTTP/1.1 400 Bad Request\r\n",
                         strlen("HTTP/1.1 400 Bad Request\r\n"), MSG_NOSIGNAL);
                    write_headers(client_sock, conf, NULL, 0);
                    if (log == 1)
                    {
                        char string2[] = "400";
                        log_on_stdout(conf, get_ip(client), reqline, string2,
                                      1);
                    }
                    return 1;
                }
                flag = 1;
            }

            if (strncmp(token, conf->vh->ip, strlen(token)) != 0 && flag == 0
                && strncmp(token, conf->vh->server_name, strlen(token)) != 0)
            {
                send(client_sock, "HTTP/1.1 400 Bad Request\r\n",
                     strlen("HTTP/1.1 400 Bad Request\r\n"), MSG_NOSIGNAL);
                write_headers(client_sock, conf, NULL, 0);
                if (log == 1)
                {
                    char string2[] = "400";
                    log_on_stdout(conf, get_ip(client), reqline, string2, 1);
                }
                return 1;
            }
        }

        // if there is port number , check if it martch the server port
        token = strtok(NULL, s);
        if (token != NULL)
        {
            if (strncmp(token, conf->vh->port, strlen(token)) != 0 && flag == 0)
            {
                send(client_sock, "HTTP/1.1 400 Bad Request\r\n",
                     strlen("HTTP/1.1 400 Bad Request\r\n"), MSG_NOSIGNAL);
                write_headers(client_sock, conf, NULL, 0);
                if (log == 1)
                {
                    char string2[] = "400";
                    log_on_stdout(conf, get_ip(client), reqline, string2, 1);
                }
                return 1;
            }
        }

        return 0;
    }
    return 0;
}

// second test after first parsing
int after_synt_valid(int client_sock, struct config *conf, char *reqline[3],
                     struct sockaddr_in client, int log)
{
    // Method not supported
    if (strncmp(reqline[0], "GET\0", 4) != 0
        && strncmp(reqline[0], "HEAD\0", 5) != 0)
    {
        send(client_sock, "HTTP/1.1 405 Method Not Allowed\r\n",
             strlen("HTTP/1.1 405 Method Not Allowed\r\n"), MSG_NOSIGNAL);
        write_headers(client_sock, conf, NULL, 0);
        if (log == 1)
        {
            char string2[] = "405";
            log_on_stdout(conf, get_ip(client), reqline, string2, 1);
        }
        return 1;
    }
    // Version not supported
    if (strncmp(reqline[2], "HTTP/1.1", 8) != 0)
    {
        send(client_sock, "HTTP/1.1 505 HTTP Version Not Supported\r\n",
             strlen("HTTP/1.1 505 HTTP Version Not Supported\r\n"),
             MSG_NOSIGNAL);
        write_headers(client_sock, conf, NULL, 0);
        if (log == 1)
        {
            char string2[] = "505";
            log_on_stdout(conf, get_ip(client), reqline, string2, 1);
        }
        return 1;
    }
    return 0;
}