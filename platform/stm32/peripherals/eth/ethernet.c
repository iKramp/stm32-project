#include "ethernet.h"
#include "../clock.h"
#include "eth_receive.h"
#include "eth_transmit.h"
#include <stdio.h>
#include <string.h>
#include "../../hal/common.h"
#include "phy.h"
#include "../nvic.h"
#include "../../../../include/settings.h"

//operate in RMII mode because some pins are not available

static volatile uint32_t *ETH_DMAMR     = (uint32_t *)(ETHERNET_BASE + 0x1000);
static volatile uint32_t *ETH_DMASBMR   = (uint32_t *)(ETHERNET_BASE + 0x1004);
static volatile uint32_t *ETH_DMACCR    = (uint32_t *)(ETHERNET_BASE + 0x1100);
static volatile uint32_t *ETH_MTLTXQOMR = (uint32_t *)(ETHERNET_BASE + 0x0D00);
static volatile uint32_t *ETH_MTLRXQOMR = (uint32_t *)(ETHERNET_BASE + 0x0D30);
static volatile uint32_t *ETH_MACA0LR   = (uint32_t *)(ETHERNET_BASE + 0x0304);
static volatile uint32_t *ETH_MACA0HR   = (uint32_t *)(ETHERNET_BASE + 0x0300);
static volatile uint32_t *ETH_MACPFR    = (uint32_t *)(ETHERNET_BASE + 0x0008);
static volatile uint32_t *ETH_MACCR     = (uint32_t *)(ETHERNET_BASE + 0x0000);
static volatile uint32_t *ETH_DMADSR    = (uint32_t *)(ETHERNET_BASE + 0x100C);
static volatile uint32_t *ETH_MACMDIOAR = (uint32_t *)(ETHERNET_BASE + 0x0200);
static volatile uint32_t *ETH_MAC1USTCR = (uint32_t *)(ETHERNET_BASE + 0x00DC);

static volatile uint32_t *ETH_DMACSR    = (uint32_t *)(ETHERNET_BASE + 0x1160);
static volatile uint32_t *ETH_DMACIER   = (uint32_t *)(ETHERNET_BASE + 0x1134);
static volatile uint32_t *ETH_DMAISR    = (uint32_t *)(ETHERNET_BASE + 0x1008);
static volatile uint32_t *ETH_MACISR    = (uint32_t *)(ETHERNET_BASE + 0x00B0);
static volatile uint32_t *ETH_MACIER    = (uint32_t *)(ETHERNET_BASE + 0x00B4);
static volatile uint32_t *ETH_MACRXTXSR = (uint32_t *)(ETHERNET_BASE + 0x00B8);
static volatile uint32_t *ETH_MACDR     = (uint32_t *)(ETHERNET_BASE + 0x0114);
static volatile uint32_t *ETH_DMACMFCR  = (uint32_t *)(ETHERNET_BASE + 0x116C);

static volatile uint32_t *RCC_AHB1RSTR = (uint32_t *)(RCC_REG + 0x80);
static volatile uint32_t *RCC_AHB1ENR  = (uint32_t *)(RCC_REG + 0xD8);
static volatile uint32_t *RCC_AHB3ENR  = (uint32_t *)(RCC_REG + 0xD4);
static volatile uint32_t *RCC_AHB4ENR  = (uint32_t *)(RCC_REG + 0xE0);

//counters
static volatile uint32_t *ETH_RX_CRC_ERROR_PACKETS = (uint32_t *)(ETHERNET_BASE + 0x0794);
static volatile uint32_t *ETH_TX_PACKET_COUNT_GOOD = (uint32_t *)(ETHERNET_BASE + 0x0768);
static volatile uint32_t *ETH_RX_ALIGNMENT_ERROR_PACKETS = (uint32_t *)(ETHERNET_BASE + 0x0798);

static volatile uint32_t *SYSCFG_PMCR = (uint32_t *)(0x58000400 + 0x04);

uint8_t MAC_ADDRESS[6];
uint8_t IP_ADDRESS[4] = IP;

uint8_t *get_mac_address() {
    return MAC_ADDRESS;
}

uint8_t *get_ip_address() {
    return IP_ADDRESS;
}

void rcc_reset() {
    // set_register(SYSCFG_PMCR, 0b111 << 21, 0b100 << 21); //set ethernet rmii phy interface
    // int _ = *SYSCFG_PMCR; //read back to ensure write completes
    set_register(RCC_AHB1ENR, 0b111 << 15, 0b111 << 15); //ETH1RXEN, ETH1TXEN, ETH1MACEN

    set_register(RCC_AHB1RSTR, 1 << 15, 1 << 15);
    wait_ms(1);
    set_register(RCC_AHB1RSTR, 1 << 15, 0);
    wait_ms(1);

    //enable all DMA clocks, random bullshit go
    *RCC_AHB3ENR |= 0b1001; //enable DMA2D and MDMA
    *RCC_AHB1ENR |= 0b11;   //enable DMA1 and DMA2
    *RCC_AHB4ENR |= 0b1 << 21; //enable BDMA and DMAMUX
}

void set_ethernet_pin(uint8_t port, uint8_t pin_num) {
    set_speed(port, pin_num, 0b11);
    set_in_pull(port, pin_num, 0); //no pull
    set_alternate(port, pin_num, 11);
}

void set_ethernet_pins() {
    set_ethernet_pin('A', 1);  //ETH_RX_CLK
    set_ethernet_pin('C', 4);  //ETH_RDX[0]
    set_ethernet_pin('C', 5);  //ETH_RDX[1]
    set_ethernet_pin('A', 7);  //ETH_RX_DV
    set_ethernet_pin('I', 10); //ETH_RX_ER
    set_ethernet_pin('C', 3);  //ETH_TX_CLK //not needed in RMII mode
    set_ethernet_pin('G', 13); //ETH_TX[0]
    set_ethernet_pin('G', 12); //ETH_TX[1]
    set_ethernet_pin('G', 11); //ETH_TX_EN
    set_ethernet_pin('C', 1);  //ETH_MDC
    set_ethernet_pin('A', 2);  //ETH_MDIO

    //non conflicting MII
    set_ethernet_pin('C', 2); //ETH_TX[2]
    set_ethernet_pin('E', 2); //ETH_TX[3]
    set_ethernet_pin('B', 0); //ETH_RX[2]
    set_ethernet_pin('B', 1); //ETH_RX[3]
    set_ethernet_pin('A', 3); //ETH_COL
    set_ethernet_pin('A', 0); //ETH_CRS
    set_ethernet_pin('B', 2); //ER again
}

void mac_from_uid(uint8_t mac[6], uint32_t uid)
{
    mac[0] = 0x02; // locally administered unicast
    mac[1] = 0x00;
    mac[2] = (uid >> 24) & 0xFF;
    mac[3] = (uid >> 16) & 0xFF;
    mac[4] = (uid >> 8) & 0xFF;
    mac[5] = uid & 0xFF;
}

void init_dma() {
    //reset
    set_register(ETH_DMAMR, 0x1, 0x1);
    while (*ETH_DMAMR & 0x1) {};

    const int HCLK_FREQ_MHZ = 200;

    int divider;

    if (HCLK_FREQ_MHZ <= 35) {
        divider = 0b010;
    } else if (HCLK_FREQ_MHZ <= 60) {
        divider = 0b011;
    } else if (HCLK_FREQ_MHZ <= 100) {
        divider = 0b000;
    } else if (HCLK_FREQ_MHZ <= 150) {
        divider = 0b001;
    } else if (HCLK_FREQ_MHZ <= 250) {
        divider = 0b100;
    } else if (HCLK_FREQ_MHZ <= 300) {
        divider = 0b101;
    } else {
        panic("Unsupported HCLK frequency for Ethernet\n");
    }

    set_register(ETH_MACMDIOAR, 0b1111 << 8, divider << 8); //HCLK3 is 200MHz, set according to documentation page 2981
    set_register(ETH_MAC1USTCR, 0x7FF, HCLK_FREQ_MHZ - 1); //same as before, page 2973


    *ETH_DMAMR |= 1 << 16; //INTM
    set_register(ETH_DMAMR, 0b111 << 12, 0b0 << 12); //PR

    set_register(ETH_DMASBMR, 0xD001, 
        0x0 << 15 | //RB
        0x1 << 12 | //AAL
        0x1 //FB
    );

    set_register(ETH_DMACCR, 0x1D0000, 
        0 << 18 | //DSL
        0 << 16   //PBLX8
    );

    set_register(ETH_DMACIER, 0xFFC7, 0xFFC7);
}

void init_mtl() {
    set_register(ETH_MTLTXQOMR, 0x7F, 
        1 << 1 | //TSF
        0b10 << 2 //TQEN
    );

    set_register(ETH_MTLRXQOMR, 0x1C7FB, 
        1 << 5 | //RSF
        1 << 4 | //FEP
        1 << 3   //FUP
    );
}

void init_mac() {
    uint32_t uid = get_uid();
    uint8_t mac[6];
    mac_from_uid(mac, uid);
    *ETH_MACA0LR = (mac[3] << 24) | (mac[2] << 16) | (mac[1] << 8) | mac[0];
    *ETH_MACA0HR = (mac[5] << 8) | mac[4] | (1 << 31);
    MAC_ADDRESS[0] = mac[0];
    MAC_ADDRESS[1] = mac[1];
    MAC_ADDRESS[2] = mac[2];
    MAC_ADDRESS[3] = mac[3];
    MAC_ADDRESS[4] = mac[4];
    MAC_ADDRESS[5] = mac[5];


    set_register(ETH_MACPFR, 0x103107FF, 
        1 << 31 | //RA
        0 << 21 | //DNTU
        0 << 20 | //IPFE
        0 << 16 | //VTFE
        0 << 06 | //PCF
        0 << 05 | //DBF
        1 << 00   //PR - promiscuous mode
    );

    set_register(ETH_MACCR, 0xFFFB7F7F,
        0 << 31 | //ARPEN
        3 << 28 | //SARC - replace source addr
        1 << 27 | //IPC
        0 << 24 | //IPG 96 bit times
        0 << 23 | //GPSLCE
        0 << 22 | //S2KP
        0 << 21 | //CST
        1 << 20 | //ACS
        0 << 19 | //WD
        0 << 16 | //JE
        1 << 14 | //FES //100mbps
        1 << 13 | //DM  //FD
        0 << 12 | //LM
        0 << 11 | //ECRSFD
        0 << 05 | //BL
        1 << 04 | //DC
        0 << 02 | //PRELEN
        1 << 01 | //TE
        1 << 00   //RE
    );

    set_register(ETH_MACIER, 0x7038, 0x7038);
}

void debug_status() {
    uint32_t debug_status = *ETH_DMADSR;
    printf("DMA status: 0x%08X\n", debug_status);
    printf("Dma Tx state: ");
    switch (debug_status >> 12) {
        case 0b00000:
            printf("Stopped (Reset or Stop Transmit cmd issued)\n");
            break;
        case 0b00001:
            printf("Running (Fetching Tx Transfer Descriptor)\n");
            break;
        case 0b00010:
            printf("Running (Waiting for status)\n");
            break;
        case 0b00011:
            printf("Running (Reading Data from system memory buffer and queuing it to the Tx buffer (Tx FIFO))\n");
            break;
        case 0b00100:
            printf("Timestamp write state\n");
            break;
        case 0b00101:
            printf("DMA suspended with error\n");
            break;
        case 0b00110:
            printf("Suspended (Tx Descriptor Unavailable or Tx Buffer Underflow)\n");
            break;
        case 0b00111:
            printf("Running (Closing Tx Descriptor)\n");
            break;
        default:
            printf("DMA in unknown state\n");
    }

    printf("Dma Rx state: ");
    switch ((debug_status >> 8) & 0b111) {
        case 0b000:
            printf("Stopped (Reset or Stop Receive Command issued)\n");
            break;
        case 0b001:
            printf("Running (Fetching Rx Transfer Descriptor)\n");
            break;
        case 0b011:
            printf("Running (Waiting for packet)\n");
            break;
        case 0b100:
            printf("Suspended (Rx Descriptor Unavailable)\n");
            break;
        case 0b101:
            printf("Running (Closing the Rx Descriptor)\n");
            break;
        case 0b110:
            printf("Timestamp write state\n");
            break;
        case 0b111:
            printf("Running (Transferring the received packet data from the Rx buffer to the system memory)\n");
            break;
        default:
            printf("DMA in unknown state\n");
    }

    printf("AHB master write channel: %d\n", debug_status & 1);
}

// Helper: compute IPv4 header checksum
uint16_t ip_checksum(uint8_t *vdata, uint32_t length) {
    uint32_t sum = 0;
    uint16_t *data = (uint16_t *)vdata;

    for (; length > 1; length -= 2) {
        sum += *data++;
    }

    if (length == 1) {
        sum += *((uint8_t *)data);
    }

    // Fold 32-bit sum to 16 bits
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}

void init_ethernet() {
    print_cache_info();
    set_ethernet_pins();
    rcc_reset();
    init_dma();
    init_phy();
    init_eth_tx();
    init_eth_rx();
    init_mtl();
    init_mac();
    set_rx_state(1);
    set_tx_state(1);

    enable_irq(NVIC_ETH);
}
