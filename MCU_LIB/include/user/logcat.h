/**
 *
 * @ 名称: logcat.h
 * @ 描述: TODO
 * @ 作者: lxw
 * @ 日期: Jan 8, 2023
 * @ 版本: V1.0
 *
 * Copyright (c) 2022. All rights reserved.
 */
#ifndef INCLUDE_USER_LOGCAT_H_
#define INCLUDE_USER_LOGCAT_H_

#define ALOGV(fmt, args...)                     logcat(LOG_TAG, __FUNCTION__, __LINE__, 'V', fmt, ##args)
#define ALOGI(fmt, args...)                     logcat(LOG_TAG, __FUNCTION__, __LINE__, 'I', fmt, ##args)
#define ALOGD(fmt, args...)                     logcat(LOG_TAG, __FUNCTION__, __LINE__, 'D', fmt, ##args)
#define ALOGW(fmt, args...)                     logcat(LOG_TAG, __FUNCTION__, __LINE__, 'W', fmt, ##args)
#define ALOGE(fmt, args...)                     logcat(LOG_TAG, __FUNCTION__, __LINE__, 'E', fmt, ##args)
#define ALOGF(fmt, args...)                     logcat(LOG_TAG, __FUNCTION__, __LINE__, 'F', fmt, ##args)


enum logcat_type {
    LOGCAT_UART,
};
typedef enum logcat_type logcat_type_t;


struct bsp_logcat_ops {
    int (*init)(void* bsp);
    int (*deinit)(void* bsp);
    int (*send_char)(void* bsp, const char ch);
    int (*send)(void* bsp, const char* const str);
};

typedef struct bsp_logcat_ops bsp_logcat_ops_t;

int logcat_init(logcat_type_t type);
int logcat_deinit();
void logcat(char* tag, const char* func, int line, char level, const char * fmt, ...);

#endif /* INCLUDE_USER_LOGCAT_H_ */
