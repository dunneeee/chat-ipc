#include "chat.h"
#include <stdio.h>

#define PORT 8888
#define USER_FILE "users.dat"

typedef struct
{
  int socket;
  char username[MAX_USERNAME];
  int authenticated;
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;

void save_user(User *user)
{
  FILE *fp = fopen(USER_FILE, "ab");
  if (fp)
  {
    fwrite(user, sizeof(User), 1, fp);
    fclose(fp);
    printf("User %s saved successfully\n", user->username);
  }
  else
  {
    printf("Failed to open user file for saving\n");
  }
}

int check_user(const char *username, const char *password)
{
  FILE *fp = fopen(USER_FILE, "rb");
  if (!fp)
  {
    printf("Failed to open user file for reading\n");
    return 0;
  }

  User user;
  while (fread(&user, sizeof(User), 1, fp))
  {
    if (strcmp(user.username, username) == 0 &&
        strcmp(user.password, password) == 0)
    {
      fclose(fp);
      printf("User %s authenticated successfully\n", username);
      return 1;
    }
  }
  fclose(fp);
  printf("Authentication failed for user %s\n", username);
  return 0;
}

void broadcast_message(const char *sender, const char *message)
{
  char buffer[BUFFER_SIZE];
  snprintf(buffer, BUFFER_SIZE, "%s: %s", sender, message);

  printf("Broadcasting message from %s: %s\n", sender, message);

  for (int i = 0; i < client_count; i++)
  {
    if (clients[i].authenticated)
    {
      send(clients[i].socket, buffer, strlen(buffer), 0);
    }
  }
}

void private_message(const char *sender, const char *target, const char *message)
{
  char buffer[BUFFER_SIZE];
  snprintf(buffer, BUFFER_SIZE, "[PM] %s: %s", sender, message);

  printf("Sending private message from %s to %s: %s\n", sender, target, message);

  for (int i = 0; i < client_count; i++)
  {
    if (clients[i].authenticated && strcmp(clients[i].username, target) == 0)
    {
      send(clients[i].socket, buffer, strlen(buffer), 0);
      return;
    }
  }
  printf("User %s not found for private message\n", target);
}

int main()
{
  int server_fd;
  struct sockaddr_in address;
  fd_set read_fds;
  int max_sd;

  // Create socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  bind(server_fd, (struct sockaddr *)&address, sizeof(address));
  listen(server_fd, 3);

  printf("Chat server started on port %d\n", PORT);

  while (1)
  {
    FD_ZERO(&read_fds);
    FD_SET(server_fd, &read_fds);
    max_sd = server_fd;

    for (int i = 0; i < client_count; i++)
    {
      int sd = clients[i].socket;
      FD_SET(sd, &read_fds);
      max_sd = (sd > max_sd) ? sd : max_sd;
    }

    select(max_sd + 1, &read_fds, NULL, NULL, NULL);

    if (FD_ISSET(server_fd, &read_fds))
    {
      int new_socket = accept(server_fd, NULL, NULL);
      clients[client_count].socket = new_socket;
      clients[client_count].authenticated = 0;
      client_count++;
      printf("New client connected, socket fd: %d\n", new_socket);
    }

    for (int i = 0; i < client_count; i++)
    {
      int sd = clients[i].socket;

      if (FD_ISSET(sd, &read_fds))
      {
        Message msg;
        int valread = read(sd, &msg, sizeof(Message));

        if (valread == 0)
        {
          printf("Client disconnected, socket fd: %d\n", sd);
          close(sd);
          // Remove client
          for (int j = i; j < client_count - 1; j++)
          {
            clients[j] = clients[j + 1];
          }
          client_count--;
          continue;
        }

        switch (msg.type)
        {
        case MSG_REGISTER:
        {
          User new_user;
          strcpy(new_user.username, msg.username);
          strcpy(new_user.password, msg.password);
          save_user(&new_user);
          send(sd, "Registration successful", 22, 0);
          printf("User %s registered\n", msg.username);
          break;
        }
        case MSG_LOGIN:
        {
          if (check_user(msg.username, msg.password))
          {
            strcpy(clients[i].username, msg.username);
            clients[i].authenticated = 1;
            send(sd, "Login successful", 16, 0);
            printf("User %s logged in\n", msg.username);
          }
          else
          {
            send(sd, "Login failed", 11, 0);
            printf("Login failed for user %s\n", msg.username);
          }
          break;
        }
        case MSG_BROADCAST:
          if (clients[i].authenticated)
          {
            broadcast_message(clients[i].username, msg.message);
          }
          break;
        case MSG_PRIVATE:
          if (clients[i].authenticated)
          {
            private_message(clients[i].username, msg.target, msg.message);
          }
          break;
        }
      }
    }
  }

  return 0;
}