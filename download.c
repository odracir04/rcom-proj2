#include "download.h"

int parseURL(char* url, URLParameters* connection) {
    const char *prefix = "ftp://";
    if (strncmp(url, prefix, PREFIX_LENGTH) != 0) {
        printf("ERROR: Could not resolve protocol (was not ftp://)\n");
        return -1;
    }

    const char *remaining = url + PREFIX_LENGTH;

    const char *at_sign = strchr(remaining, '@');
    if (at_sign) {
        char *user_pass = strndup(remaining, at_sign - remaining);
        char *colon = strchr(user_pass, ':');
        if (colon) {
            *colon = '\0';
            connection->user = strdup(user_pass);
            connection->password = strdup(colon + 1);
        } else {
            connection->user = strdup(user_pass);
            connection->password = strdup("\0");
        }
        remaining = at_sign + 1;
    } else {
        connection->user = strdup("anonymous");
        connection->password = strdup("anonymous");
    }

    const char *slash = strchr(remaining, '/');
    if (slash) {
        connection->host = strndup(remaining, slash - remaining);
        connection->url_path = strdup(slash + 1);
    } else {
        connection->host = strdup(remaining);
        connection->url_path = strdup("\0");
    }

    return 0;
}

int connectToServer(URLParameters connection) {
    struct hostent *h;
    int sockfd;
    struct sockaddr_in server_addr;

    if ((h = gethostbyname(connection.host)) == NULL) {
        printf("ERROR: Could not resolve host\n");
        return -1;
    }

    char* ip_addr = inet_ntoa(*((struct in_addr *) h->h_addr));
    printf("Resolved host, IP is %s\n", ip_addr);

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    server_addr.sin_port = htons(SERVER_PORT);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("ERROR: Could not open socket\n");
        return -1;
    }

    if (connect(sockfd,(struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("ERROR: Could not connect to host\n");
        return -1;
    }

    sleep(1);
    char response[MAX_RESPONSE] = {0};
    
    read(sockfd, response, MAX_RESPONSE);

    printf("%s\n", response);

    if (strncmp(WELCOME_CODE, response, 3) != 0) {
        printf("ERROR: Did not respond with 220\n");
        return -1;
    }

    return sockfd;
}

int loginToServer(URLParameters connection, int sockfd) {
    char user[6 + strlen(connection.user)];
    char password[6 + strlen(connection.password)];
    char response[MAX_RESPONSE] = {0};
    
    strcpy(user, "USER ");
    strcat(user, connection.user);
    user[strlen(connection.user) + 5] = '\n';

    strcpy(password, "PASS ");
    strcat(password, connection.password);
    password[strlen(connection.password) + 5] = '\n';

    write(sockfd, user, strlen(user));
    printf("%s\n", user);
    sleep(1);

    read(sockfd, response, MAX_RESPONSE);
    printf("%s\n", response);

    if (strncmp(PASSWORD_CODE, response, 3) != 0) {
        printf("ERROR: Did not respond with 331\n");
        return -1;
    }

    write(sockfd, password, strlen(password));
    printf("%s\n", password);
    sleep(1);

    read(sockfd, response, MAX_RESPONSE);
    printf("%s\n", response);

    if (strncmp(LOGIN_SUCCESS_CODE, response, 3) != 0) {
        printf("ERROR: Did not respond with 230\n");
        return -1;
    }
    
    return 0;
}

int passiveMode(int sockfd1, int* sockfd2) {
    char ip_addr[20];
    int port;
    char response[MAX_RESPONSE] = {0};
    int num1, num2, num3, num4, num5, num6;
    struct sockaddr_in server_addr;

    write(sockfd1, "pasv\n", 5);
    sleep(1);

    read(sockfd1, response, MAX_RESPONSE);
    printf("%s\n", response);

    if (strncmp(PASSIVE_MODE_CODE, response, 3) != 0) {
        printf("ERROR: Did not respond with 230\n");
        return -1;
    }

    if (sscanf(response, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).", 
               &num1, &num2, &num3, &num4, &num5, &num6) == 6) {
        sprintf(ip_addr, "%d.%d.%d.%d", num1, num2, num3, num4);
        port = 256 * num5 + num6;
        printf("Connection to open at address %s, port %d\n", ip_addr, port);   
    } 

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    server_addr.sin_port = htons(SERVER_PORT);

    if ((*sockfd2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("ERROR: Could not open socket\n");
        return -1;
    }

    if (connect(*sockfd2,(struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("ERROR: Could not connect to host\n");
        return -1;
    }
    
    printf("Connection opened at address %s, port %d\n", ip_addr, port);
    return 0;
}

int closeConnection(int sockfd1, int sockfd2) {
    if (close(sockfd1)<0) {
        printf("ERROR: Could not close socket\n");
        return -1;
    }

    if (close(sockfd2)<0) {
        printf("ERROR: Could not close socket\n");
        return -1;
    }

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    }

    URLParameters connection = {NULL, NULL, NULL, NULL};
    int sockfd1, sockfd2;

    if (parseURL(argv[1], &connection) < 0) {
        printf("ERROR: Could not parse URL\n");
        exit(-1);
    }

    if ((sockfd1 = connectToServer(connection)) < 0) {
        printf("ERROR: Could not connect to FTP server\n");
        exit(-1);
    }

    if (loginToServer(connection, sockfd1) < 0) {
        printf("ERROR: Could not log into FTP server\n");
        exit(-1);
    }

    if (passiveMode(sockfd1, &sockfd2) < 0) {
        printf("ERROR: Could not enter passive mode\n");
        exit(-1);
    }

    // getPath

    // getFile (return success or unsuccess)

    if (closeConnection(sockfd1, sockfd2) < 0) {
        printf("ERROR: Could not close connection\n");
        exit(-1);
    }

    return 0;
}