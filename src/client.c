#include "chat.h"
#include <time.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8888

int sock_fd;
char current_user[MAX_USERNAME] = "";

void send_message(Message *msg)
{
  send(sock_fd, msg, sizeof(Message), 0);
}

void handle_command(char *cmd)
{
  Message msg;
  char *token = strtok(cmd, " ");

  if (strcmp(token, "/register") == 0)
  {
    msg.type = MSG_REGISTER;
    token = strtok(NULL, " ");
    if (!token)
    {
      printf("Usage: /register username password\n");
      return;
    }
    strcpy(msg.username, token);
    token = strtok(NULL, " ");
    if (!token)
    {
      printf("Usage: /register username password\n");
      return;
    }
    strcpy(msg.password, token);
    send_message(&msg);
  }
  else if (strcmp(token, "/login") == 0)
  {
    msg.type = MSG_LOGIN;
    token = strtok(NULL, " ");
    if (!token)
    {
      printf("Usage: /login username password\n");
      return;
    }
    strcpy(msg.username, token);
    token = strtok(NULL, " ");
    if (!token)
    {
      printf("Usage: /login username password\n");
      return;
    }
    strcpy(msg.password, token);
    strcpy(current_user, msg.username);
    send_message(&msg);
  }
  else if (strcmp(token, "/all") == 0)
  {
    if (strlen(current_user) == 0)
    {
      printf("Please login first\n");
      return;
    }
    msg.type = MSG_BROADCAST;
    token = strtok(NULL, "");
    if (!token)
    {
      printf("Usage: /all message\n");
      return;
    }
    strcpy(msg.message, token);
    send_message(&msg);
  }
  else if (strcmp(token, "/input") == 0)
  {
    if (strlen(current_user) == 0)
    {
      printf("Please login first\n");
      return;
    }

    token = strtok(NULL, " ");
    if (!token)
    {
      printf("Usage: /input <all|username> <filepath>\n");
      return;
    }

    char *filepath = strtok(NULL, " ");
    if (!filepath)
    {
      printf("Usage: /input <all|username> <filepath>\n");
      return;
    }

    FILE *fp = fopen(filepath, "r");
    if (!fp)
    {
      printf("Error: Cannot open file %s\n", filepath);
      return;
    }

    char content[BUFFER_SIZE] = {0};
    size_t bytes_read = fread(content, 1, sizeof(content) - 1, fp);
    fclose(fp);

    if (bytes_read == 0)
    {
      printf("Error: File is empty or could not be read\n");
      return;
    }

    Message msg;
    if (strcmp(token, "all") == 0)
    {
      msg.type = MSG_BROADCAST;
      strcpy(msg.message, content);
    }
    else
    {
      msg.type = MSG_PRIVATE;
      strcpy(msg.target, token);
      strcpy(msg.message, content);
    }

    send_message(&msg);
  }
  else if (token[0] == '/')
  {
    if (strlen(current_user) == 0)
    {
      printf("Please login first\n");
      return;
    }
    msg.type = MSG_PRIVATE;
    strcpy(msg.target, token + 1);
    token = strtok(NULL, "");
    if (!token)
    {
      printf("Usage: /username message\n");
      return;
    }
    strcpy(msg.message, token);
    send_message(&msg);
  }
}

int main()
{
  struct sockaddr_in serv_addr;
  char buffer[BUFFER_SIZE];
  fd_set read_fds;

  sock_fd = socket(AF_INET, SOCK_STREAM, 0);

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

  connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

  printf("Connected to chat server\n");
  printf("Commands:\n");
  printf("/register username password\n");
  printf("/login username password\n");
  printf("/all message\n");
  printf("/username message\n");

  while (1)
  {
    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(sock_fd, &read_fds);

    select(sock_fd + 1, &read_fds, NULL, NULL, NULL);

    if (FD_ISSET(STDIN_FILENO, &read_fds))
    {
      fgets(buffer, BUFFER_SIZE, stdin);
      buffer[strcspn(buffer, "\n")] = 0;
      handle_command(buffer);
    }

    if (FD_ISSET(sock_fd, &read_fds))
    {
      int valread = read(sock_fd, buffer, BUFFER_SIZE);
      if (valread == 0)
      {
        printf("Server disconnected\n");
        return 0;
      }

      time_t now = time(NULL);
      struct tm *t = localtime(&now);

      char timestamp[32];
      strftime(timestamp, sizeof(timestamp), "[%H:%M:%S - %d/%m/%Y]", t);

      buffer[valread] = 0;
      printf("%s %s\n", timestamp, buffer);
    }
  }

  return 0;
}