/**
 *
 * @ 名称: logcat.c
 * @ 描述: TODO
 * @ 作者: lxw
 * @ 日期: Jan 8, 2023
 * @ 版本: V1.0
 *
 * Copyright (c) 2022. All rights reserved.
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "user/logcat.h"
#include "bsp/bsp_uart.h"
#include "util/utils.h"

struct user_logcat {
    logcat_type_t type;
    void* logcat_;
    bsp_logcat_ops_t* logcat_ops_;
};

typedef struct user_logcat user_logcat_t;

static user_logcat_t* logcat_instance;

void logcat(char* tag, const char* func, int line, char level, const char * fmt, ...) {
    if (!logcat_instance) return;
    char buf[256] = "";
    char time[13] = "";
    ms_to_time_string(time, HAL_GetTick());
    sprintf(buf, "%s %s %c %s(%5d) ", time, tag, level, func, line);

    int length = strlen(buf);
    va_list args;
    va_start (args, fmt);
    vsprintf (buf+length, fmt, args);
    va_end (args);
    if (logcat_instance) {
        switch (logcat_instance->type) {
        case LOGCAT_UART:
            logcat_instance->logcat_ops_->send(logcat_instance->logcat_, buf);
            logcat_instance->logcat_ops_->send_char(logcat_instance->logcat_, '\n');
            break;
        }
    }
}

int logcat_init(logcat_type_t type) {
    logcat_instance = (user_logcat_t*)malloc(sizeof(user_logcat_t));
    if (!logcat_instance) return -ENOMEM;
    logcat_instance->logcat_ops_ = (bsp_logcat_ops_t*)malloc(sizeof(bsp_logcat_ops_t));
    if (!logcat_instance->logcat_ops_) goto error;

    int ret = -EUNKWN;
    switch (type) {
    case LOGCAT_UART:
        logcat_instance->type = LOGCAT_UART;
        if (SUCCESS == bsp_uart_get_for_logcat(&logcat_instance->logcat_, logcat_instance->logcat_ops_)) {
            logcat_instance->logcat_ops_->init(logcat_instance->logcat_);
        } else goto error_ops;
        break;
    default:
        goto error_ops;
    }
    return SUCCESS;


error_ops:
    free(logcat_instance->logcat_ops_);
    logcat_instance->logcat_ops_ = NULL;

error:
    free(logcat_instance);
    logcat_instance = NULL;
    return ret;
}

int logcat_deinit() {
    if (logcat_instance) {
        free(logcat_instance->logcat_);
        free(logcat_instance->logcat_ops_);
        free(logcat_instance);
        logcat_instance->logcat_ = NULL;
        logcat_instance->logcat_ops_ = NULL;
        logcat_instance = NULL;
    }

    return SUCCESS;
}
