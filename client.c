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

struct thread_args
{
    int socket_fd;
    FILE *output_file;
};

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
    struct thread_args *thread_args = (struct thread_args *)args;
    int socket_fd = thread_args->socket_fd;
    FILE *output_file = thread_args->output_file;

    char buffer[BUFFER_SIZE];
    int bytes_received;
    char time_str[64];

    while ((bytes_received = recv(socket_fd, buffer, BUFFER_SIZE, 0)) > 0)
    {
        buffer[bytes_received] = '\0';

        get_current_time(time_str, sizeof(time_str));
        fprintf(output_file, "[%s]: %s\n", time_str, buffer);
        fflush(output_file);

        printf("%s\n", buffer);
    }

    return NULL;
}

void *listen_stdin(void *args)
{
    struct thread_args *thread_args = (struct thread_args *)args;
    int socket_fd = thread_args->socket_fd;
    char buffer[BUFFER_SIZE];

    while (1)
    {
        if (fgets(buffer, BUFFER_SIZE, stdin) != NULL)
        {
            send(socket_fd, buffer, strlen(buffer), 0);
        }
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    int client_fd;
    struct sockaddr_un addr;
    pthread_t receive_thread, stdin_thread;
    FILE *input_file;
    FILE *output_file;
    char buffer[BUFFER_SIZE];
    struct thread_args thread_args;

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

    thread_args.socket_fd = client_fd;
    thread_args.output_file = output_file;

    pthread_create(&receive_thread, NULL, receive_messages, &thread_args);
    pthread_create(&stdin_thread, NULL, listen_stdin, &thread_args);

    // Read lines from input file and send them to the server with a delay
    while (fgets(buffer, BUFFER_SIZE, input_file) != NULL)
    {
        send(client_fd, buffer, strlen(buffer), 0);
        sleep(1);
    }

    fclose(input_file);

    printf("Finished sending input file. You can now type messages. Press Enter to send.\n");

    pthread_join(receive_thread, NULL);
    pthread_join(stdin_thread, NULL);

    fclose(output_file);

    close(client_fd);
    return 0;
}
