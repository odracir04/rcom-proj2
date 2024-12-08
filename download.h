#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include<arpa/inet.h>
#include <string.h>

#define PREFIX_LENGTH 6
#define SERVER_PORT 21
#define MAX_RESPONSE 5000

#define WELCOME_CODE "220"
#define PASSWORD_CODE "331"
#define LOGIN_SUCCESS_CODE "230"
#define PASSIVE_MODE_CODE "227"

typedef struct {
    char* user;
    char* password;
    char* host;
    char* url_path;
} URLParameters;

int parseURL(char* url, URLParameters* connection);
int connectToServer(URLParameters connection);
int loginToServer(URLParameters connection, int sockfd);
int passiveMode(int sockfd1, int* sockfd2);
int closeConnection(int sockfd1, int sockfd2);

#endif