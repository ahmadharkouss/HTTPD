#ifndef SYNTAXERR_H_
#define SYNTAXERR_H_

#define _POSIX_C_SOURCE 200809L

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "my_strcasecmp.h"

typedef struct Header
{
    char *name;
    char *value;
    struct Header *next;
} Header;

typedef struct Request
{
    char *method;
    char *url;
    char *version;
    struct Header *headers;
    char *body;
} Request;

int my_isNumber(char *s);
int my_isdigit(char c);
struct Request *parse_request(const char *raw, int *flag1, int *flag2);
void free_header(struct Header *h);
void free_request(struct Request *req);
int run_test1(char *toparse);
struct Request *run_test2(int *error, char *toparse);
struct Header *get_header(Request *req, char *name);

#endif