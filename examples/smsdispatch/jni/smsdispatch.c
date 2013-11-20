/*
 *  Collin's Dynamic Dalvik Instrumentation Toolkit for Android
 *  Collin Mulliner <collin[at]mulliner.org>
 *
 *  (c) 2012,2013
 *
 *  License: LGPL v2.1
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <string.h>
#include <termios.h>
#include <pthread.h>
#include <sys/epoll.h>

#include <jni.h>
#include <stdlib.h>

#include "hook.h"
#include "dexstuff.h"
#include "dalvik_hook.h"
#include "base.h"

#undef log

#define log(...) \
        {FILE *fp = fopen("/data/local/tmp/smsdispatch.log", "a+");\
        fprintf(fp, __VA_ARGS__);\
        fclose(fp);}

static struct hook_t eph;
static struct dexstuff_t d;
static struct dalvik_hook_t dpdu;

// switch for debug output of dalvikhook and dexstuff code
static int debug;

static void my_log(char *msg)
{
	log(msg)
}
static void my_log2(char *msg)
{
	if (debug)
		log(msg);
}

static void my_dispatch(JNIEnv *env, jobject obj, jobjectArray pdu)
{
	/*
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
	log("pdu = 0x%x\n", pdu)
	*/
		
	// load dex classes
	int cookie = dexstuff_loaddex(&d, "/data/local/tmp/ddiclasses.dex");
	log("libsmsdispatch: loaddex res = %x\n", cookie)
	if (!cookie)
		log("libsmsdispatch: make sure /data/dalvik-cache/ is world writable and delete data@local@tmp@ddiclasses.dex\n")
	void *clazz = dexstuff_defineclass(&d, "org/mulliner/ddiexample/SMSDispatch", cookie);
	log("libsmsdispatch: clazz = 0x%x\n", clazz)

	// call constructor and passin the pdu
	jclass smsd = (*env)->FindClass(env, "org/mulliner/ddiexample/SMSDispatch");
	jmethodID constructor = (*env)->GetMethodID(env, smsd, "<init>", "([[B)V");
	if (constructor) { 
		jvalue args[1];
		args[0].l = pdu;

		jobject obj = (*env)->NewObjectA(env, smsd, constructor, args);      
		log("libsmsdispatch: new obj = 0x%x\n", obj)
		
		if (!obj)
			log("libsmsdispatch: failed to create smsdispatch class, FATAL!\n")
	}
	else {
		log("libsmsdispatch: constructor not found!\n")
	}

	// call original SMS dispatch method
	jvalue args[1];
	args[0].l = pdu;
	dalvik_prepare(&d, &dpdu, env);
	(*env)->CallVoidMethodA(env, obj, dpdu.mid, args);
	log("success calling : %s\n", dpdu.method_name)
	dalvik_postcall(&d, &dpdu);
}

static int my_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	int (*orig_epoll_wait)(int epfd, struct epoll_event *events, int maxevents, int timeout);
	orig_epoll_wait = (void*)eph.orig;
	// remove hook for epoll_wait
	hook_precall(&eph);

	// resolve symbols from DVM
	dexstuff_resolv_dvm(&d);
	
	// hook
	dalvik_hook_setup(&dpdu, "Lcom/android/internal/telephony/SMSDispatcher;", "dispatchPdus", "([[B)V", 2, my_dispatch);
	dalvik_hook(&d, &dpdu);
	        
	// call original function
	int res = orig_epoll_wait(epfd, events, maxevents, timeout);    
	return res;
}


// set my_init as the entry point
void __attribute__ ((constructor)) my_init(void);

void my_init(void)
{
	log("libsmsdispatch: started\n")
 
 	debug = 1;
 	// set log function for  libbase (very important!)
	set_logfunction(my_log2);
	// set log function for libdalvikhook (very important!)
	dalvikhook_set_logfunction(my_log2);

	hook(&eph, getpid(), "libc.", "epoll_wait", my_epoll_wait, 0);
}
