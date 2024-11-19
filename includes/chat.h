#ifndef CHAT_H
#define CHAT_H

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024
#define USERNAME_SIZE 32

typedef struct
{
  int socket;
  char username[USERNAME_SIZE];
} Client;

void handle_client(int client_socket);
void broadcast_message(const char *message, int exclude_socket);
void private_message(const char *username, const char *message);
void register_user(const char *username, const char *password);
int login_user(const char *username, const char *password);
void handle_command(int client_socket, char *command);

#endif // CHAT_H