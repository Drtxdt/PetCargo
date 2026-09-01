#include "appinc.h"
#include "ls_app_client.h"
#include "mini_player.h"
#include "lisa_thread.h"
#include "task_algo.h"
#include "ls_app_device.h"

#define ALGO_RAM_INST_MEM_SIZE      (113792) //115840 - 2k
#define ALGO_VOICE_BIN_MEM_SIZE     (2048) //2k
#define ALGO_RAM_SHARE_MEM_SIZE     (58848)

static uint8_t s_algo_inst_mem[ALGO_RAM_INST_MEM_SIZE] __attribute__((aligned(32)));
static uint8_t s_algo_share_mem[ALGO_RAM_SHARE_MEM_SIZE] __attribute__((aligned(32)));
static uint8_t s_algo_voice_mem[ALGO_VOICE_BIN_MEM_SIZE] __attribute__((aligned(32)));

extern void shell_heap_summary(void);

int32_t get_vcc_power()
{
    int adc_value;
    HAL_GPADC_Initialize(GPADC());
    HAL_GPADC_Control(GPADC(), CSK_GPADC_SAMPLETIME_64 | (CSK_GPADC_CHANNEL_SEL_1_5_VCC) | \
                                CSK_GPADC_INPUT_MODE_Single | CSK_GPADC_VREF_SEL_VBG1_2 | CSK_GPADC_DMA_DISABLE);
    HAL_GPADC_SetTriggerNum(GPADC(), 5);

    HAL_GPADC_Start(GPADC());
    HAL_GPADC_PollForConversion(GPADC(), 0);

    adc_value = HAL_GPADC_GetValue(GPADC(),CSK_GPADC_CHANNEL_SEL_1_5_VCC);
    adc_value = (uint16_t)(adc_value*1000*6/1024);
    // printk("channel is vcc, adc value is %dmV\n", adc_value);
    return adc_value;
}

int main(int argc, char *argv[])
{
    listen_client_create((char *)s_algo_share_mem, (char *)s_algo_voice_mem);

    // 初始化算法
    lisa_algo_task_create(s_algo_inst_mem, s_algo_share_mem, ALGO_RAM_INST_MEM_SIZE + ALGO_RAM_SHARE_MEM_SIZE, s_algo_voice_mem, ALGO_VOICE_BIN_MEM_SIZE, 0);

	// 设置超时时间
    extern int cfg_parser_get_timeout_time();
    set_algo_hold_frms(cfg_parser_get_timeout_time());

    static int get_adc_cnt = 0;
    static int adc_power = 0;
    while (1) {
        if (get_adc_cnt < 3) {
            get_adc_cnt++;
            x_task_sleep(500);
            adc_power = get_vcc_power();
            // printk("==============adc_power : %d==============\n", adc_power);
        } else if (get_adc_cnt == 3) {
            get_adc_cnt++;
            printk("vcc_power:%d\n", adc_power);
            // 此处只针对CSK5062杜亚窗帘修改，其他项目不做修改
            if (adc_power < 4150) {
                IP_AON_CTRL->REG_AON_TUNE1.bit.TUNE_HVLDO_VIO = 0x18;
            }
        } else {
            x_task_sleep(5000);
        }
        // shell_heap_summary();
    }
    return 0;
}
