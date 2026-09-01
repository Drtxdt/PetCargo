#include "appinc.h"
#include "luna.h"

pmu_rstsrc_t power_reset_src;
extern void vPortBacktrace(void);
extern void shell_flush_cached(void);
extern void shell_task_summary(void);
extern void shell_heap_summary(void);

static const char __stub__ sysinfo[] = "<LISTENAI(MARS):" 
    N2STR(VER_MAJOR) "." N2STR(VER_MINOR) "." N2STR(VER_BUILD) "@" __DATE__ "." __TIME__ "/>";

static const char *__strs__ exctab[] = {
    [InsUnalign_EXCn   ] = "InstAddrUnalign",
    [InsAccFault_EXCn  ] = "InstAccFault",
    [IlleIns_EXCn      ] = "InstIllegal",
    [Break_EXCn        ] = "Beakpoint",
    [LdAddrUnalign_EXCn] = "LoadAddrUnalign",
    [LdFault_EXCn      ] = "LoadAccFault",
    [StAddrUnalign_EXCn] = "StoreAddrUnalign",
    [StAccessFault_EXCn] = "StoreAccFault",
    [UmodeEcall_EXCn   ] = "U-ECall",
    [SmodeEcall_EXCn   ] = "S-ECall",
    [MmodeEcall_EXCn   ] = "M-ECall",
    [InsPageFault_EXCn ] = "InstPageFault",
    [LdPageFault_EXCn  ] = "LoadPageFault",
    [StPageFault_EXCn  ] = "StorePageFault",
};

static void hart_excep_handler(uint32_t mcause, uint32_t sp)
{
    XDGINT();
    __HAL_PMU_SW_RST_AON_RST_ENABLE();
    
    EXC_Frame_Type *const excfrm = (void *)sp;
    EXCn_Type exc = (mcause & MCAUSE_CAUSE);
    if (MmodeEcall_EXCn == exc) { excfrm->epc += 4; return; }

    shell_flush_cached();
    
    TRACE(CRLF COR_FG_YELLOW "Exception: " CORDEF);
    TRACE("CAUSE=%p DCAUSE=%p EPC=%p EXC=%d(%s)"
        , (x_handle_t)excfrm->cause, (x_handle_t)__RV_CSR_READ(CSR_MDCAUSE)
        , (x_handle_t)excfrm->epc, exc, exc < __cntof(exctab) ? exctab[exc] : "Unknown!");
    
    vPortBacktrace();
    shell_task_summary();
    shell_heap_summary();
    
    TRACE(CRLF CORDEF "==== HART HALTED====" CORDEF CRLF);

    // for debug
    volatile bool loop = true;
    while (loop) XDGINT();
}

static void hart_handler_init(void)
{
    ASSERT(__RISCV_XLEN == 32, "XLEN(%d)", __RISCV_XLEN);
    for (EXCn_Type exc = InsUnalign_EXCn; exc <= StPageFault_EXCn; exc++)
        Exception_Register_EXC(exc, (uint32_t)hart_excep_handler);
}

static void sys_boot_reason(void)
{
    const pmu_rstsrc_t rstby = HAL_PMU_GetSysResetCause();
    const pmu_wakeupsrc_t wakby = HAL_PMU_GetWakeUpCause();
    const char *rststr = NULL;
    switch (rstby) {
    case PMU_RST_POR:         rststr = "POR" ; break;
    case PMU_RST_AON:         rststr = "AON" ; break;
    case PMU_RST_SYSCFG_P_AP: rststr = "AP"  ; break;
    case PMU_RST_SYSRSTREQ:   rststr = "SW"  ; break;
    case PMU_RST_NONE:        rststr = "NONE"; break;
    default:                  rststr = "-"   ; break;
    }
    const char *wakstr = NULL;
    switch (wakby) {
    case PMU_WAKEUP_GPIOB_00
         ...
         PMU_WAKEUP_GPIOB_12: wakstr = "GPIO"; break;
    case PMU_WAKEUP_TIMER:    wakstr = "TMR" ; break;
    case PMU_WAKEUP_IWDT:     wakstr = "WDT" ; break;
    case PMU_WAKEUP_KEY:      wakstr = "KEY" ; break;
    case PMU_WAKEUP_NONE:     wakstr = "NONE"; break;
    default:                  wakstr = "-"   ; break;
    }
    power_reset_src = rstby;
    TRACE("RESET=%#x/%s WAKEUP=%#x/%s", rstby, rststr, wakby, wakstr);
}

static void boot_task(void *param)
{
    sys_boot_reason();
    srand(__get_rv_cycle());
#if CONFIG_ALGO_LUNA_ENABLE
    __HAL_CRM_LUNA_CLK_ENABLE();
    luna_init();
#endif
    __HAL_CRM_MTIME_CLK_ENABLE();
    __mapi_setup(__MAPI_STAGE_POST_RTOS);
    int main(int argc, char *argv[]);
    main(0, NULL);
    x_task_del(NULL);
}

void __noret__ entry(void)
{
    TRACE(CRLF "ListenAI.MARS!" CRLF);
    hart_handler_init();
#if CONFIG_XWDT_ENABLE
    IP_AON_CTRL->REG_AON_TUNE1.bit.RCO32K_RTRIM = 0xf;
#endif
    dma_initialize();
    __mapi_preinit();
    __mapi_setup(__MAPI_STAGE_PREV_RTOS);
    x_icache_flush();
    x_task_new("boot", boot_task, OS_STACK_DEF * 3, OS_PRIO_DEF, NULL);
    vTaskStartScheduler();
    ASSERT(false, "exit");
    while (1);
}
