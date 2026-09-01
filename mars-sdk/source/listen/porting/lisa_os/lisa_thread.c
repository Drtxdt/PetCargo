#include <lisa_mem.h>
#include <lisa_log.h>
#include <lisa_thread.h>
#include <task.h>
#include <string.h>

#define TAG "lisa_thread"

/**
 * @brief 线程创建
 *
 * @param attr
 * @param entry
 * @param arg
 * @return lisa_thread_t*
 */
lisa_thread_t *lisa_thread_create(const lisa_thread_attr_t *attr, void (*entry)(void *), void *arg)
{
	if (attr == NULL || attr->stack_size <= 0) {
		LISA_LOGE(TAG, "lisa thread create fail, param error");
		return NULL;
	}

	lisa_thread_t *thread = lisa_mem_alloc(sizeof(lisa_thread_t));
	if (thread) {
		xTaskCreate(
				entry,
				(const char *const)(attr->name),
				attr->stack_size,
				arg,
				attr->priority,
				(TaskHandle_t * const)&thread->handle);
		return thread;
	}
	return NULL;
}

/**
 * @brief 设置线程优先级
 * 
 * @param thread 
 * @param priority 新的优先级
 * @return lisa_err_t 
 */
lisa_err_t lisa_thread_set_priority(lisa_thread_t *thread, uint8_t priority)
{
	lisa_err_t result = LISA_FAIL;
	if (thread != NULL) {
		uint8_t pre_priority = uxTaskPriorityGet(thread->handle);
		if (pre_priority != priority) {
			vTaskPrioritySet(thread->handle, priority);
			LISA_LOGD(TAG, "set priority:%d", priority);
		}
		result = LISA_OK;
	}
	return result;
}

/**
 * @brief 线程销毁
 *
 * @param thread
 * @return lisa_err_t
 */
lisa_err_t lisa_thread_delete(lisa_thread_t *thread)
{
	if (thread == NULL) {
		vTaskDelete(NULL); 
		return LISA_OK;
	}

	if (thread->handle) {
		vTaskDelete(thread->handle);
	} else {
		vTaskDelete(NULL);
	}
	return LISA_OK;
}
/**
 * @brief 线程休眠 单位：秒
 *
 * @param seconds
 * @return lisa_err_t
 */
lisa_err_t lisa_thread_delay(uint32_t seconds)
{
	lisa_thread_mdelay(seconds * 1000);
	return 0;
}
/**
 * @brief 线程休眠 单位：毫秒
 *
 * @param ms
 * @return lisa_err_t
 */
lisa_err_t lisa_thread_mdelay(uint32_t ms)
{
	vTaskDelay((TickType_t)pdMS_TO_TICKS(ms));
	return 0;
}

lisa_err_t lisa_thread_yield(void)
{
	taskYIELD();
	return LISA_OK;
}

char * lisa_thread_cur_thread_name(void)
{
	TaskHandle_t task = xTaskGetCurrentTaskHandle();
	return pcTaskGetName(task);
}