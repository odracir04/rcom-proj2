#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <string.h>

#define PREFIX_LENGTH 6

typedef struct {
    char* user;
    char* password;
    char* host;
    char* url_path;
} URLParameters;

int parseURL(char* url, URLParameters* connection);

#endif