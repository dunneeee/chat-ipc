#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "chat.h"

Client clients[MAX_CLIENTS];
int client_count = 0;

void handle_command(int client_socket, char *command)
{
  char response[BUFFER_SIZE];
  if (strncmp(command, "/register", 9) == 0)
  {
    char username[USERNAME_SIZE], password[USERNAME_SIZE];
    sscanf(command + 10, "%s %s", username, password);
    register_user(username, password);
    snprintf(response, BUFFER_SIZE, "User %s registered successfully.\n", username);
    send(client_socket, response, strlen(response), 0);
  }
  else if (strncmp(command, "/login", 6) == 0)
  {
    char command_copy[BUFFER_SIZE];
    strncpy(command_copy, command, BUFFER_SIZE);
    char *username = strtok(command_copy + 7, " ");
    char *password = strtok(NULL, " ");

    if (username != NULL && password != NULL && login_user(username, password) == 1)
    {
      snprintf(response, BUFFER_SIZE, "User %s logged in successfully.\n", username);
      send(client_socket, response, strlen(response), 0);
      // Save the username in the client structure
      for (int i = 0; i < client_count; i++)
      {
        if (clients[i].socket == client_socket)
        {
          strncpy(clients[i].username, username, USERNAME_SIZE);
          break;
        }
      }
    }
    else
    {
      snprintf(response, BUFFER_SIZE, "Login failed for user %s and password %s.\n", username ? username : "unknown", password ? password : "unknown");
      send(client_socket, response, strlen(response), 0);
    }
  }
  else if (strncmp(command, "/all", 4) == 0)
  {
    broadcast_message(command + 5, client_socket);
  }
  else if (strncmp(command, "/private", 8) == 0)
  {
    char *username = strtok(command + 9, " ");
    char *message = strtok(NULL, "\0");
    private_message(username, message);
  }
  else if (strncmp(command, "/help", 5) == 0)
  {
    snprintf(response, BUFFER_SIZE,
             "Available commands:\n"
             "/register <username> <password> - Register a new user\n"
             "/login <username> <password> - Login with an existing user\n"
             "/all <message> - Send a message to all users\n"
             "/private <username> <message> - Send a private message to a user\n"
             "/help - Show this help message\n");
    send(client_socket, response, strlen(response), 0);
  }
  else
  {
    snprintf(response, BUFFER_SIZE, "Unknown command. Type /help for a list of commands.\n");
    send(client_socket, response, strlen(response), 0);
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
    else if (strncmp(buffer, "/all", 4) == 0)
    {
      broadcast_message(buffer + 5, client_socket);
    }
    else if (strncmp(buffer, "/private", 8) == 0)
    {
      char *username = strtok(buffer + 9, " ");
      char *message = strtok(NULL, "\0");
      private_message(username, message);
    }
  }

  close(client_socket);
}

void broadcast_message(const char *message, int exclude_socket)
{
  for (int i = 0; i < client_count; i++)
  {
    if (clients[i].socket != exclude_socket)
    {
      send(clients[i].socket, message, strlen(message), 0);
    }
  }
}

void private_message(const char *username, const char *message)
{
  for (int i = 0; i < client_count; i++)
  {
    if (strcmp(clients[i].username, username) == 0)
    {
      send(clients[i].socket, message, strlen(message), 0);
      break;
    }
  }
}

int main()
{
  int server_socket, client_socket;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(8080);

  bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
  listen(server_socket, 10);

  printf("Server is listening on port 8080...\n");

  while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) > 0)
  {
    if (client_count < MAX_CLIENTS)
    {
      clients[client_count].socket = client_socket;
      client_count++;
      if (fork() == 0)
      {
        handle_client(client_socket);
        exit(0);
      }
    }
    else
    {
      close(client_socket);
    }
  }

  close(server_socket);
  return 0;
}