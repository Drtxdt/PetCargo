#include "appinc.h"

#if CONFIG_GPIO_ENABLE
/***PACKAGE***|PAD|PIN|***************************************COREMUX******************************|***************AONMUX************|*ANAMUX**|BOND\
48 32 24 16 08| - | - | F0   F1   F2   F3   F4   F5  F6   F7    F8   F9 F10  F11 F12  F13  F14 F15 |  F0    F1    F2    F3   F4   F5 | F0   F1 | -  |
--------------|---|---|----------------------------------------------------------------------------|-------------- ------------------|---------|----|
 √  √ 23 15  8|A00|P00|GP00 JTCK TRX0 CTS1 CSEL SCL BCLK PAUSE PWM0 IRO DMC DCK DO00 DI00  ×    ×  |  ×     ×     ×     ×    ×    ×  | ×    ×  |    |
 √  √ 24 16  ×|A01|P01|GP01 JTMS RXD0 RTS1 MISO SDA LRCK T1CLK PWM1 IRI DMD DCK DO01 DI01  ×    ×  |  ×     ×     ×     ×    ×    ×  | ×    ×  |    |
 √  √  ×  ×  ×|A02|P02|GP02 JTCK TRX0 RXD1 MOSI SCL DATI TRIG0 PWM2 IRO DMC DCK DO02 DI02  ×    ×  |  ×     ×     ×     ×    ×    ×  | ×    ×  |    |
 √  √  ×  ×  ×|A03|P03|GP03 JTMS RXD0 TRX1 SCLK SDA DATO TRIG1 PWM3 IRI DMD DCK DO03 DI03  ×    ×  |  ×     ×     ×     ×    ×    ×  | ×    ×  |MICP|
 √  √  ×  ×  ×|A04|P04|GP04 JTCK TRX0 RXD1 CSEL SCL BCLK TRIG2 PWM4 IRO DMC DCK DO04 DI04  ×    ×  |  ×     ×     ×     ×    ×    ×  | ×    ×  |MICP|
 √  √  ×  ×  ×|A05|P05|GP05 JTMS RXD0 TRX1 MISO SDA LRCK TRIG3 PWM5 IRI DMD DCK DO05 DI05  ×    ×  |  ×     ×     ×     ×    ×    ×  | ×    ×  |MICN|
 √  √  ×  ×  ×|A06|P06|GP06 JTCK TRX0 RXD1 MOSI SCL DATI TRIG4 PWM0 IRO DMC DCK DO06 DI06  ×    ×  |  ×     ×     ×     ×    ×    ×  | ×    ×  |    |
 √  √  ×  ×  ×|A07|P07|GP07 JTMS RXD0 TRX1 SCLK SDA DATO TRIG5 PWM1 IRI DMD DCK DO07 DI07  ×    ×  |  ×     ×     ×     ×    ×    ×  | ×    ×  |    |
 √  √  ×  ×  ×|A08|P08|GP08 JTCK TRX0 RXD1 CSEL SCL DATI TRIG0 PWM2 IRO DMC DCK DO08 DI08  ×    ×  |  ×     ×     ×     ×    ×    ×  | ×    ×  |    |
 √  √  ×  ×  ×|A09|P09|GP09 JTDO RTS0 CTS1 MISO SDA DATO TRIG1 PWM3 IRI DMD DCK DO09 DI09  ×    ×  |  ×     ×     ×     ×    ×    ×  | ×    ×  |    |
 √  √ 21 13  ×|A10|P10|GP10 JTDI CTS0 RTS1 MOSI SCL BCLK TRIG2 PWM4 IRO DMC DCK DO10 DI10 CBTG  ×  |  ×     ×     ×     ×    ×    ×  | ×    ×  |    |
 √  √ 20 11  ×|A11|P11|JTDI GP11 RXD0 TRX1 SCLK SDA LRCK TRIG3 PWM5 IRI DMD DCK DO11 DI11 CBTG AN11|  ×     ×     ×     ×    ×    ×  | ×    ×  |24MI|
 √  √ 19 10  ×|A12|P12|JTDO GP12 TRX0 RXD1 CSEL SCL DATI TRIG4 PWM0 IRO DMC DCK DO12 DI12 CBTG AN12|  ×     ×     ×     ×    ×    ×  | ×    ×  |24MO|
 √  ×  ×  ×  ×|FCK|P13|FCK  GP13  ×    ×    ×    ×   ×    ×     ×    ×   ×   ×   ×    ×    ×    ×  |  ×     ×     ×     ×    ×    ×  | ×    ×  |    |
 √  ×  ×  ×  ×|FCS|P14|FCS  GP14  ×    ×    ×    ×   ×    ×     ×    ×   ×   ×   ×    ×    ×    ×  |  ×     ×     ×     ×    ×    ×  | ×    ×  |    |
 √  ×  ×  ×  ×|FSI|P15|FSI  GP15  ×    ×    ×    ×   ×    ×     ×    ×   ×   ×   ×    ×    ×    ×  |  ×     ×     ×     ×    ×    ×  | ×    ×  |    |
 √  ×  ×  ×  ×|FSO|P16|FSO  GP16  ×    ×    ×    ×   ×    ×     ×    ×   ×   ×   ×    ×    ×    ×  |  ×     ×     ×     ×    ×    ×  | ×    ×  |    |
 √  ×  ×  ×  ×|FWP|P17|FWP  GP17  ×    ×    ×    ×   ×    ×     ×    ×   ×   ×   ×    ×    ×    ×  |  ×     ×     ×     ×    ×    ×  | ×    ×  |    |
 √  ×  ×  ×  ×|FHD|P18|FHD  GP18  ×    ×    ×    ×   ×    ×     ×    ×   ×   ×   ×    ×    ×    ×  |  ×     ×     ×     ×    ×    ×  | ×    ×  |    |
 √  √ 01 01 01|B00|P19|JTCK GP19 RXD0 CTS1 MISO SDA BCLK TRIG5 PWM1 IRI DMD DCK  ×    ×    ×    ×  |^GPB00 DO00 AONO00 WK00 32KI AN00| ×    ×  |CLDN|
 √  √ 02 02 02|B01|P20|JTMS GP20 TRX0 RTS1 MOSI SCL LRCK TRIG0 PWM2 IRO DMC DCK  ×    ×    ×    ×  |^GPB01 DO01 AONO01 WK01 32KI AN01| ×    ×  |CLDP|
 √  √ 05 05 04|B02|P21|GP21 JTDI RTS0 TRX1 SCLK SDA DATI TRIG1 PWM3 IRI DMD DCK DO13 DI13  x    ×  |AONO02 DO02 ^GPB02 WK02 32KI AN02| ×    ×  |MICP|
 √  √ 06 06  ×|B03|P22|GP22 JTDO CTS0 RXD1 CSEL SCL DATO TRIG2 PWM4 IRO DMC DCK DO14 DI14  x    ×  |AONO03 DO03 ^GPB03 WK03 32KI AN03| ×    ×  |MICN|
 √  √ 09  ×  ×|B04|P23|GP23 JTCK RXD0 CTS1 MISO SDA BCLK TRIG3 PWM5 IRI DMD DCK DO15 DI15  x    ×  |^GPB04 DO04 AONO04 WK04 32KI AN04|CBT0  ×  |    |
 √  √ 10  ×  ×|B05|P24|GP24 JTMS TRX0 RTS1 MOSI SCL LRCK TRIG4 PWM0 IRO DMC DCK DO00 DI00  x    ×  |^GPB05 DO05 AONO05 WK05 32KI AN05|CBT1  ×  |    |
 √  √ 11  ×  ×|B06|P25|GP25 JTCK RTS0 TRX1 SCLK SDA DATI TRIG5 PWM1 IRI DMD DCK DO01 DI01  x    ×  |^GPB06 DO06 AONO06 WK06 32KI AN06|CBT2  ×  |    |
 √  √ 18  ×  ×|B07|P26|GP26 JTMS CTS0 RXD1 CSEL SCL DATO PAUSE PWM2 IRO DMC DCK DO02 DI02  x    ×  |^GPB07 DO07 AONO07 WK07 32KI AN07|CBT3  ×  |    |
 √  √ 12  ×  ×|B08|P27|GP27 JTCK RXD0 TRX1 MISO SDA BCLK T1CLK PWM3 IRI DMD DCK DO03 DI03  x    ×  |^GPB08 DO08 AONO08 WK08 32KI AN08|CBT4  ×  |    |
 √  √ 13  ×  ×|B09|P28|GP28 JTMS TRX0 RXD1 MOSI SCL LRCK TRIG0 PWM4 IRO DMC DCK DO04 DI04  x    ×  |^GPB09 DO09 AONO09 WK09 32KI AN09|CBT5 ADC0|MICN|
 √  √ 14  ×  ×|B10|P29|GP29 JTCK RXD0 TRX1 SCLK SDA DATI TRIG1 PWM5 IRI DMD DCK DO05 DI05  x    ×  |^GPB10 DO10 AONO10 WK10 32KI AN10|CBT6 ADC1|    |
 √  √ 15  9  ×|B11|P30|GP30 JTMS TRX0 RXD1 SCLK SCL DATO TRIG2 PWM0 IRO DMC DCK DO06 DI06  x    ×  |^GPB11 DO11 AONO11 WK11 32KI AN11|CBT7 ADC2|    |
 √  √ 16  ×  ×|B12|P31|GP31 JTCK RXD0 TRX1 CSEL SDA BCLK TRIG3 PWM1 IRI DMD DCK DO07 DI07 CBTG  ×  |^GPB12 DO12 AONO12 WK12 32KI AN12|KSEN ADC3|    |
**************|************************************************************************************|*********************************|*********|****/

const char *__strs__ cmndesc[][16] = {
    [ 0] = { "GPIOA.00", "JTAG.TCK", "UART0.TRX", "UART1.CTS", "SPI.CSEL", "I2C.SCL", "I2S.BCLK", "GPT.PAUSE", "PWM0", "IR.O", "DMIC.CLK", "DCK", "DO00", "DI00", "-"   , "-"     , },
    [ 1] = { "GPIOA.01", "JTAG.TMS", "UART0.RXD", "UART1.RTS", "SPI.MISO", "I2C.SDA", "I2S.LRCK", "GPT.T1CLK", "PWM1", "IR.I", "DMIC.DAT", "DCK", "DO01", "DI01", "-"   , "-"     , },
    [ 2] = { "GPIOA.02", "JTAG.TCK", "UART0.TRX", "UART1.RXD", "SPI.MOSI", "I2C.SCL", "I2S.DATI", "GPT.TRIG0", "PWM2", "IR.O", "DMIC.CLK", "DCK", "DO02", "DI02", "-"   , "-"     , },
    [ 3] = { "GPIOA.03", "JTAG.TMS", "UART0.RXD", "UART1.TRX", "SPI.SCLK", "I2C.SDA", "I2S.DATO", "GPT.TRIG1", "PWM3", "IR.I", "DMIC.DAT", "DCK", "DO03", "DI03", "-"   , "-"     , },
    [ 4] = { "GPIOA.04", "JTAG.TCK", "UART0.TRX", "UART1.RXD", "SPI.CSEL", "I2C.SCL", "I2S.BCLK", "GPT.TRIG2", "PWM4", "IR.O", "DMIC.CLK", "DCK", "DO04", "DI04", "-"   , "-"     , },
    [ 5] = { "GPIOA.05", "JTAG.TMS", "UART0.RXD", "UART1.TRX", "SPI.MISO", "I2C.SDA", "I2S.LRCK", "GPT.TRIG3", "PWM5", "IR.I", "DMIC.DAT", "DCK", "DO05", "DI05", "-"   , "-"     , },
    [ 6] = { "GPIOA.06", "JTAG.TCK", "UART0.TRX", "UART1.RXD", "SPI.MOSI", "I2C.SCL", "I2S.DATI", "GPT.TRIG4", "PWM0", "IR.O", "DMIC.CLK", "DCK", "DO06", "DI06", "-"   , "-"     , },
    [ 7] = { "GPIOA.07", "JTAG.TMS", "UART0.RXD", "UART1.TRX", "SPI.SCLK", "I2C.SDA", "I2S.DATO", "GPT.TRIG5", "PWM1", "IR.I", "DMIC.DAT", "DCK", "DO07", "DI07", "-"   , "-"     , },
    [ 8] = { "GPIOA.08", "JTAG.TCK", "UART0.TRX", "UART1.RXD", "SPI.CSEL", "I2C.SCL", "I2S.DATI", "GPT.TRIG0", "PWM2", "IR.O", "DMIC.CLK", "DCK", "DO08", "DI08", "-"   , "-"     , },
    [ 9] = { "GPIOA.09", "JTAG.TDO", "UART0.RTS", "UART1.CTS", "SPI.MISO", "I2C.SDA", "I2S.DATO", "GPT.TRIG1", "PWM3", "IR.I", "DMIC.DAT", "DCK", "DO09", "DI09", "-"   , "-"     , },
    [10] = { "GPIOA.10", "JTAG.TDI", "UART0.CTS", "UART1.RTS", "SPI.MOSI", "I2C.SCL", "I2S.BCLK", "GPT.TRIG2", "PWM4", "IR.O", "DMIC.CLK", "DCK", "DO10", "DI10", "CBTG", "-"     , },
    [11] = { "JTAG.TDI", "GPIOA.11", "UART0.RXD", "UART1.TRX", "SPI.SCLK", "I2C.SDA", "I2S.LRCK", "GPT.TRIG3", "PWM5", "IR.I", "DMIC.DAT", "DCK", "DO11", "DI11", "CBTG", "ANA.11", },
    [12] = { "JTAG.TDO", "GPIOA.12", "UART0.TRX", "UART1.RXD", "SPI.CSEL", "I2C.SCL", "I2S.DATI", "GPT.TRIG4", "PWM0", "IR.O", "DMIC.CLK", "DCK", "DO12", "DI12", "CBTG", "ANA.12", },
    [13] = { "FLASH.CK", "GPIOA.13", "-"        , "-"        , "-"       , "-"      , "-"       , "-"        , "-"   , "-"   , "-"       , "-"  , "-"   , "-"   , "-"   , "-"     , },
    [14] = { "FLASH.CS", "GPIOA.14", "-"        , "-"        , "-"       , "-"      , "-"       , "-"        , "-"   , "-"   , "-"       , "-"  , "-"   , "-"   , "-"   , "-"     , },
    [15] = { "FLASH.SI", "GPIOA.15", "-"        , "-"        , "-"       , "-"      , "-"       , "-"        , "-"   , "-"   , "-"       , "-"  , "-"   , "-"   , "-"   , "-"     , },
    [16] = { "FLASH.SO", "GPIOA.16", "-"        , "-"        , "-"       , "-"      , "-"       , "-"        , "-"   , "-"   , "-"       , "-"  , "-"   , "-"   , "-"   , "-"     , },
    [17] = { "FLASH.WP", "GPIOA.17", "-"        , "-"        , "-"       , "-"      , "-"       , "-"        , "-"   , "-"   , "-"       , "-"  , "-"   , "-"   , "-"   , "-"     , },
    [18] = { "FLASH.HD", "GPIOA.18", "-"        , "-"        , "-"       , "-"      , "-"       , "-"        , "-"   , "-"   , "-"       , "-"  , "-"   , "-"   , "-"   , "-"     , },
    [19] = { "JTAG.TCK", "GPIOB.00", "UART0.RXD", "UART1.CTS", "SPI.MISO", "I2C.SDA", "I2S.BCLK", "GPT.TRIG5", "PWM1", "IR.I", "DMIC.DAT", "DCK", "-"   , "-"   , "-"   , "-"     , },
    [20] = { "JTAG.TMS", "GPIOB.01", "UART0.TRX", "UART1.RTS", "SPI.MOSI", "I2C.SCL", "I2S.LRCK", "GPT.TRIG0", "PWM2", "IR.O", "DMIC.CLK", "DCK", "-"   , "-"   , "-"   , "-"     , },
    [21] = { "GPIOB.02", "JTAG.TDI", "UART0.RTS", "UART1.TRX", "SPI.SCLK", "I2C.SDA", "I2S.DATI", "GPT.TRIG1", "PWM3", "IR.I", "DMIC.DAT", "DCK", "DO13", "DI13", "-"   , "-"     , },
    [22] = { "GPIOB.03", "JTAG.TDO", "UART0.CTS", "UART1.RXD", "SPI.CSEL", "I2C.SCL", "I2S.DATO", "GPT.TRIG2", "PWM4", "IR.O", "DMIC.CLK", "DCK", "DO14", "DI14", "-"   , "-"     , },
    [23] = { "GPIOB.04", "JTAG.TCK", "UART0.RXD", "UART1.CTS", "SPI.MISO", "I2C.SDA", "I2S.BCLK", "GPT.TRIG3", "PWM5", "IR.I", "DMIC.DAT", "DCK", "DO15", "DI15", "-"   , "-"     , },
    [24] = { "GPIOB.05", "JTAG.TMS", "UART0.TRX", "UART1.RTS", "SPI.MOSI", "I2C.SCL", "I2S.LRCK", "GPT.TRIG4", "PWM0", "IR.O", "DMIC.CLK", "DCK", "DO00", "DI00", "-"   , "-"     , },
    [25] = { "GPIOB.06", "JTAG.TCK", "UART0.RTS", "UART1.TRX", "SPI.SCLK", "I2C.SDA", "I2S.DATI", "GPT.TRIG5", "PWM1", "IR.I", "DMIC.DAT", "DCK", "DO01", "DI01", "-"   , "-"     , },
    [26] = { "GPIOB.07", "JTAG.TMS", "UART0.CTS", "UART1.RXD", "SPI.CSEL", "I2C.SCL", "I2S.DATO", "GPT.PAUSE", "PWM2", "IR.O", "DMIC.CLK", "DCK", "DO02", "DI02", "-"   , "-"     , },
    [27] = { "GPIOB.08", "JTAG.TCK", "UART0.RXD", "UART1.TRX", "SPI.MISO", "I2C.SDA", "I2S.BCLK", "GPT.T1CLK", "PWM3", "IR.I", "DMIC.DAT", "DCK", "DO03", "DI03", "-"   , "-"     , },
    [28] = { "GPIOB.09", "JTAG.TMS", "UART0.TRX", "UART1.RXD", "SPI.MOSI", "I2C.SCL", "I2S.LRCK", "GPT.TRIG0", "PWM4", "IR.O", "DMIC.CLK", "DCK", "DO04", "DI04", "-"   , "-"     , },
    [29] = { "GPIOB.10", "JTAG.TCK", "UART0.RXD", "UART1.TRX", "SPI.SCLK", "I2C.SDA", "I2S.DATI", "GPT.TRIG1", "PWM5", "IR.I", "DMIC.DAT", "DCK", "DO05", "DI05", "-"   , "-"     , },
    [30] = { "GPIOB.11", "JTAG.TMS", "UART0.TRX", "UART1.RXD", "SPI.SCLK", "I2C.SCL", "I2S.DATO", "GPT.TRIG2", "PWM0", "IR.O", "DMIC.CLK", "DCK", "DO06", "DI06", "-"   , "-"     , },
    [31] = { "GPIOB.12", "JTAG.TCK", "UART0.RXD", "UART1.TRX", "SPI.CSEL", "I2C.SDA", "I2S.BCLK", "GPT.TRIG3", "PWM1", "IR.I", "DMIC.DAT", "DCK", "DO07", "DI07", "CBTG", "-"     , },
};
const char *__strs__ aondesc[][6] = {
    [ 0] = { "GPB.00", "DO00", "AON.00", "WAKE.00", "X32K.I", "ANIO.00", },
    [ 1] = { "GPB.01", "DO01", "AON.01", "WAKE.01", "X32K.I", "ANIO.01", },
    [ 2] = { "AON.02", "DO02", "GPB.02", "WAKE.02", "X32K.I", "ANIO.02", },
    [ 3] = { "AON.03", "DO03", "GPB.03", "WAKE.03", "X32K.I", "ANIO.03", },
    [ 4] = { "GPB.04", "DO04", "AON.04", "WAKE.04", "X32K.I", "ANIO.04", },
    [ 5] = { "GPB.05", "DO05", "AON.05", "WAKE.05", "X32K.I", "ANIO.05", },
    [ 6] = { "GPB.06", "DO06", "AON.06", "WAKE.06", "X32K.I", "ANIO.06", },
    [ 7] = { "GPB.07", "DO07", "AON.07", "WAKE.07", "X32K.I", "ANIO.07", },
    [ 8] = { "GPB.08", "DO08", "AON.08", "WAKE.08", "X32K.I", "ANIO.08", },
    [ 9] = { "GPB.09", "DO09", "AON.09", "WAKE.09", "X32K.I", "ANIO.09", },
    [10] = { "GPB.10", "DO10", "AON.10", "WAKE.10", "X32K.I", "ANIO.10", },
    [11] = { "GPB.11", "DO11", "AON.11", "WAKE.11", "X32K.I", "ANIO.11", },
    [12] = { "GPB.12", "DO12", "AON.12", "WAKE.12", "X32K.I", "ANIO.12", },
};
const char *__strs__ anadesc[][2] = {
    [0 ... 3] = { "-", "-"     , },
    [ 4] = { "CBTN.0", "-"     , },
    [ 5] = { "CBTN.1", "-"     , },
    [ 6] = { "CBTN.2", "-"     , },
    [ 7] = { "CBTN.3", "-"     , },
    [ 8] = { "CBTN.4", "-"     , },
    [ 9] = { "CBTN.5", "ADC.0" , },
    [10] = { "CBTN.6", "ADC.1" , },
    [11] = { "CBTN.7", "ADC.2" , },
    [12] = { "KEYSEN", "ADC.3" , },
};
const char *__strs__ bondesc[] = {
    [ 3] = "AMIC.P", [ 4] = "AMIC.P",  [5] = "AMIC.N",
    [11] = "X24M.I", [12] = "X24M.O",
    [19] = "CLSD.N", [20] = "CLSD.P", [21] = "AMIC.P", [22] = "AMIC.N",
    [28] = "AMIC.N",
    [0 ... 2] = "-", [6 ... 10] = "-", [13 ... 18] = "-", [23 ... 31] = "-"
};

typedef union {
    volatile unsigned all;
    struct {
        volatile unsigned FSEL         : 5; // bit 00~04
        volatile unsigned ANA_SEL      : 4; // bit 05~08
        volatile unsigned OD_EN        : 1; // bit 09~09
        volatile unsigned RESV_10_15   : 6; // bit 10~15
        volatile unsigned PULL_DN      : 1; // bit 16~16
        volatile unsigned PULL_UP      : 1; // bit 17~17
        volatile unsigned PULL_FRC     : 1; // bit 18~18
        volatile unsigned IE_REG       : 1; // bit 19~19
        volatile unsigned IE_FRC       : 1; // bit 20~20
        volatile unsigned OEN_REG      : 1; // bit 21~21
        volatile unsigned OEN_FRC      : 1; // bit 22~22
        volatile unsigned OUT_REG      : 1; // bit 23~23
        volatile unsigned OUT_FRC      : 1; // bit 24~24
        volatile unsigned DRV          : 2; // bit 25~26
        volatile unsigned ST           : 1; // bit 27~27
        volatile unsigned SR           : 1; // bit 28~28
        volatile unsigned RESV_29_31   : 3; // bit 29~31
    };
} volatile cmn_iomux_t;

typedef union {
    volatile unsigned all;
    struct {
        volatile unsigned FSEL         : 3; // bit 00~02
        volatile unsigned RESV_3_4     : 2; // bit 03~04
        volatile unsigned ANA_SEL      : 4; // bit 05~08
        volatile unsigned OD_EN        : 1; // bit 09~09
        volatile unsigned RESV_10_15   : 6; // bit 10~15
        volatile unsigned PULL_DN      : 1; // bit 16~16
        volatile unsigned PULL_UP      : 1; // bit 17~17
        volatile unsigned PULL_FRC     : 1; // bit 18~18
        volatile unsigned RESV_19_20   : 2; // bit 19~20
        volatile unsigned OEN_REG      : 1; // bit 21~21
        volatile unsigned OEN_FRC      : 1; // bit 22~22
        volatile unsigned RESV_23_24   : 2; // bit 23~24
        volatile unsigned DRV          : 1; // bit 25~25
        volatile unsigned RESV_26_31   : 6; // bit 26~31
    };
} volatile aon_iomux_t;

typedef struct {
    mod_inst_t self;
    x_handle_t hdrv[2];
    cmn_iomux_t *cmn;
    aon_iomux_t *aon;
} obj_priv_t;

typedef union __packed__ {
    uint32_t uarg;
    struct __packed__ {
        union __packed__ {
            uint8_t upin;
            struct __packed__ {
                uint8_t pin   : 4; // [03:00] 0~15
                uint8_t port  : 4; // [07:04] 0=PORTA 1=PORTB
            };
        };
        uint8_t                : 8;
        union __packed__ {
            uint16_t umux;
            struct __packed__ {
                uint16_t cmn   : 4; // [03:00] 0-15
                uint16_t aon   : 4; // [06:04] 0-5
                uint16_t ana   : 4; // [08:07] 0-2
                uint16_t       : 4;
            };
        };
    };
} volatile gpio_desc_t;

static const gpio_desc_t strsct[] = {
    // PORT/PIN    ANA/AON/CMN
#if CONFIG_TRACE_UART0 || CONFIG_UPROT_UART0
    { .upin=0x00, .umux=0x002 }, // A00: TRX0
#else
    { .upin=0x00, .umux=0x000 }, // A00: GPA00
#endif

#if CONFIG_I2SO_ENABLE
    { .upin=0x01, .umux=0x006 }, // A01: LRCK
    { .upin=0x0a, .umux=0x006 }, // A10: BCLK
    { .upin=0x1b, .umux=0x006 }, // B11: I2SO
#else
#if CONFIG_TRACE_UART0 || CONFIG_UPROT_UART0
    { .upin=0x01, .umux=0x002 }, // A01: RXD0
#else
    { .upin=0x01, .umux=0x000 }, // A01: GPA01
#endif
    { .upin=0x0a, .umux=0x000 }, // A10: GPA10
    { .upin=0x1b, .umux=0x000 }, // B11: GPB11
#endif

#if CONFIG_TRACE_UART1 || CONFIG_UPROT_UART1
    { .upin=0x0b, .umux=0x003 }, // A11: TXD1
    { .upin=0x0c, .umux=0x003 }, // A12: RXD1
#else
    { .upin=0x0b, .umux=0x001 }, // A11: GPA11
    { .upin=0x0c, .umux=0x001 }, // A12: GPA12
#endif

#if CONFIG_JTAG_ENABLE
    { .upin=0x10, .umux=0x000 }, // B00: TCK
    { .upin=0x11, .umux=0x000 }, // B01: TMS
#elif CONFIG_CLSD_ENABLE
    { .upin=0x10, .umux=0x050 }, // B00: CLSDN
    { .upin=0x11, .umux=0x050 }, // B01: CLSDP
#else
    { .upin=0x10, .umux=0x001 }, // B00: GPB00
    { .upin=0x11, .umux=0x001 }, // B01: GPB01
#endif

#if CONFIG_AADC_ENABLE
    { .upin=0x12, .umux=0xC50 }, // B02: MICP
    { .upin=0x13, .umux=0xC50 }, // B03: MICN
#else
    { .upin=0x12, .umux=0x020 }, // B02: GPB02
    { .upin=0x13, .umux=0x020 }, // B03: GPB03
#endif
};

////////////////////////////////////////////////////////////////////////////////////////////////////
void shell_gpio_dump(void)
{
    SHELL_ITEM_EXPORT("iodump", shell_gpio_dump, "iodump");
    obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_GPIO_NAME);

    TRACE(COR_FG_CYAN);
    TRACE("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    TRACE(" %14s  %-5s  %-5s  %3s(%3s %3s %3s)  %-6s  %-10s  %-4s  %-6s"
        , "48 32 24 16 08", "PIN", "PAD", "DOM", "AON", "ANA", "CMN", "BOND", "MUXSEL", "PULL", "I/O");
    TRACE(" ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ");
    for (int pin = 0; pin < 32; pin++) {
        switch (pin) {
        case  0 ... 12: PRINTK(COR_FG_YELLOW); break;
        case 13 ... 18: PRINTK(COR_FG_GREEN); break;
        case 19 ... 31: PRINTK(COR_FG_FUCHSIN); break;
        }

        const char *domain = NULL, *func = NULL;
        const int idx = pin % 19;
        const uint32_t msk = BIT(pin);
        if ((pin < 19) 
                || ((pin == 21 || pin == 22) && priv->aon[idx].FSEL == CSK_AON_IOMUX_FUNC_ALTER2) 
                || ((pin != 21 && pin != 22) && priv->aon[idx].FSEL == CSK_AON_IOMUX_FUNC_DEFAULT))
            domain = "CMN", func = cmndesc[pin][priv->cmn[pin].FSEL];
        else if (priv->aon[idx].FSEL == CSK_AON_IOMUX_FUNC_ALTER5)
            domain = "ANA", func = anadesc[idx][priv->aon[idx].ANA_SEL];
        else domain = "AON", func = aondesc[idx][priv->aon[idx].FSEL];
        PRINTK("  %s  %s  %s  %s  %s  PIN%02d  GP%c%02d  %3s"
            , msk & 0xFFFFFFFF ? "√" : "×", msk & 0xFFF81FFF ? "√" : "×"
            , msk & 0xFFF81C03 ? "√" : "×", msk & 0x40781C03 ? "√" : "×"
            , msk & 0x00380001 ? "√" : "×", pin, 'A' + pin / 19, idx, domain);
        if (pin < 19) PRINTK("(%3s %3d %3d)", "-", priv->cmn[pin].ANA_SEL, priv->cmn[pin].FSEL);
        else PRINTK("(%3d %3d %3d)", priv->aon[idx].FSEL, priv->aon[idx].ANA_SEL, priv->cmn[pin].FSEL);
        PRINTK("  %-6s  ", bondesc[pin]);
        if ('G' != func[0] || 'P' != func[1])
            PRINTK("%-10s", func);
        else {
            PRINTK(COR_FG_BLUE "%-10s  ", func);
            if (priv->cmn[pin].PULL_FRC) {
                if (priv->cmn[pin].PULL_UP) PRINTK("PU");
                else if (priv->cmn[pin].PULL_DN) PRINTK("PD");
                else PRINTK("P-");
            }
            else if (!(IP_GPIO->REG_PULLEN.all & msk)) PRINTK("p-");
            else if (IP_GPIO->REG_PULLTYPE.all & msk) PRINTK("pu");
            else PRINTK("pd");
            PRINTK("    ");
            if (priv->cmn[pin].OEN_FRC && priv->cmn[pin].IE_FRC) PRINTK("FIO");
            else if (priv->cmn[pin].OEN_FRC) PRINTK("FO=%d", priv->cmn[pin].OEN_REG);
            else if (priv->cmn[pin].IE_FRC) PRINTK("FI=%d", priv->cmn[pin].IE_REG);
            else if (IP_GPIO->REG_CHANNELDIR.all & msk) PRINTK("O=%c", IP_GPIO->REG_DATAOUT.all & msk ? 'H' : 'L');
            else PRINTK("I=%c", IP_GPIO->REG_DATAIN.all & msk ? 'H' : 'L');
        }
        TRACE(CORDEF);
    }
    PRINTK(COR_FG_CYAN);
    TRACE("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" CORDEF);
}

static void __fast__ gpio_porta_isr(uint32_t evt, void *user)
{
    obj_priv_t *const priv = user;
	GPIO_Control(priv->hdrv[0], CSK_GPIO_INTR_DISABLE, evt);
    uint32_t which = evt;   // TODO;
    __mapi_notify(priv->self, which, NULL);
}

static void __fast__ gpio_portb_isr(uint32_t evt, void *user)
{
    obj_priv_t *const priv = user;
	GPIO_Control(priv->hdrv[1], CSK_GPIO_INTR_DISABLE, evt);
    uint32_t which = evt;   // TODO;
    __mapi_notify(priv->self, which, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void mod_setup(mod_inst_t self, void *user)
{
    obj_priv_t *const priv = user;
    priv->self = self;
    priv->hdrv[0] = GPIOA();
    priv->hdrv[1] = GPIOB();
    GPIO_Initialize(priv->hdrv[0], gpio_porta_isr, priv);
    GPIO_Initialize(priv->hdrv[1], gpio_portb_isr, priv);

    for (int pin = 0; pin < __cntof(strsct); pin++) {
        const gpio_desc_t *const desc = &strsct[pin];
        if (desc->port == CSK_IOMUX_PAD_A)
            priv->cmn[desc->pin].FSEL = desc->cmn;
        else {
            if (priv->aon[desc->pin].FSEL == CSK_AON_IOMUX_FUNC_ALTER5)
                priv->aon[desc->pin].ANA_SEL = desc->ana;
            else
                priv->cmn[desc->pin + 19].ANA_SEL = desc->ana;
            priv->cmn[desc->pin + 19].FSEL = desc->cmn;
            priv->aon[desc->pin].FSEL = desc->aon;
        }
    }

    // MICP(A3/A4/B2)|MICN(A5/B3/B9)|CLSD(B0/B1):OEN_FRC=OEN_REG=IE_FRC=PULL_FRC=1,IE_REG=ANA_SEL=FSEL=0
    priv->aon[0].FSEL = 0;          // B0(AON_FSEL=0)
    priv->aon[1].FSEL = 0;          // B1(AON_FSEL=0)
    priv->aon[2].FSEL = 2;          // B2(AON_FSEL=2)
    priv->aon[3].FSEL = 2;          // B3(AON_FSEL=2)
    priv->aon[9].FSEL = 0;          // B9(AON_FSEL=0)
    priv->cmn[3 ].all = 0x00740000; // A3(OEN_FRC=OEN_REG=IE_FRC=PULL_FRC=1, IE_REG=FSEL=ANA_SEL=0)
    priv->cmn[4 ].all = 0x00740000; // A4(OEN_FRC=OEN_REG=IE_FRC=PULL_FRC=1, IE_REG=FSEL=ANA_SEL=0)
    priv->cmn[5 ].all = 0x00740000; // A5(OEN_FRC=OEN_REG=IE_FRC=PULL_FRC=1, IE_REG=FSEL=ANA_SEL=0)
    priv->cmn[21].all = 0x00740000; // B2(OEN_FRC=OEN_REG=IE_FRC=PULL_FRC=1, IE_REG=FSEL=ANA_SEL=0)
    priv->cmn[22].all = 0x00740000; // B3(OEN_FRC=OEN_REG=IE_FRC=PULL_FRC=1, IE_REG=FSEL=ANA_SEL=0)
    priv->cmn[28].all = 0x00740000; // B9(OEN_FRC=OEN_REG=IE_FRC=PULL_FRC=1, IE_REG=FSEL=ANA_SEL=0)
}

static obj_priv_t mod_priv = {
    .aon = (void *)IP_AON_IOMUX,
    .cmn = (void *)IP_CMN_IOMUX
};
static mod_apis_t mod_apis = {
    .setup = mod_setup,
};
__MAPI_DECLARE(CONFIG_GPIO_NAME, 0, 2, &mod_apis, &mod_priv);

#endif//CONFIG_GPIO_ENABLE
