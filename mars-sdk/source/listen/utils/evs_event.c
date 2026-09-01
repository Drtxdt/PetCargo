#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "appinc.h"
#include "lisa_log.h"
#include "lisa_err.h"
#include "lisa_mem.h"
#include "evs_event.h"
#include "lisa_time.h"
#include "lisa_queue.h"
#include "lisa_thread.h"
#include "lisa_typedef.h"

/** Event TAG */
#define TAG "event"
/** Event Queue Max Count */
#define EVENT_QUEUE_COUNT_MAX (5)
/** Event Thread Stack Size */
#define EVENT_THREAD_STACK_SIZE (288)
/** Event Thread Name */
#define EVENT_THREAD_NAME ("evs_event")
/** Event Queue Wait Time */
#define EVENT_LOOP_TIME (600)
/** Event Wtd Enable*/
#define EVENT_WTD_ENABLE (1)

struct evs_event_s {
	lisa_queue_t *queue;
	lisa_thread_t *thread;
#if EVENT_WTD_ENABLE
	mod_inst_t wtd_mdrv;
	uint32_t algo_tick;
#endif
};

extern uint32_t get_task_algo_tick();

static void _evs_event_run_task(void *param)
{
	evs_event_t *s_event = (evs_event_t *)param;
	evs_event_message_t msg;
	lisa_err_t pop_ret;
	while (true) {
		pop_ret = lisa_queue_pop(s_event->queue, &msg, sizeof(evs_event_message_t), EVENT_LOOP_TIME);
	#if EVENT_WTD_ENABLE
		if (get_task_algo_tick() != s_event->algo_tick) {
			__mapi_ctrl(s_event->wtd_mdrv, 2, NULL);
			s_event->algo_tick = get_task_algo_tick();
		} else {
			LISA_LOGE(TAG, "algo tick same\n");
		}
	#endif
		if (pop_ret != LISA_OK) continue;

		if (msg.runnable != NULL) {
			LISA_LOGD(TAG, "evt msg -> enter");
			msg.runnable(msg.user_data);
			LISA_LOGD(TAG, "evt msg -> exit");
		}
	}
}

evs_event_t *evs_event_create()
{
	evs_event_t *s_event = (evs_event_t *)lisa_mem_calloc(1, sizeof(evs_event_t));
	if (s_event == NULL) {
		LISA_LOGE(TAG, "malloc event failed!");
		return NULL;
	}
	#if EVENT_WTD_ENABLE
	s_event->algo_tick = UINT32_MAX;
	#endif
	s_event->queue = lisa_queue_create(EVENT_QUEUE_COUNT_MAX, (uint8_t *)"event", sizeof(evs_event_message_t));
	if (s_event->queue == NULL) {
		goto err_queue;
	}

#if EVENT_WTD_ENABLE
	s_event->wtd_mdrv = __MAPI_FIND(CONFIG_XWDT_NAME);
	__mapi_ctrl(s_event->wtd_mdrv, 1, NULL);
#endif

	lisa_thread_attr_t thread_attr;
	thread_attr.name = (uint8_t *)EVENT_THREAD_NAME;
	thread_attr.stack_size = EVENT_THREAD_STACK_SIZE;
	thread_attr.priority = LISA_OS_PRIORITY_NORMAL;
	s_event->thread = lisa_thread_create(&thread_attr, _evs_event_run_task, (void *)s_event);
	if (s_event->thread == NULL) {
		LISA_LOGE(TAG, "create thread error");
		goto err;
	}

	return s_event;
err:
	if (s_event->queue != NULL) {
		lisa_queue_delete(s_event->queue);
	}
err_queue:
	lisa_mem_free(s_event);
	return NULL;
}

int evs_event_send_msg_delay(evs_event_t *s_event, evs_event_message_t *msg, long delay)
{
	if (lisa_queue_push(s_event->queue, msg, sizeof(evs_event_message_t), 0) != LISA_OK) {
		LISA_LOGE(TAG, "send empty msg error");
		return -1;
	}
	return 0;
}

int evs_event_post_runnable(evs_event_t *s_event, evs_event_runnable runnable, void *user_data)
{
	evs_event_message_t msg = {runnable, user_data};
	return evs_event_send_msg_delay(s_event, &msg, 0);
}


void evs_event_destroy(evs_event_t *s_event)
{
	if (s_event->thread != NULL) {
		lisa_thread_delete(s_event->thread);
	}
	evs_event_message_t msg;
	while (lisa_queue_pop(s_event->queue, &msg, sizeof(evs_event_message_t), 0) == LISA_OK) {
	}
	lisa_mem_free(s_event);
}
