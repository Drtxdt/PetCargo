#ifndef __MODULE_API_DEFINE_HEADER__
#define __MODULE_API_DEFINE_HEADER__

#define AUD_STEP_SAMPS              (160)
typedef short aud_samp_t, aud_step_t[AUD_STEP_SAMPS];

#define MAPI_AADC_CTRL_REC_STOP     (0)
#define MAPI_AADC_CTRL_REC_START    (1)
#define MAPI_AADC_CTRL_REC_RESET    (2)
#define MAPI_AADC_CTRL_GET_SAMPS    (3)
#define MAPI_AADC_CTRL_SET_GAIN     (4)
#define MAPI_AADC_CTRL_SET_CAPLESS  (5)
#define MAPI_AADC_CTRL_SET_PGATYPE  (6)

#define MAPI_CLSD_CTRL_PLAY_STOP    (0)
#define MAPI_CLSD_CTRL_PLAY_START   (1)

#define MAPI_ALGO_EVT_WAKEUP        (0)
#define MAPI_ALGO_EVT_ERRBASE       (10000)
#define MAPI_ALGO_EVT_REFUSED       (MAPI_ALGO_EVT_ERRBASE + 1)
#define MAPI_ALGO_EVT_TIMEOUT       (MAPI_ALGO_EVT_ERRBASE + 2)

#define MAPI_CLSD_EVT_ABORT         (0)

#define MAPI_I2SO_EVT_ABORT         (0)

#define MAPI_XWDT_CTRL_CLOSE        (0)
#define MAPI_XWDT_CTRL_OPEN         (1)
#define MAPI_XWDT_CTRL_FEED         (2)

#define MAPI_TNVS_CTRL_OTP_LOCK_GET (0xA1)
#define MAPI_TNVS_CTRL_OTP_LOCK_SET (0xB2)
#define MAPI_TNVS_CTRL_OTP_ERASE    (0xC3)

#define MAPI_UPROT_CTRL_SET_BAUD    (0xD1)

#define MAPI_CTRL_ECODE_SUCC        (0)
#define MAPI_CTRL_ECODE_FAIL        (-1)

#endif//__MODULE_API_DEFINE_HEADER__
