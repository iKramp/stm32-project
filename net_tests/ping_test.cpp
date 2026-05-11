// client.c
// gcc -o client client.c
//
// Usage:
//   ./client SERVER_IP SERVER_PORT output.bin
//
// Example:
//   ./client 192.168.1.100 12345 dump.bin

#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "message.h"

#define MAGIC 0xDEADBEEF
#define BUFFER_SIZE 4096

static const char *message_type_name(uint32_t type) {
    switch (type) {
        case PING: return "PING";
        case PONG: return "PONG";
        case REQUEST_DATA: return "REQUEST_DATA";
        case REQUEST_REGION: return "REQUEST_REGION";
        case DATA_RESPONSE: return "DATA_RESPONSE";
        case REGION_RESPONSE: return "REGION_RESPONSE";
        case DATA_FINISHED: return "DATA_FINISHED";
        case HELLO: return "HELLO";
        default: return "UNKNOWN";
    }
}

static void dump_bytes(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);

        if ((i + 1) % 16 == 0)
            printf("\n");
    }

    if (len % 16 != 0)
        printf("\n");
}

static int send_message(
    int sock,
    struct sockaddr_in *addr,
    struct Message *msg,
    size_t size
) {
    ssize_t sent = sendto(
        sock,
        msg,
        size,
        0,
        (struct sockaddr *)addr,
        sizeof(*addr)
    );

    if (sent < 0) {
        perror("sendto");
        return -1;
    }

    return 0;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr,
                "Usage: %s SERVER_IP SERVER_PORT output.bin\n",
                argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    const char *output_path = argv[3];

    FILE *out = fopen(output_path, "wb");

    if (!out) {
        perror("fopen");
        return 1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        perror("socket");
        fclose(out);
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &addr.sin_addr) != 1) {
        fprintf(stderr, "Invalid server IP\n");
        fclose(out);
        close(sock);
        return 1;
    }

    // ------------------------------------------------------------
    // Send HELLO
    // ------------------------------------------------------------

    struct Message hello_msg;
    memset(&hello_msg, 0, sizeof(hello_msg));

    hello_msg.magic = MAGIC;
    hello_msg.type = HELLO;

    printf("[*] Sending HELLO\n");

    if (send_message(
            sock,
            &addr,
            &hello_msg,
            sizeof(uint32_t) * 2) < 0) {
        fclose(out);
        close(sock);
        return 1;
    }

    // ------------------------------------------------------------
    // Send REQUEST_DATA
    // ------------------------------------------------------------

    struct Message request_msg;
    memset(&request_msg, 0, sizeof(request_msg));

    request_msg.magic = MAGIC;
    request_msg.type = REQUEST_DATA;

    printf("[*] Sending REQUEST_DATA\n");

    if (send_message(
            sock,
            &addr,
            &request_msg,
            sizeof(uint32_t) * 2) < 0) {
        fclose(out);
        close(sock);
        return 1;
    }

    // ------------------------------------------------------------
    // Receive loop
    // ------------------------------------------------------------

    uint8_t buffer[BUFFER_SIZE];

    while (1) {
        struct sockaddr_in from;
        socklen_t from_len = sizeof(from);

        ssize_t received = recvfrom(
            sock,
            buffer,
            sizeof(buffer),
            0,
            (struct sockaddr *)&from,
            &from_len
        );

        if (received < 0) {
            perror("recvfrom");
            break;
        }

        printf("\n[*] Received %zd bytes\n", received);

        if (received < (ssize_t)(sizeof(uint32_t) * 2)) {
            printf("[-] Packet too small\n");
            continue;
        }

        struct Message *msg = (struct Message *)buffer;

        if (msg->magic != MAGIC) {
            printf("[-] Invalid magic: 0x%08X\n", msg->magic);
            continue;
        }

        printf("[+] Message type: %s (%u)\n",
               message_type_name(msg->type),
               msg->type);

        switch (msg->type) {
            case DATA_RESPONSE: {
                struct DataResponseMessage *resp =
                    &msg->payload.data_response;

                printf("    offset       = %u\n", resp->offset);
                printf("    length       = %u\n", resp->length);
                printf("    total_length = %u\n", resp->total_length);

                if (resp->length > sizeof(resp->data)) {
                    printf("[-] Invalid data length\n");
                    break;
                }

                if (fseek(out, resp->offset, SEEK_SET) != 0) {
                    perror("fseek");
                    break;
                }

                size_t written = fwrite(
                    resp->data,
                    1,
                    resp->length,
                    out
                );

                fflush(out);

                if (written != resp->length) {
                    perror("fwrite");
                    break;
                }

                printf("[+] Wrote %u bytes to file\n",
                       resp->length);

                break;
            }

            case DATA_FINISHED: {
                struct DataFinishedMessage *resp =
                    &msg->payload.data_finished;

                printf("[+] DATA_FINISHED received\n");
                printf("    final chunk offset = %u\n",
                       resp->offset);
                printf("    final chunk length = %u\n",
                       resp->length);

                if (resp->length <= sizeof(resp->data)) {
                    if (fseek(out, resp->offset, SEEK_SET) == 0) {
                        fwrite(resp->data, 1, resp->length, out);
                        fflush(out);
                    }
                }

                printf("[+] Transfer complete\n");

                fclose(out);
                close(sock);

                return 0;
            }

            case REGION_RESPONSE: {
                struct RegionResponseMessage *resp =
                    &msg->payload.region_response;

                printf("    x      = %u\n", resp->x);
                printf("    y      = %u\n", resp->y);
                printf("    width  = %u\n", resp->width);
                printf("    height = %u\n", resp->height);

                break;
            }

            case PING: {
                printf("[*] Received PING, replying with PONG\n");

                struct Message pong_msg;
                memset(&pong_msg, 0, sizeof(pong_msg));

                pong_msg.magic = MAGIC;
                pong_msg.type = PONG;

                if (send_message(
                        sock,
                        &from,
                        &pong_msg,
                        sizeof(uint32_t) * 2) < 0) {
                    printf("[-] Failed to send PONG\n");
                }

                break;
            }
            case PONG:
            case HELLO:
            case REQUEST_DATA:
            case REQUEST_REGION:
            default:
                printf("[*] Raw packet dump:\n");
                dump_bytes(buffer, received);
                break;
        }
    }

    fclose(out);
    close(sock);

    return 0;
}
