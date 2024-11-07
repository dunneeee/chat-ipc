#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// #include <fcntl.h>
#include <errno.h>
#include <sys/select.h>

#define SOCKET_PATH "/tmp/chat_socket"
#define BUFFER_SIZE 256
#define MAX_CLIENTS 10

int clients[MAX_CLIENTS] = {0};

void broadcast_message(int sender, const char *message)
{
    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "Client %d: %s", sender, message);

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        int fd = clients[i];
        if (fd > 0 && fd != sender)
        {
            send(fd, buffer, strlen(buffer), 0);
        }
    }
}

int main()
{
    int server_fd, client_fd, max_fd, activity;
    struct sockaddr_un addr;
    fd_set read_fds;
    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) == -1)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Chat server is running on %s\n", SOCKET_PATH);

    while (1)
    {

        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        max_fd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int fd = clients[i];
            if (fd > 0)
            {
                FD_SET(fd, &read_fds);
            }
            if (fd > max_fd)
            {
                max_fd = fd;
            }
        }

        activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
        {
            perror("Select error");
        }

        if (FD_ISSET(server_fd, &read_fds))
        {
            if ((client_fd = accept(server_fd, NULL, NULL)) < 0)
            {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i] == 0)
                {
                    clients[i] = client_fd;
                    printf("Client connected, fd: %d\n", client_fd);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int fd = clients[i];
            if (FD_ISSET(fd, &read_fds))
            {
                int bytes_read = read(fd, buffer, BUFFER_SIZE);
                if (bytes_read == 0)
                {

                    close(fd);
                    clients[i] = 0;
                    printf("Client disconnected, fd: %d\n", fd);
                    snprintf(buffer, BUFFER_SIZE, "Client %d disconnected", fd);
                    broadcast_message(fd, buffer);
                }
                else
                {

                    buffer[bytes_read] = '\0';
                    printf("Message from client %d: %s\n", fd, buffer);
                    broadcast_message(fd, buffer);
                }
            }
        }
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
