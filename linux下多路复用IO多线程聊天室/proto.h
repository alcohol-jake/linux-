#ifndef PROTO_H__
#define PROTO_H__

#define SERVERPORT "2024"
#define MAXSIZE 1024

struct user
{
    int usersd;
    char message[MAXSIZE];
    char username[50];
    char password[50];
    char signature[50];
    char toname[50];
    int type;
};

#endif
