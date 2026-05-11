#include "eth_receive.h"
#include "../sram.h"
#include "ethernet.h"
#include <string.h>
#include <stdio.h>
#include "../nvic.h"
#include "../../net/handle_packet.h"
#include "../../settings.h"

static volatile uint32_t *ETH_DMACRXRLR = (uint32_t *)(ETHERNET_BASE + 0x1130);
static volatile uint32_t *ETH_DMACRXDLAR = (uint32_t *)(ETHERNET_BASE + 0x111C);
static volatile uint32_t *ETH_DMACRXDTPR = (uint32_t *)(ETHERNET_BASE + 0x1128);
static volatile uint32_t *ETH_DMACRXCR   = (uint32_t *)(ETHERNET_BASE + 0x1108);
static volatile uint32_t *ETH_DMACSR    = (uint32_t *)(ETHERNET_BASE + 0x1160);

#define ETH_RX_QUEUE_ADDR ((uint32_t)eth_rx_queue_addr())
#define ETH_RX_DATA_START (eth_rx_data_start())

static int FIRST_DMA_DESCRIPTOR_INDEX = 0;

struct RxDescriptor {
    uint32_t buf_1_addr;
    uint32_t reserved;
    uint32_t buf_2_addr;
    uint32_t flags;
};

void init_eth_rx() {
    struct RxDescriptor *rx_queue = (struct RxDescriptor *)(uintptr_t)ETH_RX_QUEUE_ADDR;
    for (int i = 0; i < NUM_DESCRIPTORS; i++) {
        rx_queue[i].buf_1_addr = ETH_RX_DATA_START + i * 1024; //each buffer is 1024B
        rx_queue[i].buf_2_addr = 0;
        rx_queue[i].flags = 1 << 31 | 1 << 30 | 1 << 24; //set ownership to DMA, IOC, set buf addr 1 valid
    }

    set_register(ETH_DMACRXRLR, 0b111111111, NUM_DESCRIPTORS - 1);
    *ETH_DMACRXDLAR = (uintptr_t)ETH_RX_QUEUE_ADDR;
    *ETH_DMACRXDTPR = (uintptr_t)ETH_RX_QUEUE_ADDR + (NUM_DESCRIPTORS - 1) * 16;

    set_register(ETH_DMACRXCR, 0x3F7FFF, 
        32 << 16  | //RXPBL
        1024 << 1 | //RBSZ
        0           //SR
    );

    //write addr and tail
    printf("RX queue head pointer: 0x%08X\n", *ETH_DMACRXDLAR);
    printf("RX queue tail pointer: 0x%08X\n", *ETH_DMACRXDTPR);
}

void set_rx_state(uint8_t enabled) {
    if (enabled) {
        *ETH_DMACRXCR |= 0x1; //set SR bit
    } else {
        *ETH_DMACRXCR &= ~0x1; //clear SR bit
    }
    printf("RX state set to %d, should be set to %d\n", *ETH_DMACRXCR & 0x1, enabled);
}

uint8_t get_rx_state() {
    printf("RX queue tail pointer: 0x%08X\n", *ETH_DMACRXDTPR);
    return (*ETH_DMACRXCR & 0x1) != 0;
}

void reset_descriptor(uint32_t index) {
    struct RxDescriptor *rx_queue = (struct RxDescriptor *)(uintptr_t)ETH_RX_QUEUE_ADDR;
    rx_queue[index].buf_1_addr = ETH_RX_DATA_START + index * 1024; //reset buffer address
    rx_queue[index].buf_2_addr = 0;
    rx_queue[index].flags = 1 << 31 | 1 << 30 | 1 << 24; //set ownership to DMA, IOC, set buf addr 1 valid

    __asm volatile ("dsb"); //drain store buffer

    uint32_t next_tail = ETH_RX_QUEUE_ADDR + index * 16;
    *ETH_DMACRXDTPR = next_tail;
}

int is_for_me(uint8_t *packet, uint32_t length) {
    if (length < 14) {
        return 0; //not even a valid Ethernet frame
    }

    uint8_t *dst_mac = packet;
    uint8_t same_mac = memcmp(dst_mac, get_mac_address(), 6) == 0;
    uint8_t broadcast = memcmp(dst_mac, (uint8_t[])BROADCAST_MAC, 6) == 0;

    if (!same_mac && !broadcast) {
        return 0;
    }

    if (length < 14 + 20) {
        return 0; //not even a valid IPv4 packet
    }
    uint8_t *eth_type = packet + 12;
    uint8_t is_ipv4 = eth_type[0] == 0x08 && eth_type[1] == 0x00;
    uint8_t is_arp = eth_type[0] == 0x08 && eth_type[1] == 0x06;
    if (!is_ipv4 && !is_arp) {
        return 0; //not IPv4 or arp
    }

    if (is_arp) {
        return 1;
    }

    uint8_t *ip_header = packet + 14;
    uint8_t *ip_addr = ip_header + 16;
    uint8_t same_ip = memcmp(ip_addr, get_ip_address(), 4) == 0;
    uint8_t broadcast_ip = is_broadcast(ip_addr, (uint8_t[])SUBNET_MASK);
    uint8_t is_for_me_result = same_ip || broadcast_ip;

    if (!is_for_me_result) {
        return 0;
    }

    uint8_t is_udp = ip_header[9] == 17;

    if (!is_udp) {
        return 0; //only accept UDP packets
    }

    //compare port
    uint8_t *udp_header = ip_header + 20;
    uint16_t dst_port = (udp_header[2] << 8) | udp_header[3];
    if (dst_port != UDP_PORT) {
        return 0; //not for our port
    }

    return 1;
}

//returns data
uint32_t receive_packet_data(uint8_t *buffer) {
    uint32_t next_tail_index = FIRST_DMA_DESCRIPTOR_INDEX;

    struct RxDescriptor *rx_queue = (struct RxDescriptor *)(uintptr_t)ETH_RX_QUEUE_ADDR;
    struct RxDescriptor *desc = &rx_queue[next_tail_index];
    if (desc->flags & (1 << 31)) {
        //DMA still owns this descriptor, no packet received
        return -1;
    }

    FIRST_DMA_DESCRIPTOR_INDEX = (FIRST_DMA_DESCRIPTOR_INDEX + 1) % NUM_DESCRIPTORS;

    if (desc->flags & (1 << 30)) {
        reset_descriptor(next_tail_index);
        return 0; 
    }

    uint8_t first_desc = (desc->flags & (1 << 29)) != 0;
    uint8_t last_desc = (desc->flags & (1 << 28)) != 0;
    if (!first_desc || !last_desc) {
        //skip multi descriptor packets
        reset_descriptor(next_tail_index);
        return 0;
    }

    uint32_t buf_addr = ETH_RX_DATA_START + next_tail_index * 1024;
    uint32_t packet_length = desc->flags & 0x3FFF;

    int is_for_me_result = is_for_me((uint8_t *)(uintptr_t)buf_addr, packet_length);
    if (!is_for_me_result) {
        reset_descriptor(next_tail_index);
        return 0;
    }

    memmove(buffer, (void *)(intptr_t)buf_addr, packet_length);

    reset_descriptor(next_tail_index);
    return packet_length;
}

void ETH_IRQHandler() {
    uint32_t ints_saved = irq_save();
    uint32_t dma_csr = *ETH_DMACSR;
    *ETH_DMACSR = dma_csr; //clear interrupts

    uint8_t buffer[1024];
    while (1) {
        uint32_t len = receive_packet_data(buffer);
        if (len == (uint32_t)-1) {
            break; //no more packets
        }
        if (len == 0) {
            continue; //packet not for us or multi-descriptor packet, ignore
        }
        handle_packet(buffer, len);
    }

    irq_restore(ints_saved);
}
