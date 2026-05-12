#include "eth_transmit.h"
#include "../sram.h"
#include "ethernet.h"
#include <string.h>
#include <stdio.h>
#include "../../../../include/packet.h"

volatile uint32_t *ETH_DMACTXRLR  = (uint32_t *)(ETHERNET_BASE + 0x112C);
volatile uint32_t *ETH_DMACTXDLAR = (uint32_t *)(ETHERNET_BASE + 0x1114);
volatile uint32_t *ETH_DMACTXDTPR = (uint32_t *)(ETHERNET_BASE + 0x1120);
volatile uint32_t *ETH_DMACTXCR   = (uint32_t *)(ETHERNET_BASE + 0x1104);

#define ETH_TX_QUEUE_ADDR ((uint32_t)eth_tx_queue_addr())
#define ETH_TX_DATA_START (eth_tx_data_start())

static int FIRST_SOFTWARE_DESCRIPTOR_INDEX = 0;

struct TxDescriptor {
    uint32_t buf_1_addr;
    uint32_t buf_2_addr;
    uint32_t tdes2;
    uint32_t flags;
};

void init_eth_tx() {
    struct TxDescriptor *tx_queue = (struct TxDescriptor *)(uintptr_t)ETH_TX_QUEUE_ADDR;
    for (int i = 0; i < NUM_DESCRIPTORS; i++) {
        tx_queue[i].buf_1_addr = 0;
        tx_queue[i].buf_2_addr = 0;
        tx_queue[i].tdes2 = 0;
        tx_queue[i].flags = 0;
    }

    set_register(ETH_DMACTXRLR, 0b111111111, NUM_DESCRIPTORS - 1);
    *ETH_DMACTXDLAR = ETH_TX_QUEUE_ADDR;
    *ETH_DMACTXDTPR = ETH_TX_QUEUE_ADDR;

    set_register(ETH_DMACTXCR, 0x3F1011, 
        32 << 16 | //TXPBL
        0 << 16  | //TSE
        0 << 4   | //OSF
        0          //ST
    );
}

void set_tx_state(uint8_t enabled) {
    if (enabled) {
        *ETH_DMACTXCR |= 0x1; //set ST bit
    } else {
        *ETH_DMACTXCR &= ~0x1; //clear ST bit
    }
}

uint8_t get_tx_state() {
    return (*ETH_DMACTXCR & 0x1) != 0;
}

void fill_default_tx_data(uint8_t *data, uint8_t *dest_mac, uint8_t *dest_ip) {
    uint8_t *start = data;

    // Destination MAC (broadcast)
    memcpy(data, dest_mac, 6);
    data += 6;

    // Source MAC (fake but valid)
    memcpy(data, get_mac_address(), 6);
    data += 6;

    // EtherType = IPv4
    *data++ = 0x08;
    *data++ = 0x00;

    *data++ = 0x45;       // Version (4) + IHL (5)
    *data++ = 0x00;       // DSCP/ECN
    *data++ = 0x00;       // Total length (placeholder)
    *data++ = 0x00;       // Total length (placeholder)
    *data++ = 0x00;       // Identification
    *data++ = 0x00;       // Identification
    *data++ = 0x40;       // Flags (don't fragment)
    *data++ = 0x00;       // Fragment offset
    *data++ = 0x40;       // TTL
    *data++ = 0x11;       // Protocol (UDP)
    *data++ = 0x00;       // Header checksum (placeholder)
    *data++ = 0x00;       // Header checksum (placeholder)

    memcpy(data, get_ip_address(), 4);
    data += 4;

    memcpy(data, dest_ip, 4);
    data += 4;
}

void fill_missing(uint8_t *data, uint32_t payload_length) {
    uint32_t length = 20 + 8 + payload_length; // IP header + UDP header + payload

    //fill in total length
    data[16] = length >> 8;
    data[17] = length & 0xff;

    //calculate IP header checksum
    uint16_t csum = ip_checksum(data + 14, 20);
    data[24] = csum & 0xff;
    data[25] = csum >> 8;
}

void add_udp_payload(uint8_t *buffer, uint8_t *payload, uint32_t payload_len, uint16_t dst_port) {
    uint8_t *p = buffer + 14 + 20; //start of UDP header

    uint16_t src_port = UDP_PORT;
    uint16_t udp_length = 8 + payload_len;

    *p++ = src_port >> 8;
    *p++ = src_port & 0xff;

    *p++ = dst_port >> 8;
    *p++ = dst_port & 0xff;

    *p++ = udp_length >> 8;
    *p++ = udp_length & 0xff;

    *p++ = 0x00; *p++ = 0x00; // UDP checksum = 0 (valid for IPv4)

    memcpy(p, payload, payload_len);
}

//compat function
uint8_t send_udp(uint8_t *data, uint16_t length, uint8_t *dest_ip, uint16_t dest_port) {
    return transmit_data_udp(data, length, dest_ip, (uint8_t[])BROADCAST_MAC, dest_port);
}

uint8_t transmit_data_udp(uint8_t *payload, uint32_t payload_len, uint8_t *dest_ip, uint8_t *dest_mac, uint16_t dest_port) {
    uint8_t tmp_buffer[1500];
    fill_default_tx_data(tmp_buffer, dest_mac, dest_ip);
    add_udp_payload(tmp_buffer, payload, payload_len, dest_port);
    fill_missing(tmp_buffer, payload_len);
    int full_length = payload_len + 14 + 20 + 8;

    return transmit_data(tmp_buffer, full_length);
}

uint8_t transmit_data(uint8_t *data, uint32_t length) {
    if (length > 1024) {
        panic("Packet too large to transmit");
    }
    uint32_t curr_tail_index = FIRST_SOFTWARE_DESCRIPTOR_INDEX;
    uint32_t next_tail_index = (curr_tail_index + 1) % NUM_DESCRIPTORS;
    uint32_t next_tail = ETH_TX_QUEUE_ADDR + next_tail_index * 16;

    struct TxDescriptor *tx_queue = (struct TxDescriptor *)(uintptr_t)ETH_TX_QUEUE_ADDR;
    struct TxDescriptor *next_desc = &tx_queue[next_tail_index];
    if (next_desc->flags & (1 << 31)) {
        //queue almost full, for simplicity require 1 free descriptor
        return 0;
    }

    FIRST_SOFTWARE_DESCRIPTOR_INDEX = next_tail_index;

    uint32_t buffer_addr = ETH_TX_DATA_START + curr_tail_index * 1024; //each buffer is 1024B
    memcpy((void *)(uintptr_t)buffer_addr, data, length);

    volatile struct TxDescriptor *curr_desc = &tx_queue[curr_tail_index];
    curr_desc->buf_1_addr = buffer_addr; //each buffer is 1024B
    curr_desc->buf_2_addr = 0;
    curr_desc->tdes2 = length | (1 << 31); //set length, interrupt on completion
    curr_desc->flags = 0 << 31 | //don't set ownership to DMA yet apparently
                       1 << 29 | //first descriptor
                       1 << 28 | //last descriptor
                       0 << 26 | //enable padding and crc
                       0 << 25 | //MAC 0
                       0b10 << 23 | //replace source MAC
                       length;
    curr_desc->flags |= 1 << 31; //set ownership to DMA

    __asm volatile ("dsb"); //drain store buffer before telling DMA about it
                                 
    *ETH_DMACTXDTPR = next_tail;

    return 1;
}
