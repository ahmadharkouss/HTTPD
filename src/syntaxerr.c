#include "syntaxerr.h"

// check if content lengtth value is a number
int my_isNumber(char *s)
{
    for (int i = 0; s[i] != '\0'; i++)
    {
        if (my_isdigit(s[i]) == 0)
            return 0;
    }
    return 1;
}
int my_isdigit(char c)
{
    if (c >= '0' && c <= '9')
    {
        return 1;
    }
    return 0;
}

// flag1=1 contains host
// flag2=1 contains content-length
struct Request *parse_request(const char *raw, int *flag1, int *flag2)
{
    struct Request *req = NULL;
    req = malloc(sizeof(struct Request));
    if (!req)
    {
        return NULL;
    }
    memset(req, 0, sizeof(struct Request));
    /*
    memset(req->method ,0, sizeof(req->method));
    memset(req->url ,0, sizeof(req->url));
    memset(req->version ,0, sizeof(req->version));
    memset(req->body ,0, sizeof(req->body));
*/

    // Method
    size_t meth_len = strcspn(raw, " ");
    if (memcmp(raw, "GET", strlen("GET")) == 0)
    {
        req->method = "GET";
    }
    else if (memcmp(raw, "HEAD", strlen("HEAD")) == 0)
    {
        req->method = "HEAD";
    }
    else
    {
        req->method = "UNSUPPORTED";
    }
    raw += meth_len + 1;

    // Request-URI
    size_t url_len = strcspn(raw, " ");
    req->url = malloc(url_len + 1);
    if (!req->url)
    {
        free_request(req);
        return NULL;
    }

    memcpy(req->url, raw, url_len);
    req->url[url_len] = '\0';
    raw += url_len + 1;

    // HTTP-Version
    size_t ver_len = strcspn(raw, "\r\n");
    req->version = malloc(ver_len + 1);
    if (!req->version)
    {
        free_request(req);
        return NULL;
    }
    memcpy(req->version, raw, ver_len);
    req->version[ver_len] = '\0';
    raw += ver_len + 2;

    struct Header *header = NULL, *last = NULL;
    while (raw[0] != '\r' && raw[1] != '\n')
    {
        last = header;
        header = malloc(sizeof(Header));
        if (!header)
        {
            free_request(req);
            return NULL;
        }

        // name
        size_t name_len = strcspn(raw, ":");
        header->name = malloc(name_len + 1);
        if (!header->name)
        {
            free_request(req);
            return NULL;
        }
        memcpy(header->name, raw, name_len);
        header->name[name_len] = '\0';
        raw += name_len + 1;
        while (*raw == ' ')
        {
            raw++;
        }
        if (my_strcasecmp(header->name, "Host\0") == 0)
        {
            *flag1 = 1;
        }
        if (my_strcasecmp(header->name, "Content-Length\0") == 0)
        {
            *flag2 = 1;
        }
        // value
        size_t value_len = strcspn(raw, "\r\n");
        header->value = malloc(value_len + 1);
        if (!header->value)
        {
            free_request(req);
            return NULL;
        }
        memcpy(header->value, raw, value_len);
        header->value[value_len] = '\0';
        raw += value_len + 2;

        // next
        header->next = last;
    }
    req->headers = header;
    raw += 2;

    size_t body_len = strlen(raw);
    req->body = malloc(body_len + 1);
    if (!req->body)
    {
        free_request(req);
        return NULL;
    }
    memcpy(req->body, raw, body_len);
    req->body[body_len] = '\0';

    return req;
}

void free_header(struct Header *h)
{
    if (h)
    {
        free(h->name);
        free(h->value);
        free_header(h->next);
        free(h);
    }
}

void free_request(struct Request *req)
{
    free(req->url);
    free(req->version);
    free_header(req->headers);
    free(req->body);
    free(req);
}

// add function to get header
struct Header *get_header(Request *req, char *name)
{
    struct Header *h;
    for (h = req->headers; h; h = h->next)
    {
        if (my_strcasecmp(h->name, name) == 0)
        {
            return h;
        }
    }
    return NULL;
}

int iscolon(char *str)
{
    size_t i = 0;
    while (str[i] != '\0')
    {
        if (str[i] == ':')
        {
            return 1;
        }
        i++;
    }
    return 0;
}

// test1
int run_test1(char *toparse)
{
    // for opti only
    if (toparse[0] == '\n' || toparse[1] == '\n')
    {
        return -1;
    }
    // check for CRLF at random positions
    int j = 0, ctr = 0;
    while (toparse[j + 1] != '\0')
    {
        if (toparse[j] == '\n' && toparse[j + 1] == '\r')
        {
            ctr++;
        }
        j++;
    }
    // case on crlf not separe in body but other not treated
    //  there always one crlrf
    if (ctr > 1)
    {
        return -1;
    }
    // check for empty spaces in request line
    ctr = j = 0;
    while (toparse[j] != '\0' && toparse[j] != '\n')
    {
        if (toparse[j] == ' ')
        {
            ctr++;
        }
        j++;
    }
    if (ctr != 2)
    {
        return -1;
    }

    // check for : in header part
    //  skip request line
    // COPY STRING BEFORE
    char *copy = calloc(strlen(toparse) + 1, sizeof(char));
    if (copy == NULL)
    {
        return -1;
    }
    memcpy(copy, toparse, strlen(toparse));
    char *token2;
    token2 = strtok(copy, "\n");
    if (token2 != NULL)
    {
        token2 = strtok(NULL, "\n");
    }
    if (token2 != NULL && strlen(token2) >= 2)
    {
        if (iscolon(token2) == 0 || token2[0] == ':'
            || token2[strlen(token2) - 2] == ':')
        {
            free(copy);
            return -1;
        }
    }
    while (token2 != NULL)
    {
        token2 = strtok(NULL, "\n");
        if (token2 != NULL)
        {
            // REACHED CONTENT PART
            if (strncmp(token2, "\r", strlen("\r")) == 0)
            {
                break;
            }
            if (strlen(token2) >= 2)
            {
                // test also , if there is empty line between body and headers
                if (iscolon(token2) == 0 || token2[0] == ':'
                    || token2[strlen(token2) - 2] == ':')
                {
                    free(copy);
                    return -1;
                }
            }
        }
    }

    free(copy);
    return 0;
}

// test2

struct Request *run_test2(int *error, char *toparse)
{
    int flag1 = 0;
    int flag2 = 0;
    struct Request *req = parse_request(toparse, &flag1, &flag2);
    // check if methods are in uppercase or empty
    if (strncmp(req->method, "UNSUPPORTED\0", sizeof("UNSUPPORTED\0")) == 0)
    {
        *error = 1;
        return NULL;
    }
    // check if uri is empty
    // check if http version is empty
    // no host header

    if (req->url[0] == 0 || req->version[0] == 0 || flag1 == 0)
    {
        *error = 1;
        return NULL;
    }
    // there is a host header and content lenght header
    else if (flag2 == 1) // check if body has same length//OR IF THER IS BODY
    {
        struct Header *r = get_header(req, "Content-Length");
        if (r != NULL)
        {
            // check if content length is an integer
            if (my_isNumber(r->value) == 0)
            {
                printf("err3\n");
                *error = 1;
                return NULL;
            }

            // there is no body and content length is different than 0
            if (req->body[0] == 0 && r->value[0] != '0')
            {
                *error = 1;
                return NULL;
            }
            // check if body and content length correspond
            size_t x = atoi(r->value);
            if (x != strlen(req->body))
            {
                *error = 1;

                return NULL;
            }
        }
    }
    // ther is no content length, check if ther is body
    else
    {
        if (req->body[0] != 0)
        {
            *error = 1;

            return NULL;
        }
    }
    *error = 0;
    return req;
}

/*
int main(void)
{
    char str[100]="GET /index.html HTTP/1.1\r\nHost:      localhost \r\n\r\n";
    int err1=run_test1(str); printf("%d\n\n",err1);
    int err2=0;
    struct Request *rq = run_test2(&err2,str);
    printf("%d",err2);
}
*/