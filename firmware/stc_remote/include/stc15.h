#ifndef PETCARGO_REMOTE_STC15_H
#define PETCARGO_REMOTE_STC15_H
__sfr __at(0x80) P0; __sfr __at(0x88) TCON; __sfr __at(0x89) TMOD;
__sfr __at(0x8A) TL0; __sfr __at(0x8B) TL1; __sfr __at(0x8C) TH0; __sfr __at(0x8D) TH1;
__sfr __at(0x8E) AUXR; __sfr __at(0x90) P1; __sfr __at(0x91) P1M1; __sfr __at(0x92) P1M0;
__sfr __at(0x93) P0M1; __sfr __at(0x94) P0M0; __sfr __at(0x95) P2M1; __sfr __at(0x96) P2M0;
__sfr __at(0xA0) P2; __sfr __at(0xA8) IE; __sfr __at(0xB8) IP; __sfr __at(0xB0) P3; __sfr __at(0xB1) P3M1;
__sfr __at(0xB2) P3M0; __sfr __at(0xBC) ADC_CONTR; __sfr __at(0xBD) ADC_RES;
__sfr __at(0x98) SCON; __sfr __at(0x99) SBUF; __sfr __at(0xA2) P_SW1;
__sfr __at(0xAF) IE2; __sfr __at(0xD6) T2H; __sfr __at(0xD7) T2L;
__sbit __at(0x8C) TR0; __sbit __at(0x8D) TF0; __sbit __at(0x8E) TR1; __sbit __at(0x8F) TF1;
__sbit __at(0xA9) ET0; __sbit __at(0xAF) EA; __sbit __at(0xA3) PIN_LED_SEL;
__sbit __at(0xB7) PIN_IR_TX;
__sbit __at(0x99) TI;
#endif
