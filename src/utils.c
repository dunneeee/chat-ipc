#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chat.h"

void register_user(const char *username, const char *password)
{
  FILE *file = fopen("users.txt", "a");
  if (file == NULL)
  {
    perror("Failed to open file");
    exit(EXIT_FAILURE);
  }
  fprintf(file, "%s %s\n", username, password);
  fclose(file);
}

int login_user(const char *username, const char *password)
{
  // Make a copy of password so we can modify it
  char password_copy[USERNAME_SIZE];
  strncpy(password_copy, password, USERNAME_SIZE - 1);
  password_copy[USERNAME_SIZE - 1] = '\0';

  // Remove trailing newline from password if present
  char *newline = strchr(password_copy, '\n');
  if (newline)
    *newline = '\0';

  printf("Login attempt for user '%s' with password '%s'\n", username, password_copy);

  FILE *file = fopen("users.txt", "r");
  if (file == NULL)
  {
    file = fopen("users.txt", "a+");
    if (file == NULL)
    {
      perror("Failed to create file");
      exit(EXIT_FAILURE);
    }
    fclose(file);
    file = fopen("users.txt", "r");
  }

  char file_username[USERNAME_SIZE] = {0};
  char file_password[USERNAME_SIZE] = {0};
  int found = 0;

  while (fscanf(file, "%s %s", file_username, file_password) == 2)
  {
    printf("Comparing:\n");
    printf("File : username='%s'(%zu) password='%s'(%zu)\n",
           file_username, strlen(file_username),
           file_password, strlen(file_password));
    printf("Input: username='%s'(%zu) password='%s'(%zu)\n",
           username, strlen(username),
           password_copy, strlen(password_copy));

    if (strcmp(username, file_username) == 0 &&
        strcmp(password_copy, file_password) == 0)
    {
      found = 1;
      break;
    }
  }

  fclose(file);
  return found;
}
