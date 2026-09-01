#ifndef __LISTENAI_PA_MANAGER_H__
#define __LISTENAI_PA_MANAGER_H__

#include <stdint.h>

#define PA_CONTROL_IO_PAD  CSK_IOMUX_PAD_B
#define PA_CONTROL_IO_NUM  11   // PA引脚配置
#define PA_CONTROL_IO_FUNC CSK_IOMUX_FUNC_DEFAULT
#define PA_OUT_ON          (1)
#define PA_OUT_OFF         (0)

// PA delay off time
#define LS_PA_BASE_TIME (20 * 1000)
// PA实际打开耗时 (250ms 默认耗时， 0 不耗时)
#define LS_PA_ON_REALLY_TIME (100)

// Forever
#define LS_PA_FOREVER (0xffffffffUL)

typedef enum PA_MGR_STATE {
    // PA OFF
    PA_MGR_OFF = 0,
    // PA ON
    PA_MGR_ON = 1,

    PA_MGR_NONE = 0xFF,
} PA_MGR_STATE;

#define PA_PRINT_STATE(s) \
    (s == PA_MGR_ON)? "ON": \
    (s == PA_MGR_OFF)? "OFF": \
    (s == PA_MGR_NONE)? "NONE":"UNKNOW"

/**
 * @brief  PA GPIO Init
 */
void pa_manager_pre_init();

/**
 * @brief  PA OnOFF
 * @param  onoff    0: OFF, other: ON
 */
int pa_manager_onoff(int onoff);

/**
 * @brief   Init PA Manager
 * @param   init_state  Init State
 */
void pa_manager_init(PA_MGR_STATE init_state);

/**
 * @brief   Refresh PA state
 * @param   next_state  Next state of PA
 * @param   duration    Time(ms) of switching to off after change PA state
 *                      default: LS_PA_BASE_TIME
 *                      
 * @param   by_which    Caller
 */
void pa_manager_refresh(PA_MGR_STATE next_state, uint32_t duration, const char *const by_which);

/**
 * @brief  打开PA同步接口
 */
void pa_manager_refresh_on_sync(uint32_t duration, const char *const by_which);

/**
 * @brief   重置 PA state
 * @param   by_which    Caller
 */
void pa_manager_reset_state(const char *const by_which);

/**
 * @brief   获取 PA state
 */
PA_MGR_STATE pa_manager_get_state();

#endif