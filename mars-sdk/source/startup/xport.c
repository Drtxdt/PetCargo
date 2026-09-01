#include "appinc.h"

#if 1
#undef errno
extern int errno;
int _open(const char *name, int flags, int mode)
{
    errno = ENOSYS;
    return -1;
}
int _close(int fd)
{
    errno = EBADF;
    return -1;
}
int _stat(char *file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}
int _fstat(int file, struct stat* st)
{
    if ((STDOUT_FILENO == file) || (STDERR_FILENO == file)) {
        st->st_mode = S_IFCHR;
        return 0;
    } else {
        errno = EBADF;
        return -1;
    }
}
int _lseek(int file, int offset, int whence)
{
    errno = EBADF;
    return -1;
}
int _getpid(void)
{
    errno = EINVAL;
    return -1;
}
int _isatty(int fd)
{
    errno = EBADF;
    return -1;
}
int _kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}
ssize_t _read(int fd, void *ptr, size_t len)
{
    return -1;
}
ssize_t _write(int fd, const void *ptr, size_t len)
{
    int shell_task_printf(const char *text);
    return shell_task_printf(ptr);
}
#endif

#if 1
void *_malloc_r(struct _reent *ptr, size_t size)
{
	return pvPortMalloc(size);
}
void *_realloc_r(struct _reent *ptr, void *old, size_t newlen)
{
	if (old) vPortFree(old);
	return pvPortMalloc(newlen);
}
void *_calloc_r(struct _reent *ptr, size_t size, size_t len)
{
	return memset(pvPortMalloc(size * len), 0, size);
}
void _free_r(struct _reent *ptr, void *addr)
{
	if (!addr) vPortFree(addr);
}
#endif

void vPortBacktrace(void)
{
#if CONFIG_MODULE_BTRACE
    #define BTRACE_MAX_DEPTH 32
    uint32_t *const bstack = x_malloc(sizeof(uint32_t) * BTRACE_MAX_DEPTH, sizeof(uint32_t));
    int depth = btrace_dump_stack(bstack, BTRACE_MAX_DEPTH);
    TRACE(CRLF COR_FG_YELLOW "Backtrace:" CORDEF);
    PRINTK(COR_BG_RED CONFIG_TOOLCHAIN_PREFIX CONFIG_BTRACE_PREFIX "build/bin/app.elf");
    for (int i = 0; i < depth; i++) PRINTK(" %08x", bstack[i]);
    TRACE(CRLF CORDEF);
    x_free(bstack);
#endif//CONFIG_MODULE_BTRACE
}

void vPortAsserted(const char *file, int line, const char *func, const char *expr)
{
    LOGE("ASSERED(%s:%d:%s): %s", file, line, func, expr);
    __EBREAK();
}

#if CONFIG_MODULE_FREERTOS
#if configUSE_IDLE_HOOK
void vApplicationIdleHook(void)
{
}
#endif//configUSE_IDLE_HOOK

#if configUSE_MALLOC_FAILED_HOOK
void vApplicationMallocFailedHook(void)
{
    ASSERT(false, "MallocFailed@%s", x_task_name(NULL));
}
#endif//configUSE_MALLOC_FAILED_HOOK

#if configCHECK_FOR_STACK_OVERFLOW
void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
    ASSERT(false, "StackOverflow@%s", name);
}
#endif//configCHECK_FOR_STACK_OVERFLOW

#if configUSE_HEAP_MALLOC_FREE_HOOK
typedef struct mrec_node {
    struct mrec_node *next;
    TaskHandle_t task;
    const void *addr;
    size_t size;
} mrec_node_t;
typedef struct { mrec_node_t *head; struct { bool alloc, free; }; } mrec_inst_t;
static mrec_inst_t mrec = { .head = NULL, .alloc = false, .free = false };
void vPortMallocHook(size_t size, void *addr)
{
    if (mrec.alloc) return;
    mrec.alloc = true;
    mrec_node_t *node = pvPortMalloc(sizeof(mrec_node_t));
    mrec.alloc = false;
    node->task = xTaskGetCurrentTaskHandle();
    node->addr = addr;
    node->size = size;
    node->next = mrec.head;
    mrec.head = node;
}
void vPortFreeHook(size_t size, void *addr)
{
    if (mrec.free) return;
    mrec_node_t *node = mrec.head, *prev = NULL;
    while (node) {
        if (node->addr == addr) {
            if (prev) prev->next = node->next;
            else mrec.head = node->next;
            mrec.free = true;
            vPortFree(node);
            mrec.free = false;
            break;
        }
        prev = node;
        node = node->next;
    }
}
#endif//configUSE_HEAP_MALLOC_FREE_HOOK

void shell_heap_summary(void)
{
    SHELL_ITEM_EXPORT("heap", shell_heap_summary, "show heap summary");
	TRACE(CRLF COR_FG_YELLOW "Heap Summary:" CORDEF);
	TRACE("%10s %10s %10s %10s", "[---total]", "[used.now]", "[free.now]", "[free.min]");
	TRACE("%10d %10d %10d %10d", xPortGetTotalHeapSize()
        , xPortGetTotalHeapSize() - xPortGetFreeHeapSize()
        , xPortGetFreeHeapSize(), xPortGetMinimumEverFreeHeapSize());
#if configUSE_HEAP_MALLOC_FREE_HOOK
    const mrec_node_t *node = mrec.head;
	TRACE("Heap Detail:");
    TRACE("%20s  %10s  %6s", "[owner]", "[addr]", "[size]");
    vTaskSuspendAll();
    while (node) {
        const char *task = node->task ? pcTaskGetName(node->task) : "-";
        TRACE("%9s@%p  %p  %6d", task, node->task, node->addr, node->size);
        node = node->next;
    }
    xTaskResumeAll();
#endif
}
void shell_task_summary(void)
{
    SHELL_ITEM_EXPORT("task", shell_task_summary, "show task summary");
    uint32_t tasks = uxTaskGetNumberOfTasks();
	TaskStatus_t *const items = x_malloc(tasks * sizeof(TaskStatus_t), sizeof(uint32_t));
	if (items) {
		uint32_t total = 0;
		tasks = uxTaskGetSystemState(items, tasks, &total);
		if (total > 0) {
			TRACE(CRLF COR_FG_YELLOW "Tasks Summary:" CORDEF);
			TRACE("%20s %9s %9s %9s %12s %9s"
                , "[TASK]", "[STA.]", "[PRIO.]", "[STK/B]", "[CALL.]", "[PCT.%]");
			for (TaskStatus_t *stat = items; stat < &items[tasks]; stat++) {
				TRACE("%9s@%p %9s %9d %9d %12d %8d%%", stat->pcTaskName, stat->xHandle
					, ((const char *[]){"RUN", "RDY", "BLK", "SUS", "DEL" })[stat->eCurrentState]
					, (int)stat->uxCurrentPriority, (int)stat->usStackHighWaterMark * sizeof(StackType_t)
                    , (int)stat->ulRunTimeCounter, (int)(100.0f * stat->ulRunTimeCounter / total));
			}
		}
		x_free(items);
	}
}

void shell_reboot(void)
{
	SHELL_ITEM_EXPORT("reboot", shell_reboot, "system reboot");
	TRACE("reboot by user");
    XDGINT();
    __HAL_PMU_SW_RST_AON_RST_ENABLE();
    while (1);
}

#if CONFIG_AADC_ENABLE
void shell_adc_config(int again, int dgain)
{
    SHELL_ITEM_EXPORT("adconf", shell_adc_config, "adconf <AGAIN> <DGAIN>");
    mod_inst_t const aadc = __MAPI_FIND(CONFIG_AADC_NAME);
    ASSERT(aadc, "aadc");
    int gain[2] = {again, dgain};
    __mapi_ctrl(aadc, MAPI_AADC_CTRL_SET_GAIN, &gain[0]);      // set gain
}

void shell_clsd_config(int en)
{
    SHELL_ITEM_EXPORT("classd", shell_clsd_config, "classd <val>, 0:close 1:open");
    mod_inst_t const clsd = __MAPI_FIND(CONFIG_CLSD_NAME);
    ASSERT(clsd, "clsd");
    __mapi_ctrl(clsd, en ? MAPI_CLSD_CTRL_PLAY_START : MAPI_CLSD_CTRL_PLAY_STOP, NULL);
}
#endif

void shell_reg_write(uint32_t reg, uint32_t val)
{
    SHELL_ITEM_EXPORT("reg.set", shell_reg_write, "reg.set <reg> <val>");
    TRACE(CRLF "SET: REG(0x%08x)=0x%08x", (unsigned)reg, (unsigned)val);
    *((volatile uint32_t *)reg) = val;
}

uint32_t shell_reg_read(uint32_t reg)
{
    SHELL_ITEM_EXPORT("reg.get", shell_reg_read, "reg.get <reg>");
    uint32_t val = *((volatile uint32_t *)reg);
    TRACE(CRLF "GET: REG(0x%08x)=0x%08x", (unsigned)reg, (unsigned)val);
    return val;
}

void shell_sys_halt(void)
{
    SHELL_ITEM_EXPORT("halt", shell_sys_halt, "halt");
    XBREAK();
}

void shell_sys_call(void)
{
    SHELL_ITEM_EXPORT("call", shell_sys_call, "call");
    XECALL();
}

#endif//CONFIG_MODULE_FREERTOS
