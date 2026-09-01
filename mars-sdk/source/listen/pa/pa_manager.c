#include "appinc.h"
#include "lisa_log.h"
#include "lisa_mem.h"
#include "evs_utils.h"
#include "lisa_mutex.h"
#include "pa_manager.h"
#include "lisa_timer.h"
#include "Driver_GPIO.h"
#include "lisa_typedef.h"
#include "lisa_semaphore.h"
#include "ls_app_cfg_mgr.h"

#define TAG "PA_MGR"

typedef struct pa_manager_s {
	PA_MGR_STATE 	m_state;
	uint32_t 		m_timer_period;
	lisa_timer_t 	*m_change_timer;
	lisa_mutex_t 	*m_lock;
} pa_manager_t;

static pa_manager_t *s_pa_hdl = NULL;

#define PA_MGR_SWITCH_CP_COUNT (3)

static void *PA_DRV_HANDLE = NULL;

#if (LS_PA_ON_REALLY_TIME > 0)
#define WAIT_PA_REALLY_ON_TIMEOUT		(LS_PA_ON_REALLY_TIME + 100)
static lisa_timer_t *s_pa_on_timer		= NULL;
static lisa_semaphore_t *s_pa_on_sem	= NULL;
static bool s_pa_on_really				= false;
#endif

void pa_manager_pre_init()
{
	pa_config_item_t pa_config_item = cfg_parser_get_pa_config();
	uint32_t pa_out_off_val = (pa_config_item.enable_level == 1) ? 0 : 1;
	LISA_LOGI(TAG, "pa manager pre init PA");
	PA_DRV_HANDLE = (pa_config_item.ctl_io_pad == CSK_IOMUX_PAD_B) ? GPIOB() : GPIOA();
	GPIO_Initialize(PA_DRV_HANDLE, NULL, NULL);
	IOMuxManager_PinConfigure(pa_config_item.ctl_io_pad, pa_config_item.ctl_io_num, PA_CONTROL_IO_FUNC);
	GPIO_SetDir(PA_DRV_HANDLE, (1 << pa_config_item.ctl_io_num), CSK_GPIO_DIR_OUTPUT);
	GPIO_PinWrite(PA_DRV_HANDLE, (1 << pa_config_item.ctl_io_num), pa_out_off_val);
}

int pa_manager_onoff(int onoff)
{
	if (PA_DRV_HANDLE != NULL) {
		pa_config_item_t pa_config_item = cfg_parser_get_pa_config();
		uint32_t pa_out_on_val = pa_config_item.enable_level;
		uint32_t pa_out_off_val = (pa_out_on_val == 1) ? 0 : 1;
		if (onoff) {
			LISA_LOGI(TAG, "PA ON");
			GPIO_PinWrite(PA_DRV_HANDLE, (1 << pa_config_item.ctl_io_num), pa_out_on_val);
		} else {
			LISA_LOGI(TAG, "PA OFF");
			GPIO_PinWrite(PA_DRV_HANDLE, (1 << pa_config_item.ctl_io_num), pa_out_off_val);
		}
	}
	return 0;
}

/**
 * @brief  Switch PA State
 * @param  next_state		Next PA State
 */
static void __pa_switch_state(PA_MGR_STATE next_state)
{
	if (s_pa_hdl) {
		lisa_mutex_lock(s_pa_hdl->m_lock, LISA_OS_WAIT_FOREVER);

		switch (next_state) {
			case PA_MGR_OFF: {
				if (s_pa_hdl->m_state != PA_MGR_OFF) {
					// pa off
					if (pa_manager_onoff(0) == 0) {
						// set state
						s_pa_hdl->m_state = PA_MGR_OFF;
						#if (LS_PA_ON_REALLY_TIME > 0)
							LISA_LOGD(TAG, "Stop pa on timer and clear sem.");
							s_pa_on_really = false;
							lisa_timer_stop(s_pa_on_timer);
							lisa_semaphore_take(s_pa_on_sem, 0);
						#endif
					}
				}
			} break;
			case PA_MGR_ON: {
				if (s_pa_hdl->m_state != PA_MGR_ON) {
					// pa on
					if (pa_manager_onoff(1) == 0) {
						// set state
						s_pa_hdl->m_state = PA_MGR_ON;
						#if (LS_PA_ON_REALLY_TIME > 0)
							bool timer_isactive = lisa_timer_isactive(s_pa_on_timer);
							LISA_LOGD(TAG, "pa on timer is active %s.", timer_isactive ? "yes":"no and start");
							if (!timer_isactive) lisa_timer_start(s_pa_on_timer);
						#endif
					}
				}
			} break;
			case PA_MGR_NONE: {
				s_pa_hdl->m_state = PA_MGR_NONE;
			};
		}

		lisa_mutex_unlock(s_pa_hdl->m_lock);
	}
}

static int __handle_pa_off_runnable(void *arg)
{
	LISA_LOGI(TAG, "PA off, by timeout");
	__pa_switch_state(PA_MGR_OFF);
	return 0;
}

/**
 * @brief 	PA OFF Timer Callback
 * @param  	arg		PA Manager Handle
 */
static void __pa_off_timeout(void *arg)
{
	evs_handler_post_runnable(__handle_pa_off_runnable, arg);
}

#if (LS_PA_ON_REALLY_TIME > 0)
static void pa_on_really(void *arg)
{
	LISA_LOGD(TAG, "pa really on and notify!");
	s_pa_on_really = true;
	lisa_semaphore_give(s_pa_on_sem);
}
#endif

void pa_manager_init(PA_MGR_STATE init_state)
{
	pa_config_item_t pa_config_item = cfg_parser_get_pa_config();
	s_pa_hdl = (pa_manager_t *)lisa_mem_alloc(sizeof(pa_manager_t));
	ASSERT(s_pa_hdl, "pa handle null");

	s_pa_hdl->m_change_timer = lisa_timer_create(pa_config_item.hold_time, __pa_off_timeout, s_pa_hdl);
	ASSERT(s_pa_hdl->m_change_timer, "pa change timer null");

#if (LS_PA_ON_REALLY_TIME > 0)
	s_pa_on_timer = lisa_timer_create(LS_PA_ON_REALLY_TIME, pa_on_really, NULL);
	ASSERT(s_pa_on_timer, "pa on really timer null");

	s_pa_on_sem = lisa_semaphore_create(1);
	ASSERT(s_pa_on_sem, "pa on sem null");
#endif

	s_pa_hdl->m_lock = lisa_mutex_create();
	ASSERT(s_pa_hdl->m_lock, "pa lock null");

	s_pa_hdl->m_timer_period = pa_config_item.hold_time;

	s_pa_hdl->m_state = PA_MGR_NONE;
	pa_manager_refresh(init_state, pa_config_item.hold_time, "pa_mgr_init");
}

void pa_manager_refresh(PA_MGR_STATE next_state, uint32_t duration, const char *const by_which)
{
	if (!s_pa_hdl) return;

	LISA_LOGD(TAG, "Refresh PA to %s, timeout %u, by \"%s\"",
						PA_PRINT_STATE(next_state), duration, by_which);
	if (next_state == PA_MGR_ON) {
		__pa_switch_state(next_state);
	} else if (next_state == PA_MGR_OFF) {
		// delay change PA state to off
		if (duration == 0) {
			__pa_switch_state(next_state);
			// stop change timer
			lisa_timer_stop(s_pa_hdl->m_change_timer);
		}
	}

	if (duration == LS_PA_FOREVER) {
		lisa_timer_stop(s_pa_hdl->m_change_timer);
	} else {
		// launch change to off timer
		if (duration != s_pa_hdl->m_timer_period) {
			lisa_timer_change_period(s_pa_hdl->m_change_timer, duration);
			s_pa_hdl->m_timer_period = duration;
			// Restart Timer
			lisa_timer_reset(s_pa_hdl->m_change_timer);
		} else {
			lisa_timer_stop(s_pa_hdl->m_change_timer);
			lisa_timer_start(s_pa_hdl->m_change_timer);
		}
	}
	LISA_LOGD(TAG, "Refresh PA end by \"%s\"", by_which);
}

void pa_manager_refresh_on_sync(uint32_t duration, const char *const by_which)
{
	if (!s_pa_hdl) return;

	pa_manager_refresh(PA_MGR_ON, duration, by_which);
#if (LS_PA_ON_REALLY_TIME > 0)
	LISA_LOGD(TAG, "%s wait pa really on.", s_pa_on_really ? "Needn't":"Need");
	if (!s_pa_on_really) {
		lisa_err_t ret = lisa_semaphore_take(s_pa_on_sem, WAIT_PA_REALLY_ON_TIMEOUT);
		LISA_LOGD(TAG, "Wait pa really on %s.", (ret == LISA_OK) ? "successed":"failed");
	}
#endif
}

void pa_manager_reset_state(const char *const by_which)
{
	if (!s_pa_hdl) return;

	LISA_LOGD(TAG, "Reset PA to %s, by \"%s\"", PA_PRINT_STATE(PA_MGR_NONE), by_which);
	__pa_switch_state(PA_MGR_NONE);
}

PA_MGR_STATE pa_manager_get_state()
{
	if (s_pa_hdl) {
		return s_pa_hdl->m_state;
	}
	return PA_MGR_NONE;
}