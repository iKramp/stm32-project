#include "ltdc.h"

#define LTDC_BASE 0x50001000
#define WIDTH 480
#define HEIGHT 272

#define PIXEL_FORMAT_ARGB8888 0x0
#define PIXEL_FORMAT_RGB888   0x1
#define PIXEL_FORMAT_RGB565   0x2


#define H_SYNC 10
#define V_SYNC 10
#define H_BP 43
#define V_BP 12
#define H_ACTIVE WIDTH
#define V_ACTIVE HEIGHT
#define H_FP 8
#define V_FP 4


volatile uint32_t *LTDC_SSCR = (uint32_t *)(LTDC_BASE + 0x08);
volatile uint32_t *LTDC_BPCR = (uint32_t *)(LTDC_BASE + 0x0C);
volatile uint32_t *LTDC_AWCR = (uint32_t *)(LTDC_BASE + 0x10);
volatile uint32_t *LTDC_TWCR = (uint32_t *)(LTDC_BASE + 0x14);
volatile uint32_t *LTDC_GCR  = (uint32_t *)(LTDC_BASE + 0x18);
volatile uint32_t *LTDC_SRCR = (uint32_t *)(LTDC_BASE + 0x24);
volatile uint32_t *LTDC_BCCR = (uint32_t *)(LTDC_BASE + 0x2C);
volatile uint32_t *LTDC_IER  = (uint32_t *)(LTDC_BASE + 0x34);
volatile uint32_t *LTDC_ISR  = (uint32_t *)(LTDC_BASE + 0x38);
volatile uint32_t *LTDC_ICR  = (uint32_t *)(LTDC_BASE + 0x3C);
// ... Other LTDC registers as needed
volatile uint32_t *LTDC_L1CR = (uint32_t *)(LTDC_BASE + 0x84);
volatile uint32_t *LTDC_L1WHPCR = (uint32_t *)(LTDC_BASE + 0x88);
volatile uint32_t *LTDC_L1WVPCR = (uint32_t *)(LTDC_BASE + 0x8C);
volatile uint32_t *LTDC_L1CKCR  = (uint32_t *)(LTDC_BASE + 0x90);
volatile uint32_t *LTDC_L1PFCR  = (uint32_t *)(LTDC_BASE + 0x94);
volatile uint32_t *LTDC_L1CACR  = (uint32_t *)(LTDC_BASE + 0x98);
volatile uint32_t *LTDC_L1DCCR  = (uint32_t *)(LTDC_BASE + 0x9C);
volatile uint32_t *LTDC_L1BFCR  = (uint32_t *)(LTDC_BASE + 0xA0);
volatile uint32_t *LTDC_L1CFBAR = (uint32_t *)(LTDC_BASE + 0xAC);
volatile uint32_t *LTDC_L1CFBLR = (uint32_t *)(LTDC_BASE + 0xB0);
volatile uint32_t *LTDC_L1CFBLNR= (uint32_t *)(LTDC_BASE + 0xB4);

void set_ltdc_pin(uint8_t port, uint8_t pin_num) {
    set_speed(port, pin_num, 0b11);
    set_in_pull(port, pin_num, 0); //no pull
    set_alternate(port, pin_num, 14);
}

void dma2d_init() {
    volatile uint32_t *RCC_AHB3RSTR = (uint32_t *)(RCC_REG + 0x7C);
    volatile uint32_t *RCC_AHB3ENR = (uint32_t *)(RCC_REG + 0xD4);

    //enable dma2d clock
    set_register(RCC_AHB3ENR, 1 << 4, 1 << 4);

    //reset dma2d
    set_register(RCC_AHB3RSTR, 1 << 4, 1 << 4);
    set_register(RCC_AHB3RSTR, 1 << 4, 0 << 4);
}

void set_ltdc_pins() {
    volatile uint32_t *RCC_APB3ENR = (uint32_t *)(RCC_REG + 0xE4);
    volatile uint32_t *RCC_APB3RSTR = (uint32_t *)(RCC_REG + 0x8C);
    //enable ltdc clock
    set_register(RCC_APB3ENR, 1 << 3, 1 << 3);

    set_ltdc_pin('H', 9); // R3
    set_ltdc_pin('H', 1); //is in BSP

    set_ltdc_pin('I', 0); // G5
    set_ltdc_pin('I', 1); // G6
    set_ltdc_pin('I', 9); // VSYNC
    set_ltdc_pin('I', 12);// HSYNC
    set_ltdc_pin('I', 14);// CLK
    set_ltdc_pin('I', 15);// R0

    set_ltdc_pin('J', 0); // R1
    set_ltdc_pin('J', 1); // R2
    set_ltdc_pin('J', 3); // R4
    set_ltdc_pin('J', 4); // R5
    set_ltdc_pin('J', 5); // R6
    set_ltdc_pin('J', 6); // R7
    set_ltdc_pin('J', 7); // R8
    set_ltdc_pin('J', 8); // G1
    set_ltdc_pin('J', 9); // G2
    set_ltdc_pin('J', 10);// G3
    set_ltdc_pin('J', 11);// G4
    set_ltdc_pin('J', 12);// B0
    set_ltdc_pin('J', 13);// B1
    set_ltdc_pin('J', 14);// B2
    set_ltdc_pin('J', 15);// B3

    set_ltdc_pin('K', 0); //is in BSP
    set_ltdc_pin('K', 2); // G7
    set_ltdc_pin('K', 3); // B4
    set_ltdc_pin('K', 4); // B5
    set_ltdc_pin('K', 5); // B6
    set_ltdc_pin('K', 6); // B7
    set_ltdc_pin('K', 7); // DE

    set_moder('D', 7, MODER_OUTPUT);
    write_pin('D', 7, 1);

    set_moder('K', 0, MODER_OUTPUT); //BL
    write_pin('K', 0, 1); //turn on backlight

    set_register(RCC_APB3RSTR, 1 << 3, 1 << 3);
    set_register(RCC_APB3RSTR, 1 << 3, 0);
}

struct LayerConfig {
    uint32_t window_x0;
    uint32_t window_x1;
    uint32_t window_y0;
    uint32_t window_y1;
    uint32_t pixel_format;
    uint32_t bpp;
    uint32_t alpha;
    uint32_t framebuffer_address;
    uint32_t defualt_color;
};

void setup_layer(uint32_t layer, struct LayerConfig config) {
    volatile uint32_t *LxCR;
    volatile uint32_t *LxWHPCR;
    volatile uint32_t *LxWVPCR;
    volatile uint32_t *LxCKCR;
    volatile uint32_t *LxPFCR;
    volatile uint32_t *LxCACR;
    volatile uint32_t *LxDCCR;
    volatile uint32_t *LxBFCR;
    volatile uint32_t *LxCFBAR;
    volatile uint32_t *LxCFBLR;
    volatile uint32_t *LxCFBLNR;

    uint32_t width = config.window_x1 -  config.window_x0 + 1;
    uint32_t height = config.window_y1 -  config.window_y0 + 1;

    if (layer == 0) {
        LxCR = LTDC_L1CR;
        LxWHPCR = LTDC_L1WHPCR;
        LxWVPCR = LTDC_L1WVPCR;
        LxCKCR = LTDC_L1CKCR;
        LxPFCR = LTDC_L1PFCR;
        LxCACR = LTDC_L1CACR;
        LxDCCR = LTDC_L1DCCR;
        LxBFCR = LTDC_L1BFCR;
        LxCFBAR = LTDC_L1CFBAR;
        LxCFBLR = LTDC_L1CFBLR;
        LxCFBLNR = LTDC_L1CFBLNR;
    } else {
        LxCR = LTDC_L1CR + (0x80 / 4);
        LxWHPCR = LTDC_L1WHPCR + (0x80 / 4);
        LxWVPCR = LTDC_L1WVPCR + (0x80 / 4);
        LxCKCR = LTDC_L1CKCR + (0x80 / 4);
        LxPFCR = LTDC_L1PFCR + (0x80 / 4);
        LxCACR = LTDC_L1CACR + (0x80 / 4);
        LxDCCR = LTDC_L1DCCR + (0x80 / 4);
        LxBFCR = LTDC_L1BFCR + (0x80 / 4);
        LxCFBAR = LTDC_L1CFBAR + (0x80 / 4);
        LxCFBLR = LTDC_L1CFBLR + (0x80 / 4);
        LxCFBLNR = LTDC_L1CFBLNR + (0x80 / 4);
    }

    //position
    set_register(LxWHPCR, 0x0FFF0FFF, 
        (config.window_x1 + 1 + H_BP) << 16 |
        (config.window_x0 + 1 + H_BP)
    );

    set_register(LxWVPCR, 0x0FFF0FFF, 
        (config.window_y1 + 1 + V_BP) << 16 |
        (config.window_y0 + 1 + V_BP)
    );

    //pixel format
    set_register(LxPFCR, 0x00000007, config.pixel_format & 0x7);

    //default color
    set_register(LxDCCR, 0xFFFFFFFF, config.defualt_color);

    set_register(LxCACR, 0x000000FF, config.alpha & 0xFF);
    set_register(LxBFCR, 0x00000F0F, 0x607); //pixel alpha * constant alpha

    //framebuffer address
    set_register(LxCFBAR, 0xFFFFFFFF, config.framebuffer_address);

    //pitch and line length
    set_register(LxCFBLR, 0x1FFF1FFF, 
        ((width * config.bpp) & 0x1FFF) << 16 |
        (((width * config.bpp) + 0x7) & 0x1FFF)
    );

    //number of lines
    set_register(LxCFBLNR, 0x000007FF, height & 0x7FF);

    // enable
    set_register(LxCR, 1, 1);

    //reload
    set_register(LTDC_SRCR, 1, 1); // reload immediately
}

//Pixel clock @ 10MHz

void init_display() {

    set_ltdc_pins();
    dma2d_init();

    set_register(LTDC_GCR, 0xF0018881,
        0 << 31 | //HSPOL = active low
        0 << 30 | //VSPOL = active low
        0 << 29 | //DEPOL = not data enable is low
        0 << 28 | //PCPOL = input pixel clock
        0 << 16 | //DEN   = dither disable
        0 << 0   //LTDCEN
    );

    set_register(LTDC_SSCR, 0xFFF07FF, //sync
        ((H_SYNC - 1) & 0xFFF) << 16 | 
        ((V_SYNC - 1) & 0x7FF)
    );

    set_register(LTDC_BPCR, 0xFFF07FF, //back porch
        ((H_SYNC + H_BP - 1) & 0xFFF) << 16 | 
        ((V_SYNC + V_BP - 1) & 0x7FF)
    );

    set_register(LTDC_AWCR, 0xFFF07FF, //active width
        ((H_SYNC + H_BP + H_ACTIVE - 1) & 0xFFF) << 16 | 
        ((V_SYNC + V_BP + V_ACTIVE - 1) & 0x7FF)
    );

    set_register(LTDC_TWCR, 0xFFF07FF, //total width
        ((H_SYNC + H_BP + H_ACTIVE + H_FP - 1) & 0xFFF) << 16 | 
        ((V_SYNC + V_BP + V_ACTIVE + V_FP - 1) & 0x7FF)
    );

    
    set_register(LTDC_BCCR, 0xFFFFFFFF, 0x00FFFFFF); //solid color background

    //disabled for now
    struct LayerConfig layer0_config = {
        .window_x0 = 9,
        .window_x1 = 488,
        .window_y0 = 9,
        .window_y1 = 280,
        .pixel_format = PIXEL_FORMAT_ARGB8888,
        .bpp = 4, //match with pixel format
        .alpha = 255,
        .framebuffer_address = 0xD0000000, //sdram address
        .defualt_color = 0x00FFFFFF,
    };
    setup_layer(0, layer0_config);

    set_register(LTDC_IER, 3 << 1, 3 << 1);

    //enable ltdc
    set_register(LTDC_GCR, 1, 1);
}
