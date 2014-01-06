ddi - Dynamic Dalvik Instrumentation Toolkit
===

Simple and easy to use toolkit for dynamic instrumentation of Dalvik code. Instrumentation is based on library injection and hooking method entry points (in-line hooking). The actual instrumentation code is written using the JNI interface.

The DDI further supports loading additional dex classes into a process. This enables instrumentation code to be partially written in Java and thus simplifies interacting with the instrumented process and the Android framework.

The toolkit is based on ADBI (see below) and consists of one main library called dalvikhook. Dalvikhook uses ADBI and the hijack utility that is part of ADBI.

**hijack (from ADBI)**

The hijack tool provides the injection functionality. It supports a number of  modes for supporting older and newer Android devices. hijack provides help on the command line.
 
**libdalvikhook**

The library provides the hooking and unhooking functionality. The library is compiled as a static library so it can be directly included in the actual instrumentation library. This is done so we can keep everything in /data/local/tmp. 

Below we provide and easy to follow step-by-step instructions for howto build and use DDI.

**Examples**

There are two examples included in the library. The *strmon* example hooks a number of methods from String related classes and the the getMethod used for reflection. The *smsdispatch* example hooks the SMSDispatcher of the Android framework. This example loads additional dex classes into the com.android.phone process. The instrumentation code takes every incoming SMS message and reverses the message body and injects a fake message with the reverse message text (you will get two messages). All examples are supplied in full source. For details please read slide deck [1].

=== External Resources ===

more information at: 
 http://www.mulliner.org/android/

slides about this toolkit:

 [1] http://www.mulliner.org/android/feed/mulliner_ddi_summercon2013.pdf
 
 [2] http://www.mulliner.org/android/feed/androidruntime_syscan13.pdf

=== Prerequisites ===


Android SDK

Android NDK

ADBI (see below)

== Build ADBI ==

git clone https://github.com/crmulliner/adbi.git

follow readme

folders should be:
```
 adbi/
 ddi/
```

== Pull Libraries from Device ==

```
cd dalvikhook
cd jni
cd libs
adb pull /system/lib/libdl.so
adb pull /system/lib/libdvm.so
```

== Build libdalvikhook ==

```
cd dalvikhook
cd jni
ndk-build
```

== Build strmon example ==

```
cd examples
cd strmon
cd jni
ndk-build
cd ..
adb push libs/armeabi/libstrmon.so /data/local/tmp
```

== How to Run strmon ==

```
adb shell
su
cd /data/local/tmp
# GET PID from com.android.contacts
>/data/local/tmp/strmon.log
chmod 777 /data/local/tmp/strmon.log
./hijack -d -p PID -l /data/local/tmp/libstrmon.so
cat strmon.log
```

output:

```
libstrmon: started
do_patch
sb20.toString() = en_US
sb13 = Latn
sb13.equalsIgnoreCase() = 0 Arab
sb13 = Latn
sb13.equalsIgnoreCase() = 0 Hebr
sb20.toString() = en-US
sb7 = :
sb7.indexOf() = -1 (i=0) \E
sb20.toString() = \Q:\E
```


== Advanced Options ==

Inject code at application startup before application code starts executing.
This is done by attaching to zygote (-z -p PID_of_zygote) and
using the -s option to supply the main class of application (take from manifest 
or by running 'ps' on the adb shell).

```
adb shell
su
cd /data/local/tmp
# GET PID of >>> zygote <<<
./hijack -d -p PID -z -l /data/local/tmp/libstrmon.so -s com.android.contacts
```

== Build smsdispatch example (advanced!) ==

```
cd examples
cd smsdispatch
cd jni
ndk-build
cd ..
adb push libs/armeabi/libsmsdispatch.so /data/local/tmp
```

== Howto Run smsdispatch ==

```
adb push ddiclasses.dex /data/local/tmp/
adb shell
su
cd /data/local/tmp
>/data/local/tmp/smsdispatch.log
chmod 777 /data/local/tmp/smsdispatch.log
chmod 777 /data/dalvik-cache/
# GET PID from com.android.phone
./hijack -d -p PID -l /data/local/tmp/libsmsdispatch.so
```

send SMS message to that phone (send to yourself if you only have one phone)

further notes: if you have problems that your modified version of ddiclasses.dex is not loaded you need to
remove the class from the dalvik cache ```rm /data/dalvik-cache/data@local@tmp@ddiclasses.dex```

now inspect logfiles and logcat...


```
$ adb logcat
SmsReceiverService( 5527): onStart: #1 mResultCode: -1 = Activity.RESULT_OK
D/dalvikvm( 5527): GC_EXPLICIT freed 264K, 3% free 15600K/15943K, paused 2ms+4ms
D/dalvikvm( 5515): DexOpt: --- BEGIN 'ddiclasses.dex' (bootstrap=0) ---
D/dalvikvm( 5618): DexOpt: load 35ms, verify+opt 160ms
D/dalvikvm( 5515): DexOpt: --- END 'ddiclasses.dex' (success) ---
D/dalvikvm( 5515): DEX prep '/data/local/tmp/ddiclasses.dex': copy in 5ms, rewrite 349ms
I/System.out( 5515): org.mulliner.ddiexample.SMSDispatch(pdu)
I/System.out( 5515): ddiexample: incoming SMS
I/System.out( 5515): ddiexample: Abcd1234 nilloc
I/System.out( 5515): ddiexample: +18571234567
I/System.out( 5515): ddiexample: fake SMS
I/System.out( 5515): ddiexample: collin 4321dcbA
I/System.out( 5515): Intent { act=android.provider.Telephony.SMS_RECEIVED (has extras) }
I/System.out( 5515): ddiexample: appname: com.android.phone.PhoneApp@41816460
V/SmsReceiverService( 5527): onStart: #1 mResultCode: -1 = Activity.RESULT_OK
V/SmsReceiverService( 5527): onStart: #2 mResultCode: -1 = Activity.RESULT_OK
```

smsdispatch.log

```
cat smsdispatch.log

libsmsdispatch: started
hooking:   epoll_wait = 0x400a1378 ARM using 0x46e4a6d4
dvm_hand = 0xb000c490
dvm_dalvik_system_DexFile = 0x408943d0
dvm_java_lang_Class = 0x408946b0
_Z13dvmThreadSelfv = 0x4084184d
_Z32dvmCreateStringFromCstrAndLengthPKcj = 0x408431f5
_Z23dvmGetSystemClassLoaderv = 0x40859f85
_Z21dvmIsClassInitializedPK11ClassObject = 0x408363cd
dvmInitClass = 0x40859a01
_Z36dvmFindVirtualMethodHierByDescriptorPK11ClassObjectPKcS3_ = 0x4085ad85
_Z31dvmFindDirectMethodByDescriptorPK11ClassObjectPKcS3_ = 0x4085ad75
_Z17dvmIsStaticMethodPK6Method = 0x408361ed
dvmAllocObject = 0x40843495
_Z14dvmCallMethodVP6ThreadPK6MethodP6ObjectbP6JValueSt9__va_list = 0x4084f971
_Z14dvmCallMethodAP6ThreadPK6MethodP6ObjectbP6JValuePK6jvalue = 0x4084f81d
_Z22dvmAddToReferenceTableP14ReferenceTableP6Object = 0x4083f615
_Z16dvmSetNativeFuncP6MethodPFvPKjP6JValuePKS_P6ThreadEPKt = 0x4085791d
_Z15dvmUseJNIBridgeP6MethodPv = 0x408385a9
_Z20dvmDecodeIndirectRefP6ThreadP8_jobject = 0x0
_Z21dvmLinearSetReadWriteP6ObjectPv = 0x4083c935
_Z22dvmGetCurrentJNIMethodv = 0x40837041
_Z20dvmFindInstanceFieldPK11ClassObjectPKcS3_ = 0x4085aab9
_Z16dvmCallJNIMethodPKjP6JValuePK6MethodP6Thread = 0x4083be9d
_Z17dvmDumpAllClassesi = 0x40857a69
_Z12dvmDumpClassPK11ClassObjecti = 0x40857f35
_Z18dvmFindLoadedClassPKc = 0x40857aa1
_Z16dvmHashTableLockP9HashTable = 0x40836961
_Z18dvmHashTableUnlockP9HashTable = 0x4083694d
_Z14dvmHashForeachP9HashTablePFiPvS1_ES1_ = 0x40833665
_Z13dvmInstanceofPK11ClassObjectS1_ = 0x40836811
gDvm = 0x4089ac58
dexstuff_loaddex, path = 0x46e4e8f0
cookie = 0x1bae50
libsmsdispatch: loaddex res = 1bae50
dexstuff_defineclass: org/mulliner/ddiexample/SMSDispatch using 1bae50
sys classloader = 0x40a4a400
cur m classloader = 0x0
class = 0x41825c80
libsmsdispatch: clazz = 0x41825c80
libsmsdispatch: new obj = 0x95700025
success calling : dispatchPdus
```
