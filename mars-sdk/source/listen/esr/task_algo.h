#ifndef __CSK_APPLICATION_ALGO_TASK_HEADER__
#define __CSK_APPLICATION_ALGO_TASK_HEADER__
#include <stdint.h>
#include "w_ivw.h"

typedef enum ls_algo_msg_type {
    e_algo_msg_type_esr_start,    		// 算法开始
    e_algo_msg_type_esr_stop,     		// 算法停止
    e_algo_msg_type_esr_asr_mode,  		// 识别模式
	e_algo_msg_type_esr_reinit,  		// 重新初始化
	e_algo_msg_type_esr_wakeup_mode,  	// 唤醒模式
	e_algo_msg_type_play_start,   		// 通知算法模块开始播音
	e_algo_msg_type_play_stop,    		// 通知算法模块停止播音
	e_algo_msg_type_esr_restart,		// 重启算法
	e_algo_msg_type_esr_init,			// 初始化
	e_algo_msg_type_esr_deinit,			// 逆初始化
	e_algo_msg_type_factory_test,		// 产测模式

	e_algo_msg_type_count,
} ls_algo_msg_type_t;

typedef enum esr_algo_state {
	e_state_fetchinit = -1,
	e_state_idle	  = 0,
	e_state_init,	 // init state
	e_state_reinit,	 // reinit state
	e_state_running, // running state
    e_state_restart, // restart state
    e_state_deinit,  // deinit state
	e_state_count
} esr_algo_state_t;

/**
 * @brief 			设置当前唤醒词
 * @param p_wakeup  唤醒词
 */
void set_algo_wakeup_word(char *p_wakeup);

/**
 * @brief       设置算法超时时间
 * @param frms  单位秒
 */
void set_algo_hold_frms(uint32_t frms);

/**
 * @brief       设置算法高门限超时时间
 * @param frms  单位秒
 */
void set_algo_thre_grade_hold_frms(uint32_t frms);

/**
 * @brief 			给算法处理模式发送消息
 * @param msg_type 	消息的类型
 */
void send_to_algo_task_msg(ls_algo_msg_type_t msg_type);

/**
 * @brief 获取算法运行状态
 * 
 * @return esr_algo_state_t 
 */
esr_algo_state_t get_algo_state();

/**
 * @brief 						创建esr处理线程
 * @param algo_inst_mem 		实例内存
 * @param algo_share_mem 		共享内存
 * @param pool_size 			内存池大小
 * @param high_grade_timeout 	高门限超时时间，当设置为0时，则关闭高门限功能。单位：秒
 */
void lisa_algo_task_create(uint8_t *algo_inst_mem, uint8_t *algo_share_mem, uint32_t pool_size,  uint8_t *algo_voice_mem, uint32_t voice_size, uint32_t high_grade_timeout);

typedef enum {
	REGISTER_INNER_TYPE_CMD = 0, 	// 命令词
	REGISTER_INNER_TYPE_WAKE,		// 唤醒词
	REGISTER_INNER_TYPE_ABSORB		// 吸收词
} register_inner_type_t;

/* 注册类型 */
typedef enum {
	REGISTER_TYPE_WAKE_WORD = 0,	//注册唤醒词
	REGISTER_TYPE_CMD_LIST,     	//注册命令词 - 遍历注册
	REGISTER_TYPE_CMD_INTR,			//注册命令词 - 指定注册
	REGISTER_TYPE_USER_QUIT,        //退出学习模式
	REGISTER_TYPE_UNKONW
} register_type_t;

/* 删除类型 */
typedef enum {
    REGISTER_DELETE_ALL = 0,	//删除所有
    REGISTER_DELETE_WAKE,		//删除唤醒词
    REGISTER_DELETE_CMD,		//删除命令词
    REGISTER_DELETE_INTR_CMD,   //删除指定命令词
    REGISTER_DELETE_ALL_CMD,   //删除指定全部命令词
} register_delete_t;

/* 注册状态 */
typedef enum {
    REGISTER_STATUS_INIT = 0,
    REGISTER_STATUS_START,
    REGISTER_STATUS_SUCCESS,
    REGISTER_STATUS_FAILED,
    REGISTER_STATUS_TIMEOUT_QUIT,
    REGISTER_STATUS_AGAIN,
    REGISTER_STATUS_AUTO_NEXT,
    REGISTER_STATUS_USER_NEXT,
    REGISTER_STATUS_USER_RETRY,
    REGISTER_STATUS_ERROR_CONFLICT,
    REGISTER_STATUS_ERROR_LENGTH,
    REGISTER_STATUS_ERROR_SPEED,
    REGISTER_STATUS_ERROR_SIMILA,
    REGISTER_STATUS_DELETE
} register_status_t;

int voice_register_status_process(register_status_t reg_st, const char *audios, uint8_t size);

int ls_app_register_type_process(register_type_t reg_type, const char *audios, uint8_t size);

int ls_app_register_del_process(register_delete_t del, const char *audios, uint8_t size);

int ls_app_register_set_cmd_intr_words(const char *reg_cmd_words, int reg_cmd_index);

int intr_cmd_voice_bin_init(uint8_t *voice_bin_addr, int res_all_cnt, int *out_voice_size);

int intr_cmd_voice_bin_check_count(uint8_t *voice_bin_addr, int cmd_res_cnt);

#endif /*__CSK_APPLICATION_ALGO_TASK_HEADER__*/