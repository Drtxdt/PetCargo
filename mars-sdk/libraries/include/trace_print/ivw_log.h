#ifndef __IVW_LOG_H__
#define __IVW_LOG_H__


#define LOG_LEVEL_A 0
#define LOG_LEVEL_I 10
#define LOG_LEVEL_V 11
#define LOG_LEVEL_D 12
#define LOG_LEVEL_W 13
#define LOG_LEVEL_E 14
#define LOG_LEVEL_N 100

#define IVW_LOG_LEVEL (LOG_LEVEL_A)

int portIvwTrace(int level, const char *fmt, ...);

#endif
