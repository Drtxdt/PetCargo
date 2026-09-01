#include <string.h>
#include <stdbool.h>
#include "lisa_log.h"
#include "lisa_mem.h"
#include "ls_app_algo.h"

// #define WAKEUP_WORD_KEY_START ("\"keyword\":\"")
// #define WAKEUP_WORD_KEY_START_LEN (11)  // "keyword":"
#define WAKEUP_WORD_KEY_START ("\"intentStr\":\"")
#define WAKEUP_WORD_KEY_START_LEN (13)  // "intentStr":"
#define WAKEUP_WORD_KEY_END ("\"")
#define WAKEUP_WORD_KEY_END_LEN (1)  // "

#define TAG "algo"

/** 是否为数字 */
static bool is_digit(char str)
{
	return (str >= '0' && str <= '9');
}

/** 是否为特殊字符 */
static bool is_special(char str)
{
	return (str == '\r');
}

/**
 * 移除字符串中的数字
 * @param input     待处理字符串
 * @return          处理后的字符串
 */
static char *__remove_digits_and_special(char *input)
{
	int dest_len = 0;
	char *dest = lisa_mem_calloc(1, strlen(input) + 1);
	char *src = input;

	while (*src) {
		if (is_digit(*src) || is_special(*src)) {
			src++;
			continue;
		}
		dest[dest_len++] = *src++;
	}
	return dest;
}

int ls_app_algo_keyword_extract(const uint8_t *const in, uint8_t *out, int max_len)
{
	int ret = -1;
	char *kw_s_str = strstr((const char *)in, WAKEUP_WORD_KEY_START);
	if (kw_s_str == NULL) {
		return ret;
	}
	char *kw_e_str = strstr(kw_s_str + WAKEUP_WORD_KEY_START_LEN, WAKEUP_WORD_KEY_END);
	if (kw_e_str == NULL) {
		return ret;
	}
	uint16_t real_keyword_len = kw_e_str - kw_s_str - WAKEUP_WORD_KEY_START_LEN;
	char *real_keyword = lisa_mem_calloc(1, real_keyword_len + 1);
	memcpy(real_keyword, kw_s_str + WAKEUP_WORD_KEY_START_LEN, real_keyword_len);

	char *proc_keyword = __remove_digits_and_special(real_keyword);
	if (proc_keyword) {
		ret = 0;
		const int kw_len = strlen(proc_keyword);
		if (kw_len < max_len) {
			memcpy(out, proc_keyword, kw_len + 1);
		} else {
			strncpy((char *)out, proc_keyword, max_len);
		}
		lisa_mem_free(proc_keyword);
	}
	lisa_mem_free(real_keyword);
	return ret;
}

int ls_app_algo_keyword_format(const uint8_t *const in_keyword, uint8_t *out_keyword, int out_max_len)
{
	int ret = -1;
	char *proc_keyword = __remove_digits_and_special((char *)in_keyword);
	if (proc_keyword) {
		ret = 0;
		const int kw_len = strlen(proc_keyword);
		if (kw_len < out_max_len) {
			memcpy(out_keyword, proc_keyword, kw_len + 1);
		} else {
			strncpy((char *)out_keyword, proc_keyword, out_max_len);
		}
		lisa_mem_free(proc_keyword);
	}
	return ret;
}