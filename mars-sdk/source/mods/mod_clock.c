#include "appinc.h"

#if CONFIG_CLOCK_ENABLE
void shell_rctrim_set(short trim)
{
    SHELL_ITEM_EXPORT("trim.set", shell_rctrim_set, "trim.set <trim>");
    uint8_t trim1 = (uint8_t)((trim + 16) / 32) & 0x1f, trim2 = (uint8_t)(trim % 32) & 0x1f;
    IP_AON_CTRL->REG_OSC24M_CTRL.bit.OSC24M_OSC_TRIM1 = trim1;          // major: -16:+15
    IP_AON_CTRL->REG_OSC24M_CTRL.bit.OSC24M_OSC_TRIM2 = trim2;          // minor: -16:+15
    TRACE(CRLF "TRIM=%d REGV=0x%04x", trim, IP_AON_CTRL->REG_OSC24M_CTRL.all & 0xffc0);
}

void shell_rctrim_get(void)
{
    SHELL_ITEM_EXPORT("trim.get", shell_rctrim_get, "trim.get");
    uint8_t trim1 = IP_AON_CTRL->REG_OSC24M_CTRL.bit.OSC24M_OSC_TRIM1;  // major: -16:+15
    uint8_t trim2 = IP_AON_CTRL->REG_OSC24M_CTRL.bit.OSC24M_OSC_TRIM2;  // minor: -16:+15
    short trim = (char)(trim1 << 3) * 4 + (char)(trim2 << 3) / 8;
    TRACE(CRLF "TRIM=%d REGV=0x%04x", trim, IP_AON_CTRL->REG_OSC24M_CTRL.all & 0xffc0);
}

void shell_clock_select(int sel)
{
    SHELL_ITEM_EXPORT("cksel", shell_clock_select, "cksel <0~25>");
    IP_SYSCTRL->REG_TEST_CTRL.bit.DBG_CLK_SEL = sel;
    IP_SYSCTRL->REG_TEST_CTRL.bit.DBG_CLK_EN = 1;
    IP_CMN_IOMUX->REG_PAD_GPIOA_10.bit.PAD_GPIOA_10_FSEL = CSK_IOMUX_FUNC_ALTER11;
    int clk = 0;
    switch (sel) {
    case  0: /* RC24M    */ clk = CRM_GetSrcFreq(CRM_IpSrcOscClk); break;
    case  1: /* XTAL24M  */ clk = CRM_GetSrcFreq(CRM_IpSrcXtalClk); break;
    case  2: /* PLLAUD   */ clk = CRM_GetSrcFreq(CRM_IpSrcAudClk); break;
    case  3: /* AON32K   */ clk = CRM_GetSrcFreq(CRM_IpSrcAon32kClk); break;
    case  4: /* GPTCLKS  */ clk = CRM_GetGpt_sFreq(); break;
    case  5: /* GPTCLKT0 */ clk = CRM_GetGpt_t0Freq(); break;
    case  6: /* I2C      */ clk = CRM_GetI2c0Freq(); break;
    case  7: /* SPI      */ clk = CRM_GetSpi0Freq(); break;
    case  8: /* UART0    */ clk = CRM_GetUart0Freq(); break;
    case  9: /* UART1    */ clk = CRM_GetUart1Freq(); break;
    case 10: /* IRCLK    */ clk = CRM_GetIrFreq(); break;
    case 11: /* IRCLRTX  */ clk = CRM_GetIr_txFreq(); break;
    case 12: /* IRDBG    */ clk = -1; break;
    case 13: /* FLASH    */ clk = CRM_GetFlashFreq(); break;
    case 14: /* GPADC    */ clk = CRM_GetGpadcFreq(); break;
    case 15: /* GPIO     */ clk = CRM_GetGpio0Freq(); break;
    case 16: /* APC      */ clk = CRM_GetApcFreq(); break;
    case 17: /* AUDBG    */ clk = CRM_GetCodecFreq(); break;
    case 18: /* HCLK     */ clk = CRM_GetHclkFreq(); break;
    case 19: /* CMNPCLK  */ clk = CRM_GetAp_peri_pclkFreq(); break;
    case 20: /* AONPCLK  */ clk = CRM_GetAon_cfg_pclkFreq(); break;
    case 21: /* MTIME    */ clk = CRM_GetMtimeFreq(); break;
    case 22: /* CORE     */ clk = CRM_GetCpuFreq(); break;
    case 23: /* DMA      */ clk = CRM_GetDmaFreq(); break;
    case 24: /* LUNA     */ clk = CRM_GetLunaFreq(); break;
    case 25: /* I2S      */ clk = CRM_GetI2sFreq(); break;
    default: break;
    }
    TRACE(CRLF "Clock(TYPE=%d FREQ=%d) to GPIOA10", sel, clk);
}

void shell_clock_disp(void)
{
    SHELL_ITEM_EXPORT("clock", shell_clock_disp, "show clocks");
}

static void clock_disp_task(void *param)
{
    x_task_sleep(2);
    const char *const spwire = 
    #if CONFIG_BOARD_FLASH_QPI
        "QPI";
    #elif CONFIG_BOARD_FLASH_DPI
        "DPI";
    #else
        "SPI";
    #endif
    LOGI("%s@%dMHz %s@%dMHz"
        , "CORE", (int)CRM_GetSrcFreq(CRM_IpSrcCoreClk) / 1000000
        , spwire, (int)CRM_GetSrcFreq(CRM_IpSrcFlashClk) / 1000000);
    x_task_del(NULL);
}

static void mod_setup(mod_inst_t self, void *user)
{
#if CONFIG_CLOCK_CORE
#if CONFIG_CLOCK_CORE_300M
    CRM_InitCoreSrc(CRM_IpCore_300MHz);
#elif CONFIG_CLOCK_CORE_240M
    CRM_InitCoreSrc(CRM_IpCore_240MHz);
#elif CONFIG_CLOCK_CORE_200M
    CRM_InitCoreSrc(CRM_IpCore_200MHz);
#elif CONFIG_CLOCK_CORE_150M
    CRM_InitCoreSrc(CRM_IpCore_150MHz);
#elif CONFIG_CLOCK_CORE_133M
    CRM_InitCoreSrc(CRM_IpCore_133MHz);
#elif CONFIG_CLOCK_CORE_120M
    CRM_InitCoreSrc(CRM_IpCore_120MHz);
#elif CONFIG_CLOCK_CORE_100M
    CRM_InitCoreSrc(CRM_IpCore_100MHz);
#endif
    SystemCoreClockUpdate();
#endif

#if CONFIG_CLOCK_FLASH
#if CONFIG_CLOCK_FLASH_120M
    CRM_InitFlashSrc(CRM_IpFlash_120MHz);
#elif CONFIG_CLOCK_FLASH_100M
    CRM_InitFlashSrc(CRM_IpFlash_100MHz);
#elif CONFIG_CLOCK_FLASH_086M
    CRM_InitFlashSrc(CRM_IpFlash_86MHz);
#elif CONFIG_CLOCK_FLASH_080M
    CRM_InitFlashSrc(CRM_IpFlash_80MHz);
#endif
    x_icache_flush();
#endif

    x_task_new(CONFIG_CLOCK_NAME, clock_disp_task, OS_STACK_DEF * 2, OS_PRIO_DEF, NULL);
}

static mod_apis_t mod_apis = {
    .setup = mod_setup,
};
__MAPI_DECLARE(CONFIG_CLOCK_NAME, 0, 0, &mod_apis, NULL);
#endif//CONFIG_CLOCK_ENABLE
