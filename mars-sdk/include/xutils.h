#ifndef __XFER_UTILS_HEADER__
#define __XFER_UTILS_HEADER__

#include "sdkinc.h"

#define __packed__          __attribute__((packed))
#define __naked__           __attribute__((naked))
#define __depre__           __attribute__((deprecated))
#define __setup__           __attribute__((constructor))
#define __noret__           __attribute__((noreturn))
#define __noinline__        __attribute__((noinline))
#define __const__           __attribute__((const))
#define __used__            __attribute__((used))
#define __unused__          __attribute__((unused))
#define __weak__            __attribute__((weak))
#define __align(N)          __attribute__((aligned(N)))
#define __at(addr)          __attribute__((at(addr)))
#define __format(fp,vp)     __attribute__((format(printf, fp, vp)))
#define __hide__            __attribute__((visibility("hidden")))
#define __fast__            __attribute__((section(".fast_text")))
#define __strs__            __attribute__((section(".strdat")))
#define __stub__            __attribute__((section(".stub"), used))
#define __sect(sct)         __attribute__((section(sct)))
#define __optimize(OP)      __attribute__((optimize(OP)))
#define __wrap(F)           __wrap_##F
#define __real(F)           __real_##F
#define static_inline       static inline
#define static_noinline     static __noinline__
#define __maxof(A,B)        ({typeof(A) _a=(A),_b=(B);_a>_b?_a:_b;})
#define __minof(A,B)        ({typeof(A) _a=(A),_b=(B);_a<_b?_a:_b;})
#define __cntof(A)          (sizeof(A)/sizeof(A[0]))
#define __offsof(T,M)       ((size_t)&((T*)0)->M)
#define __contof(P,T,M)     ((T*)(((char*)(P))-__offsof(T,M)))
#define XINFINITE           (portMAX_DELAY)
#define XMS2TK(MS)			((MS)==XINFINITE?portMAX_DELAY:((x_msec_t)(MS)/portTICK_PERIOD_MS))
#define XTK2MS(TK)          ((int)(TK)*portTICK_PERIOD_MS)
#define XEGINT()            __RV_CSR_SET(CSR_MSTATUS,MSTATUS_MIE)
#define XDGINT()            __RV_CSR_CLEAR(CSR_MSTATUS,MSTATUS_MIE)
#define XLOP()              __NOP()
#define XWFI()              __WFI()
#define XWFE()              __WFE()
#define XECALL()            __ECALL()
#define XBREAK()            __EBREAK()
#define XISB()              __FENCE_I()
#define XDMB()              __RWMB()
#define XBIT(NB)			(1<<(NB))
#define XBITS(HB, LB)	    ((2<<(HB))-(1<<(LB)))
#define XCONCAT(A,B)        A##B
#define XVANAME(A,B)        XCONCAT(A,B)
#define __XARGS(_X)         (_X)
#define __XARGN(_0,_1,_2,_3,_4,_5,_6,_7,N,...) (N==1?!!#_0[0]:N)
#define __XARGC(...)        __XARGS(__XARGN(__VA_ARGS__,8,7,6,5,4,3,2,1))
#define XARGC(...)          __XARGC(__VA_ARGS__)

typedef void *x_handle_t;
typedef x_handle_t x_queue_t, x_mutex_t, x_sema_t, x_task_t, x_event_t, x_stmbuf_t, x_list_t, x_item_t;
typedef const char *const x_name_t;
typedef unsigned int x_evtbit_t;
typedef TickType_t x_msec_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// memory apis
typedef struct { uint32_t size, pofs; } x_memext_t;
static_inline x_handle_t x_malloc(size_t size, size_t align)
{
    if (!size) return NULL;
    if (align < sizeof(x_handle_t)) align = sizeof(x_handle_t);
    ASSERT(!(align & (align - 1)), "align(%d) !2^n", align);
    x_handle_t *const mem = pvPortMalloc(sizeof(x_memext_t) + --align + size);
    x_handle_t *const ptr = (x_handle_t *)(((size_t)mem + sizeof(x_memext_t) + align) & ~align);
    x_memext_t *const ext = (x_memext_t *)ptr - 1;
    ext->pofs = (size_t)ptr - (size_t)mem;
    ext->size = size;
    return (x_handle_t)ptr;
}
static_inline void x_free(x_handle_t ptr)
{
    ASSERT(ptr, "xfree");
    x_memext_t *const ext = (x_memext_t *)ptr - 1;
    vPortFree((uint8_t *)ptr - ext->pofs);
}
static_inline size_t x_msize(x_handle_t ptr)
{
    ASSERT(ptr, "xsize");
    x_memext_t *const ext = (x_memext_t *)ptr - 1;
    return ext->size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// task apis
typedef void (* x_task_func_t)(void *param);
static_inline x_task_t x_task_new(x_name_t name, x_task_func_t func, size_t stack, int prio, void *param)
{
    TaskHandle_t inst = NULL;
    xTaskCreate(func, name, stack, param, prio, &inst);
    return inst;
}
static_inline void x_task_del(x_task_t inst)
{
    vTaskDelete(inst);
}
static_inline void x_task_yield(void)
{
    portYIELD();
}
static_inline void x_task_sleep(x_msec_t msec)
{
    ASSERT(!xPortIsInsideInterrupt(), "isr context");
    vTaskDelay(XMS2TK(msec));
    // if (xPortIsInsideInterrupt()) x_task_yield();
    // else vTaskDelay(XMS2TK(msec));
}
static_inline uint32_t x_task_take(bool clr, x_msec_t msec)
{
    return ulTaskNotifyTake(clr, XMS2TK(msec));
}
static_inline void x_task_give(x_task_t inst)
{
    if (!xPortIsInsideInterrupt()) xTaskNotifyGive(inst);
    else {
        BaseType_t yield;
        vTaskNotifyGiveFromISR(inst, &yield);
        portYIELD_FROM_ISR(yield);
    }
}
static_inline bool x_task_wait(uint32_t clr0, uint32_t clr1, uint32_t *value, x_msec_t msec)
{
    return pdPASS == xTaskNotifyWait(clr0, clr1, value, XMS2TK(msec));
}
static_inline bool x_task_notify(x_task_t inst, uint32_t value)
{
    if (!xPortIsInsideInterrupt()) return pdPASS == xTaskNotify(inst, value, eSetBits);
    BaseType_t yield;
    BaseType_t xret = xTaskNotifyFromISR(inst, value, eSetBits, &yield);
    portYIELD_FROM_ISR(yield);
    return xret;
}
static_inline int x_task_prio(x_task_t inst, int prio)
{
    int oprio = uxTaskPriorityGet(inst);
    vTaskPrioritySet(inst, prio);
    return oprio;
}
static_inline x_task_t x_task_inst(void)
{
    return (x_task_t)xTaskGetCurrentTaskHandle();
}
static_inline const char *x_task_name(x_task_t inst)
{
    return pcTaskGetName(inst);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// queue apis
static_inline x_queue_t x_queue_new(x_name_t name, int count, size_t size)
{
    (void)name;
    return xQueueCreate(count, size);
}
static_inline bool x_queue_send(x_queue_t inst, void *item, bool front, x_msec_t msec)
{
    BaseType_t cploc = front ? queueSEND_TO_FRONT : queueSEND_TO_BACK;
    if (!xPortIsInsideInterrupt()) return pdTRUE == xQueueGenericSend(inst, item, XMS2TK(msec), cploc);
    BaseType_t yield = pdFALSE;
    BaseType_t ret = xQueueGenericSendFromISR(inst, item, &yield, cploc);
    portYIELD_FROM_ISR(yield);
    return pdTRUE == ret;
}
static_inline bool x_queue_recv(x_queue_t inst, void *item, x_msec_t msec)
{
    if (!xPortIsInsideInterrupt()) return pdTRUE == xQueueReceive(inst, item, XMS2TK(msec));
    BaseType_t yield = pdFALSE;
    BaseType_t ret = xQueueReceiveFromISR(inst, item, &yield);
    portYIELD_FROM_ISR(yield);
    return pdTRUE == ret;
}
static_inline bool x_queue_peek(x_queue_t inst, void *item, x_msec_t msec)
{
    if (!xPortIsInsideInterrupt()) return pdTRUE == xQueuePeek(inst, item, XMS2TK(msec));
    return pdTRUE == xQueuePeekFromISR(inst, item);
}
static_inline bool x_queue_drop(x_queue_t inst, void *item, x_msec_t msec)
{
    return x_queue_recv(inst, item, msec);
}
static_inline int x_queue_items(x_queue_t inst)
{
    if (!xPortIsInsideInterrupt()) return uxQueueMessagesWaiting(inst);
    return uxQueueMessagesWaitingFromISR(inst);
}
static_inline int x_queue_spaces(x_queue_t inst)
{
    if (!xPortIsInsideInterrupt()) return uxQueueSpacesAvailable(inst);
    return uxQueueSpacesAvailableFromISR(inst);
}
static_inline void x_queue_del(x_queue_t inst)
{
    vQueueDelete(inst);
}
static_inline void x_queue_rst(x_queue_t inst)
{
    xQueueReset(inst);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// semaphore
static_inline x_sema_t x_sema_new(x_name_t name)
{
    (void)name;
    return xSemaphoreCreateBinary();
}
static_inline void x_sema_del(x_sema_t inst)
{
    vSemaphoreDelete(inst);
}
static_inline bool x_sema_give(x_sema_t inst)
{
    if (!xPortIsInsideInterrupt()) return pdTRUE == xSemaphoreGive(inst);
    BaseType_t yield = pdFALSE;
    BaseType_t ret = xSemaphoreGiveFromISR(inst, &yield);
    portYIELD_FROM_ISR(yield);
    return pdTRUE == ret;
}
static_inline bool x_sema_take(x_sema_t inst, x_msec_t msec)
{
    if (!xPortIsInsideInterrupt()) return pdTRUE == xSemaphoreTake(inst, XMS2TK(msec));
    BaseType_t yield = pdFALSE;
    BaseType_t ret = xSemaphoreTakeFromISR(inst, &yield);
    portYIELD_FROM_ISR(yield);
    return pdTRUE == ret;   
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// semaphore and mutex
static_inline x_mutex_t x_mutex_new(x_name_t name)
{
    (void)name;
    return xSemaphoreCreateMutex();
}
static_inline bool x_mutex_lock(x_mutex_t inst, x_msec_t msec)
{
    if (xPortIsInsideInterrupt()) return false;
    return pdTRUE == xSemaphoreTake(inst, XMS2TK(msec));
}
static_inline bool x_mutex_unlock(x_mutex_t inst)
{
    if (xPortIsInsideInterrupt()) return false;
    return pdTRUE == xSemaphoreGive(inst);
}
static_inline void x_mutex_del(x_mutex_t inst)
{
    vSemaphoreDelete(inst);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// event apis
#define XEVT_AUTO_CLR		XBIT(30)
#define XEVT_WAIT_ALL		XBIT(31)
#define XEVT_BITS_MSK		XBITS(23, 0)
static_inline x_event_t x_event_new(x_name_t name)
{
    (void)name;
    return xEventGroupCreate();
}
static_inline void x_event_del(x_event_t inst)
{
    vEventGroupDelete(inst);
}
static_inline x_evtbit_t x_event_wait(x_event_t inst, x_evtbit_t emsk, x_msec_t msec)
{
    return xEventGroupWaitBits(inst, emsk & XEVT_BITS_MSK, emsk & XEVT_AUTO_CLR
        , emsk & XEVT_WAIT_ALL, XMS2TK(msec)) & (emsk & XEVT_BITS_MSK);
}
static_inline bool x_event_set(x_event_t inst, x_evtbit_t emsk)
{
    if (xPortIsInsideInterrupt()) {
        BaseType_t yield = pdFALSE;
        BaseType_t ret = xEventGroupSetBitsFromISR(inst, emsk & XEVT_BITS_MSK, &yield);
        portYIELD_FROM_ISR(yield);
        return ret;
    }
    xEventGroupSetBits(inst, emsk & XEVT_BITS_MSK);
    return true;
}
static_inline bool x_event_clr(x_event_t inst, x_evtbit_t emsk)
{
    if (xPortIsInsideInterrupt()) return pdPASS == xEventGroupClearBitsFromISR(inst, emsk & XEVT_BITS_MSK);
    xEventGroupClearBits(inst, emsk & XEVT_BITS_MSK);
    return true;
}
static_inline x_evtbit_t x_event_get(x_event_t inst, x_evtbit_t emsk)
{
    if (xPortIsInsideInterrupt()) return xEventGroupGetBitsFromISR(inst) & (XEVT_BITS_MSK & emsk); 
    return xEventGroupGetBits(inst) & (emsk & XEVT_BITS_MSK);
}
static_inline x_evtbit_t x_event_sync(x_event_t inst, x_evtbit_t smsk, x_evtbit_t wmsk, x_msec_t msec)
{
    ASSERT(!xPortIsInsideInterrupt(), "isr context not allowed");
    // return xEventGroupSync(inst, smsk, wmsk, XMS2TK(msec));
    return x_event_set(inst, smsk), x_event_wait(inst, wmsk, msec);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// stream buffer apis
static_inline x_stmbuf_t x_stmbuf_new(x_name_t name, size_t size, size_t trig)
{
    return xStreamBufferCreate(size, trig);
}
static_inline void x_stmbuf_del(x_stmbuf_t inst)
{
    vStreamBufferDelete(inst);
}
static_inline void x_stmbuf_rst(x_stmbuf_t inst)
{
    xStreamBufferReset(inst);
}
static_inline size_t x_stmbuf_send(x_stmbuf_t inst, const void *data, size_t size, x_msec_t msec)
{
    if (!xPortIsInsideInterrupt()) return xStreamBufferSend(inst, data, size, XMS2TK(msec));
    BaseType_t yield = pdFALSE;
    BaseType_t ret = xStreamBufferSendFromISR(inst, data, size, &yield);
    portYIELD_FROM_ISR(yield);
    return ret;
}
static_inline size_t x_stmbuf_recv(x_stmbuf_t inst, void *data, size_t size, x_msec_t msec)
{
    size_t recv = 0;
    if (xPortIsInsideInterrupt()) {
        BaseType_t yield = pdFALSE;
        recv = xStreamBufferReceiveFromISR(inst, data, size, &yield);
        portYIELD_FROM_ISR(yield);
    } else {
    #if 0
        recv = xStreamBufferReceive(inst, data, size, XMS2TK(msec));
    #else
        int wait = XMS2TK(msec);
        int tick = (int)xTaskGetTickCount();
        do {
            recv += xStreamBufferReceive(inst, (uint8_t *)data + recv, size - recv, wait);
            wait -= (xTaskGetTickCount() - tick);
        } while (recv < size && wait > 0);
    #endif
    }
    return recv;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// misc rtos apis
static_inline int x_os_tick(void)
{
    return xPortIsInsideInterrupt() ? xTaskGetTickCountFromISR() : xTaskGetTickCount();
}
static_inline void x_os_suspend(void)
{
    vTaskSuspendAll();
}
static_inline void x_os_resume(void)
{
    xTaskResumeAll();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static_inline void x_icache_flush(void)
{
    HAL_InvalidateICache();
    extern uint32_t __vmaof_cache, __lenof_cache;
    if ((size_t)&__lenof_cache) {
        HAL_LockICache_by_Addr(&__vmaof_cache, (uint32_t)&__lenof_cache);
        TRACE("ICLOCK=%d", (int)&__lenof_cache);
    }
}

void dmacopy(void *dst, const void *src, size_t size, x_msec_t msec);
void dmawait(x_msec_t msec);

#endif//__XFER_UTILS_HEADER__
