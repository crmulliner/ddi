/*
 *  Collin's Dynamic Dalvik Instrumentation Toolkit for Android
 *  Collin Mulliner <collin[at]mulliner.org>
 *
 *  (c) 2012,2013
 *
 *  License: LGPL v2.1
 *
 */

void (*libdalvikhook_log_function)(char *logmsg);

void* dalvikhook_set_logfunction(void *func);

#define log(...) \
        {char __msg[1024] = {0};\
        snprintf(__msg, sizeof(__msg)-1, __VA_ARGS__);\
        libdalvikhook_log_function(__msg); }
        
