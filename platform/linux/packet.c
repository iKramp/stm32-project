#include "../../include/packet.h"
#include "../../core/net/handle_packet.h"
#include "packet.h"
//include all linux/libc networking headers
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

static int stream_fd = -1;

void set_stream_fd(int fd) {
    stream_fd = fd;
}

uint8_t send_udp(uint8_t *data, uint16_t length, uint8_t *dest_ip, uint16_t dest_port) {
    if (stream_fd < 0) {
        return 0; // Not initialized
    }

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(dest_port);
    memcpy(&dest_addr.sin_addr.s_addr, dest_ip, 4);

    ssize_t sent_bytes = sendto(stream_fd, data, length, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    return sent_bytes == length ? 1 : 0;
}

void process_incoming_packets() {
    if (stream_fd < 0) {
        return; // Not initialized
    }

    uint8_t buffer[1024];
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);

    //receive and process in loop, if no more data, return
    while (1) {
        ssize_t recv_len = recvfrom(stream_fd, buffer, sizeof(buffer), MSG_DONTWAIT, (struct sockaddr *)&src_addr, &addr_len);
        if (recv_len < 0) {
            break; // No more data
        }

        uint8_t *src_ip = (uint8_t *)&src_addr.sin_addr.s_addr;
        uint16_t src_port = ntohs(src_addr.sin_port);

        handle_udp_packet(buffer, recv_len, src_ip, src_port);
    }
}
