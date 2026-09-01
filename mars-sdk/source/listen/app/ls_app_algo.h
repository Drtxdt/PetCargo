#ifndef ____APP_ALGO_PROC_H__
#define ____APP_ALGO_PROC_H__

#include "stdint.h"

#define LS_CLIENT_TIMEOUT ("LOCAL_ASR_TIMEOUT")

/**
 * @brief           从算法的Param1中解析出keyword
 * @param in 	    算法JSON串
 * @param out	    解析后的字符串
 * @param max_len	解析后字符串最大长度
 * @return  0       0:解析成功 -1:解析失败
 */
int ls_app_algo_keyword_extract(const uint8_t *const in, uint8_t *out, int max_len);

int ls_app_algo_keyword_format(const uint8_t *const in_keyword, uint8_t *out_keyword, int out_max_len);

#endif  //____APP_ALGO_PROC_H__