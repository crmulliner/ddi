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
        {FILE *fp = fopen("/data/local/tmp/strmon.log", "a+"); if (fp) {\
        fprintf(fp, __VA_ARGS__);\
        fclose(fp);}}

static struct hook_t eph;
static struct dexstuff_t d;

static int debug;

static void my_log(char *msg)
{
	log("%s", msg)
}
static void my_log2(char *msg)
{
	if (debug)
		log("%s", msg)
}

static struct dalvik_hook_t sb1;
static struct dalvik_hook_t sb2;
static struct dalvik_hook_t sb3;
static struct dalvik_hook_t sb5;
static struct dalvik_hook_t sb6;
static struct dalvik_hook_t sb7;
static struct dalvik_hook_t sb8;
static struct dalvik_hook_t sb9;
static struct dalvik_hook_t sb10;
static struct dalvik_hook_t sb11;
static struct dalvik_hook_t sb13;
static struct dalvik_hook_t sb14;
static struct dalvik_hook_t sb20;

// helper function
void printString(JNIEnv *env, jobject str, char *l)
{
	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		log("%s%s\n", l, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}
}

// patches
static void* sb1_tostring(JNIEnv *env, jobject obj)
{
	dalvik_prepare(&d, &sb1, env);
	void *res = (*env)->CallObjectMethod(env, obj, sb1.mid); 
	//log("success calling : %s\n", sb1.method_name)
	dalvik_postcall(&d, &sb1);

	const char *s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		log("sb1.toString() = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}

	return res;
}

static void* sb20_tostring(JNIEnv *env, jobject obj)
{
/*
	log("tostring\n")
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
*/

	dalvik_prepare(&d, &sb20, env);
	void *res = (*env)->CallObjectMethod(env, obj, sb20.mid); 
	//log("success calling : %s\n", sb20.method_name)
	dalvik_postcall(&d, &sb20);

	const char *s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		log("sb20.toString() = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}

	return res;
}

static void* sb2_compareto(JNIEnv *env, jobject obj, jobject str)
{
/*
	log("compareto\n")
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
	log("str = 0x%x\n", str)
*/

	jvalue args[1];
	args[0].l = str;
	dalvik_prepare(&d, &sb2, env);
	int res = (*env)->CallIntMethodA(env, obj, sb2.mid, args); 
	//log("success calling : %s\n", sb2.method_name)
	dalvik_postcall(&d, &sb2);

	printString(env, obj, "sb2 = "); 

	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		log("sb2.comapreTo() = %d s> %s\n", res, s)
		(*env)->ReleaseStringUTFChars(env, str, s);
	}
	
	return (void *)res;
}

static void* sb3_comparetocase(JNIEnv *env, jobject obj, jobject str)
{
/*
	log("comparetocase\n")
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
	log("str = 0x%x\n", str)
*/

	jvalue args[1];
	args[0].l = str;
	dalvik_prepare(&d, &sb3, env);
	int res = (*env)->CallIntMethodA(env, obj, sb3.mid, args); 
	//log("success calling : %s\n", sb3.method_name)
	dalvik_postcall(&d, &sb3);

	printString(env, obj, "sb3 = "); 

	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		log("sb3.comapreToIgnoreCase() = %d s> %s\n", res, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}

	return (void *)res;
}

static void* sb7_indexof(JNIEnv *env, jobject obj, jobject str, jint i)
{
/*
	log("indexof\n")
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
	log("str = 0x%x\n", str)
*/

	jvalue args[2];
	args[0].l = str;
	args[1].i = i;
	dalvik_prepare(&d, &sb7, env);
	int res = (*env)->CallIntMethodA(env, obj, sb7.mid, args);
	//log("success calling : %s\n", sb7.method_name)
	dalvik_postcall(&d, &sb7);

	printString(env, obj, "sb7 = "); 

	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		log("sb7.indexOf() = %d (i=%d) %s\n", res, i, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}

	return (void *)res;
}

static void* sb11_indexof(JNIEnv *env, jobject obj, jobject str, jint i)
{
/*
	log("indexof\n")
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
	log("str = 0x%x\n", str)
*/

	jvalue args[2];
	args[0].l = str;
	args[1].i = i;
	dalvik_prepare(&d, &sb11, env);
	int res = (*env)->CallIntMethodA(env, obj, sb11.mid, args);
	//log("success calling : %s\n", sb11.method_name)
	dalvik_postcall(&d, &sb11);

	printString(env, obj, "sb11 = "); 

	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		log("sb11.indexOf() = %d (i=%d) %s\n", res, i, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}

	return (void *)res;
}

static void* sb10_startswith(JNIEnv *env, jobject obj, jobject str, jint i)
{
/*
	log("indexof\n")
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
	log("str = 0x%x\n", str)
*/

	jvalue args[2];
	args[0].l = str;
	args[1].i = i;
	dalvik_prepare(&d, &sb10, env);
	int res = (*env)->CallBooleanMethodA(env, obj, sb10.mid, args);
	//log("success calling : %s\n", sb10.method_name)
	dalvik_postcall(&d, &sb10);

	printString(env, obj, "sb10 = "); 

	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		log("sb10.startswith() = %d (i=%d) %s\n", res, i, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}

	return (void *)res;
}

static void* sb8_matches(JNIEnv *env, jobject obj, jobject str)
{
/*
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
	log("str = 0x%x\n", str)
*/

	jvalue args[1];
	args[0].l = str;
	dalvik_prepare(&d, &sb8, env);
	int res = (*env)->CallBooleanMethodA(env, obj, sb8.mid, args);
	//log("success calling : %s\n", sb8.method_name)
	dalvik_postcall(&d, &sb8);

	printString(env, obj, "sb8 = "); 

	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		log("sb8.matches() = %d %s\n", res, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}

	return (void *)res;
}

static void* sb13_equalsIgnoreCase(JNIEnv *env, jobject obj, jobject str)
{
/*
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
	log("str = 0x%x\n", str)
*/

	jvalue args[1];
	args[0].l = str;
	dalvik_prepare(&d, &sb13, env);
	int res = (*env)->CallBooleanMethodA(env, obj, sb13.mid, args);
	//log("success calling : %s\n", sb13.method_name)
	dalvik_postcall(&d, &sb13);

	printString(env, obj, "sb13 = "); 

	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		log("sb13.equalsIgnoreCase() = %d %s\n", res, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}

	return (void *)res;
}

static void* sb14_contentEquals(JNIEnv *env, jobject obj, jobject str)
{
/*
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
	log("str = 0x%x\n", str)
*/

	jvalue args[1];
	args[0].l = str;
	dalvik_prepare(&d, &sb14, env);
	int res = (*env)->CallBooleanMethodA(env, obj, sb14.mid, args);
	//log("success calling : %s\n", sb14.method_name)
	dalvik_postcall(&d, &sb14);

	printString(env, obj, "sb14 = "); 

	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		log("sb14.contentEquals() = %d %s\n", res, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}

	return (void *)res;
}

static void* sb9_endswith(JNIEnv *env, jobject obj, jobject str)
{
/*
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
	log("str = 0x%x\n", str)
*/

	jvalue args[1];
	args[0].l = str;
	dalvik_prepare(&d, &sb9, env);
	int res = (*env)->CallBooleanMethodA(env, obj, sb9.mid, args);
	//log("success calling : %s\n", sb9.method_name)
	dalvik_postcall(&d, &sb9);

	printString(env, obj, "sb9 = "); 

	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		log("sb9.endswith() = %d %s\n", res, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}

	return (void *)res;
}


static void* sb6_contains(JNIEnv *env, jobject obj, jobject str)
{
/*
	log("contains\n")
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
	log("str = 0x%x\n", str)
*/

	jvalue args[1];
	args[0].l = str;
	dalvik_prepare(&d, &sb6, env);
	int res = (*env)->CallBooleanMethodA(env, obj, sb6.mid, args);
	//log("success calling : %s\n", sb6.method_name)
	dalvik_postcall(&d, &sb6);

	printString(env, obj, "sb6 = "); 

	return (void *)res;
}

static void* sb5_getmethod(JNIEnv *env, jobject obj, jobject str, jobject cls)
{
/*
	log("getmethod\n")
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
	log("str = 0x%x\n", str)
	log("cls = 0x%x\n", cls)
*/

	jvalue args[2];
	args[0].l = str;
	args[1].l = cls;
	dalvik_prepare(&d, &sb5, env);
	void *res = (*env)->CallObjectMethodA(env, obj, sb5.mid, args); 
	log("success calling : %s\n", sb5.method_name)
	dalvik_postcall(&d, &sb5);

	if (str) {
		const char *s = (*env)->GetStringUTFChars(env, str, 0);
		if (s) {
			log("sb5.getmethod = %s\n", s)
			(*env)->ReleaseStringUTFChars(env, str, s); 
		}
	}

	return (void *)res;
}

void do_patch()
{
	log("do_patch()\n")

	dalvik_hook_setup(&sb1, "Ljava/lang/StringBuffer;",  "toString",  "()Ljava/lang/String;", 1, sb1_tostring);
	dalvik_hook(&d, &sb1);

	dalvik_hook_setup(&sb20, "Ljava/lang/StringBuilder;",  "toString",  "()Ljava/lang/String;", 1, sb20_tostring);
	dalvik_hook(&d, &sb20);

	dalvik_hook_setup(&sb2, "Ljava/lang/String;", "compareTo", "(Ljava/lang/String;)I", 2, sb2_compareto);
	dalvik_hook(&d, &sb2);

	dalvik_hook_setup(&sb3, "Ljava/lang/String;", "compareToIgnoreCase", "(Ljava/lang/String;)I", 2, sb3_comparetocase);
	dalvik_hook(&d, &sb3);

	dalvik_hook_setup(&sb13, "Ljava/lang/String;", "equalsIgnoreCase", "(Ljava/lang/String;)Z", 2, sb13_equalsIgnoreCase);
	dalvik_hook(&d, &sb13);
	
	dalvik_hook_setup(&sb6, "Ljava/lang/String;", "contains", "(Ljava/lang/CharSequence;)Z", 2, sb6_contains);
	dalvik_hook(&d, &sb6);

	dalvik_hook_setup(&sb14, "Ljava/lang/String;", "contentEquals", "(Ljava/lang/StringBuffer;)Z", 2, sb14_contentEquals);
	dalvik_hook(&d, &sb14);

	dalvik_hook_setup(&sb7, "Ljava/lang/String;", "indexOf", "(Ljava/lang/String;I)I", 3, sb7_indexof);
	dalvik_hook(&d, &sb7);
	
	dalvik_hook_setup(&sb11, "Ljava/lang/StringBuffer;", "indexOf", "(Ljava/lang/String;I)I", 3, sb11_indexof);
	dalvik_hook(&d, &sb11);
	
	dalvik_hook_setup(&sb9, "Ljava/lang/String;", "endsWith", "(Ljava/lang/String;)Z", 2, sb9_endswith);
	dalvik_hook(&d, &sb9);
	
	dalvik_hook_setup(&sb10, "Ljava/lang/String;", "startsWith", "(Ljava/lang/String;I)Z", 3, sb10_startswith);
	dalvik_hook(&d, &sb10);
	
	dalvik_hook_setup(&sb8, "Ljava/lang/String;", "matches", "(Ljava/lang/String;)Z", 2, sb8_matches);
	dalvik_hook(&d, &sb8);

	dalvik_hook_setup(&sb5, "Ljava/lang/Class;", "getMethod", "(Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;", 3, sb5_getmethod);
	dalvik_hook(&d, &sb5);
}

static int my_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	int (*orig_epoll_wait)(int epfd, struct epoll_event *events, int maxevents, int timeout);
	orig_epoll_wait = (void*)eph.orig;
	// remove hook for epoll_wait
	hook_precall(&eph);

	// resolve symbols from DVM
	dexstuff_resolv_dvm(&d);
	// insert hooks
	do_patch();
	
	// call dump class (demo)
	dalvik_dump_class(&d, "Ljava/lang/String;");
        
	// call original function
	int res = orig_epoll_wait(epfd, events, maxevents, timeout);    
	return res;
}

// set my_init as the entry point
void __attribute__ ((constructor)) my_init(void);

void my_init(void)
{
	log("libstrmon: started\n")
 
 	// set to 1 to turn on, this will be noisy
	debug = 0;

 	// set log function for  libbase (very important!)
	set_logfunction(my_log2);
	// set log function for libdalvikhook (very important!)
	dalvikhook_set_logfunction(my_log2);

    hook(&eph, getpid(), "libc.", "epoll_wait", my_epoll_wait, 0);
}
