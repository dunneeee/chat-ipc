#ifndef CHAT_H
#define CHAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define MAX_CLIENTS 30
#define BUFFER_SIZE 1024
#define MAX_USERNAME 32
#define MAX_PASSWORD 32

#define MSG_REGISTER 1
#define MSG_LOGIN 2
#define MSG_BROADCAST 3
#define MSG_PRIVATE 4
#define MSG_INPUT_FILE 5

typedef struct
{
  int type;
  char username[MAX_USERNAME];
  char password[MAX_PASSWORD];
  char message[BUFFER_SIZE];
  char target[MAX_USERNAME];
} Message;

typedef struct
{
  char username[MAX_USERNAME];
  char password[MAX_PASSWORD];
} User;

#endif