#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "chat.h"

void chat(int socket)
{
  char buffer[BUFFER_SIZE];
  while (1)
  {
    fgets(buffer, BUFFER_SIZE, stdin);
    send(socket, buffer, strlen(buffer), 0);
  }
}

void handle_command(int client_socket, char *command)
{
  if (strncmp(command, "/help", 5) == 0)
  {
    printf("Available commands:\n"
           "/register <username> <password> - Register a new user\n"
           "/login <username> <password> - Login with an existing user\n"
           "/all <message> - Send a message to all users\n"
           "/private <username> <message> - Send a private message to a user\n"
           "/help - Show this help message\n");
  }
  else
  {
    printf("Received command from server: %s\n", command);
  }
}

void handle_client(int client_socket)
{
  char buffer[BUFFER_SIZE];
  int bytes_received;

  while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0)
  {
    buffer[bytes_received] = '\0';
    if (buffer[0] == '/')
    {
      handle_command(client_socket, buffer);
    }
  }

  close(client_socket);
}

int main()
{
  int client_socket;
  struct sockaddr_in server_addr;

  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(8080);

  connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

  printf("Connected to the server. Type your messages...\n");

  if (fork() == 0)
  {
    chat(client_socket);
  }
  else
  {
    char buffer[BUFFER_SIZE];
    int bytes_received;
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0)
    {
      buffer[bytes_received] = '\0';
      printf("%s\n", buffer);
    }
  }

  close(client_socket);
  return 0;
}