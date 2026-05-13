#include "../../../include/packet.h"
#include <stdio.h>
#include <string.h>
#include "../peripherals/eth/ethernet.h"
#include "../peripherals/eth/eth_transmit.h"
#include "../../../core/net/handle_packet.h"


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

    handle_udp_packet(payload, length - 14 - 20 - 8, src_ip, src_port);
}

void process_incoming_packets() {
    //do nothing, is interrupt driven
}
