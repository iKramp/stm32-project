#include "packet_handler.hpp"

//C compat wrapper
#include <stdio.h>
#include <string.h>
extern "C" {
    #include "handle_packet.h"
    #include "../peripherals/eth/ethernet.h"
    #include "../peripherals/eth/eth_transmit.h"
}

//arr of void functions for handling different message types
MessageHandler message_handlers[NUM_MESSAGE_TYPES] = {0};

void register_message_handler(enum MessageType type, MessageHandler handler) {
    message_handlers[type] = handler;
}

MessageHandler get_message_handler(uint32_t type) {
    return message_handlers[type];
}

void ping_handler(struct Message *msg, uint8_t *src_ip, uint8_t *src_mac, uint16_t src_port) {
    struct Message reply = {0};
    reply.magic = 0xDEADBEEF;
    reply.type = PONG;

    uint32_t msg_size = get_message_size(&reply);

    transmit_data_udp((uint8_t *)&reply, msg_size, src_ip, src_mac, src_port);
}

void init_packet_handlers(uint8_t server) {
    register_message_handler(PING, ping_handler);
}

void reply_arp(uint8_t *packet) {
    uint8_t *ether_hdr = packet;
    uint8_t *arp_hdr = packet + 14;

    uint8_t is_arp_request = arp_hdr[6] == 0x00 && arp_hdr[7] == 0x01;
    uint8_t hw_is_ether = arp_hdr[0] == 0x00 && arp_hdr[1] == 0x01;
    uint8_t proto_is_ipv4 = arp_hdr[2] == 0x08 && arp_hdr[3] == 0x00;
    if (!is_arp_request || !hw_is_ether || !proto_is_ipv4) {
        return;
    }

    uint8_t *sender_mac = arp_hdr + 8;
    uint8_t *sender_ip = arp_hdr + 14;
    uint8_t *target_ip = arp_hdr + 24;

    uint8_t ip_match = memcmp(target_ip, get_ip_address(), 4) == 0;
    if (!ip_match) {
        return;
    }

    printf("got arp, replying\n");

    // Build ARP reply
    uint8_t reply[42];
    memcpy(reply, sender_mac, 6); // Destination MAC = sender MAC
    memcpy(reply + 6, get_mac_address(), 6); // Source MAC = our MAC
    reply[12] = 0x08; // EtherType = ARP
    reply[13] = 0x06;
    reply[14] = 0x00; // Hardware type = Ethernet
    reply[15] = 0x01;
    reply[16] = 0x08; // Protocol type = IPv4
    reply[17] = 0x00;
    reply[18] = 0x06; // Hardware size = 6
    reply[19] = 0x04; // Protocol size = 4
    reply[20] = 0x00; // Opcode = reply
    reply[21] = 0x02;
    memcpy(reply + 22, get_mac_address(), 6); // Sender MAC = our MAC
    memcpy(reply + 28, target_ip, 4); // Sender IP = target IP
    memcpy(reply + 32, sender_mac, 6); // Target MAC = sender MAC
    memcpy(reply + 38, sender_ip, 4); // Target IP = sender IP

    transmit_data(reply, 42);
}

//starts with ethernet header
void handle_packet(uint8_t *packet, uint32_t length) {
    uint8_t *ether_hdr = packet;
    uint8_t *ip_hdr = packet + 14;
    uint8_t *udp_hdr = ip_hdr + 20;
    uint8_t *payload = udp_hdr + 8;

    uint8_t is_arp = ether_hdr[12] == 0x08 && ether_hdr[13] == 0x06;
    if (is_arp) {
        reply_arp(packet);
        return;
    }

    uint8_t *src_ip = ip_hdr + 12;
    uint8_t *src_mac = ether_hdr + 6;
    uint16_t src_port = (udp_hdr[0] << 8) | udp_hdr[1];

    struct Message *msg = (struct Message *)payload;
    if (msg->magic != 0xDEADBEEF) {
        return; //not a valid message
    }

    MessageHandler handler = get_message_handler(msg->type);
    if (handler) {
        handler(msg, src_ip, src_mac, src_port);
    } else {
        printf("No handler for message type %d\n", msg->type);
    }
}
