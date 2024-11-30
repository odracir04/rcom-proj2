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

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    }

    URLParameters connection = {NULL, NULL, NULL, NULL};
    if (parseURL(argv[1], &connection) < 0) {
        printf("ERROR: Could not parse URL\n");
        exit(-1);
    }

    printf("User: %s\n", connection.user ? connection.user : "(null)");
    printf("Password: %s\n", connection.password ? connection.password : "(null)");
    printf("Host: %s\n", connection.host ? connection.host : "(null)");
    printf("URL Path: %s\n", connection.url_path ? connection.url_path : "(null)");

    // connectToServer

    // loginHost

    // getPath

    // getFile (return success or unsuccess)

    return 0;
}