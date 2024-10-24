#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define SOCKET_PATH "/tmp/chat_socket"
#define BUFFER_SIZE 256

void get_current_time(char *buffer, size_t buffer_size)
{
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", timeinfo);
}

void *receive_messages(void *args)
{
    int socket_fd = *((int *)((void **)args)[0]);
    FILE *output_file = *((FILE **)((void **)args)[1]);
    char buffer[BUFFER_SIZE];
    int bytes_received;
    char time_str[64];

    while ((bytes_received = recv(socket_fd, buffer, BUFFER_SIZE, 0)) > 0)
    {
        buffer[bytes_received] = '\0';

        get_current_time(time_str, sizeof(time_str));
        fprintf(output_file, "[%s]: %s\n", time_str, buffer);
        fflush(output_file);

        printf("Message from server: %s\n", buffer);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    int client_fd;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];
    pthread_t receive_thread;
    FILE *input_file;
    FILE *output_file;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *input_filename = argv[1];
    input_file = fopen(input_filename, "r");
    if (!input_file)
    {
        perror("Failed to open input file");
        exit(EXIT_FAILURE);
    }

    const char *output_filename = argv[2];
    output_file = fopen(output_filename, "a");
    if (!output_file)
    {
        perror("Failed to open output file");
        fclose(input_file);
        exit(EXIT_FAILURE);
    }

    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1)
    {
        perror("Socket failed");
        fclose(input_file);
        fclose(output_file);
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("Connect failed");
        close(client_fd);
        fclose(input_file);
        fclose(output_file);
        exit(EXIT_FAILURE);
    }

    void *args[] = {&client_fd, &output_file};
    pthread_create(&receive_thread, NULL, receive_messages, args);

    while (fgets(buffer, BUFFER_SIZE, input_file))
    {
        send(client_fd, buffer, strlen(buffer), 0);
    }

    fclose(input_file);

    printf("Finished sending input file. You can now type messages. Press Enter to send.\n");

    while (1)
    {
        if (fgets(buffer, BUFFER_SIZE, stdin) != NULL)
        {
            send(client_fd, buffer, strlen(buffer), 0);
        }
    }

    pthread_join(receive_thread, NULL);

    fclose(output_file);

    close(client_fd);
    return 0;
}
