#ifndef PETCARGO_STC15_H
#define PETCARGO_STC15_H

/* IAP15F2K61S2 / STC15F2K60S2 register subset used by PetCargo. */
__sfr __at (0x80) P0;
__sfr __at (0x87) PCON;
__sfr __at (0x88) TCON;
__sfr __at (0x89) TMOD;
__sfr __at (0x8A) TL0;
__sfr __at (0x8B) TL1;
__sfr __at (0x8C) TH0;
__sfr __at (0x8D) TH1;
__sfr __at (0x8E) AUXR;
__sfr __at (0x8F) INT_CLKO;
__sfr __at (0x90) P1;
__sfr __at (0x91) P1M1;
__sfr __at (0x92) P1M0;
__sfr __at (0x93) P0M1;
__sfr __at (0x94) P0M0;
__sfr __at (0x95) P2M1;
__sfr __at (0x96) P2M0;
__sfr __at (0x98) SCON;
__sfr __at (0x99) SBUF;
__sfr __at (0x9A) S2CON;
__sfr __at (0x9B) S2BUF;
__sfr __at (0xA0) P2;
__sfr __at (0xA2) P_SW1;
__sfr __at (0xA8) IE;
__sfr __at (0xB8) IP;
__sfr __at (0xBA) P_SW2;
__sfr __at (0xAF) IE2;
__sfr __at (0xB0) P3;
__sfr __at (0xB1) P3M1;
__sfr __at (0xB2) P3M0;
__sfr __at (0xB3) P4M1;
__sfr __at (0xB4) P4M0;
__sfr __at (0xBC) ADC_CONTR;
__sfr __at (0xBD) ADC_RES;
__sfr __at (0xBE) ADC_RESL;
__sfr __at (0xC0) P4;
__sfr __at (0xC8) P5;
__sfr __at (0xC9) P5M1;
__sfr __at (0xCA) P5M0;
__sfr __at (0xD6) T2H;
__sfr __at (0xD7) T2L;
__sfr __at (0xD8) CCON;
__sfr __at (0xD9) CMOD;
__sfr __at (0xDB) CCAPM1;
__sfr __at (0xE9) CL;
__sfr __at (0xEB) CCAP1L;
__sfr __at (0xF9) CH;
__sfr __at (0xFB) CCAP1H;

__sbit __at (0x8C) TR0;
__sbit __at (0x8D) TF0;
__sbit __at (0x8E) TR1;
__sbit __at (0x8F) TF1;
__sbit __at (0x98) RI;
__sbit __at (0x99) TI;
__sbit __at (0xA9) ET0;
__sbit __at (0xAB) ET1;
__sbit __at (0xAC) ES;
__sbit __at (0xAF) EA;

/* Nets verified from the STC-B schematic. */
__sbit __at (0xA3) PIN_LED_SEL; /* P2.3: 0 seven-segment, 1 LEDs */
__sbit __at (0xA4) PIN_VIB;     /* P2.4, active low */
__sbit __at (0xA5) PIN_345_SCL; /* P2.5 */
__sbit __at (0xA6) PIN_345_SDA; /* P2.6 */
__sbit __at (0xB2) PIN_KEY1;    /* P3.2, physical K1 */
__sbit __at (0xB3) PIN_KEY2;    /* P3.3, physical K2 */
__sbit __at (0xB4) PIN_BEEP;    /* P3.4 / T1CLKO */
__sbit __at (0x92) PIN_HALL;    /* P1.2, A3144 active low */
__sbit __at (0x95) PIN_RTC_CLK; /* P1.5 */
__sbit __at (0xCC) PIN_RTC_IO;  /* P5.4, ISP must select GPIO, not RESET */
__sbit __at (0x96) PIN_RTC_RST; /* P1.6. P1.7 is exclusively navigation ADC. */
__sbit __at (0xC0) PIN_EE_SDA;  /* P4.0 */
__sbit __at (0xCD) PIN_EE_SCL;  /* P5.5 */
__sbit __at (0xC4) PIN_SM_S1;   /* P4.4 -> ULN2003 -> SM S1 */
__sbit __at (0xC3) PIN_SM_S2;   /* P4.3 -> ULN2003 -> SM S2 */
__sbit __at (0xB6) PIN_IR_RX;    /* P3.6 / CCP1_2, TSOP34840 */
__sbit __at (0xB7) PIN_IR_TX;    /* P3.7 / CCP2_2, ULN2003 IR LED */

#endif
