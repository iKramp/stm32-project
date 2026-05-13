#include "../../include/platform_specific.h"
#include "../../include/settings.h"
#include "../../core/scene_data.h"
#include "packet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <sys/mman.h>

void platform_init(uint8_t server) {
    int socket_fd = -1;
    if (server) {
        struct sockaddr_in server_addr, client_addr;
        socklen_t client_len = sizeof(client_addr);

        socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_fd < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);

        if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("bind");
            close(socket_fd);
            exit(EXIT_FAILURE);
        }

        set_stream_fd(socket_fd);

        //memory map assets/buffers_dump.bin
        int fd = open("assets/buffers_dump.bin", O_RDONLY);
        if (fd < 0) {
            perror("open");
            close(socket_fd);
            exit(EXIT_FAILURE);
        }

        int file_size = lseek(fd, 0, SEEK_END);
        if (file_size < 0) {
            perror("lseek");
            close(fd);
            close(socket_fd);
            exit(EXIT_FAILURE);
        }

        uint8_t *mapped_data = (uint8_t *)mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (mapped_data == MAP_FAILED) {
            perror("mmap");
            close(fd);
            close(socket_fd);
            exit(EXIT_FAILURE);
        }

        set_scene_data(mapped_data, file_size);

    } else {
        struct sockaddr_in server_addr;

        socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_fd < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);

        if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
            perror("inet_pton");
            close(socket_fd);
            exit(EXIT_FAILURE);
        }

        printf("Client connecting to server %s:%d\n", SERVER_IP, PORT);

        set_stream_fd(socket_fd);
    }
}

uint8_t *get_data_buffer() {
    //allocate 128MB, same as 2 QSPI data banks in stm32h750b-dk
    uint8_t *buffer = (uint8_t *)malloc(128 * 1024 * 1024);
    if (!buffer) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    uint8_t *end_of_buffer = buffer + 128 * 1024 * 1024;
    printf("Data buffer allocated at address: %p, end of buffer: %p\n", buffer, end_of_buffer);
    return buffer;
}
