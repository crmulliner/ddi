/*
 *  Collin's Dynamic Dalvik Instrumentation Toolkit for Android
 *  Collin Mulliner <collin[at]mulliner.org>
 *
 *  (c) 2012,2013
 *
 *  License: LGPL v2.1
 *
 */

#include <string.h>
#include <jni.h>
#include <stdio.h>
#include <fcntl.h>
#include <dlfcn.h>

#include "Common.h"

#ifndef __dexstuff_h__
#define __dexstuff_h__

#define ALLOC_DEFAULT  0x00
#define ALLOC_DONT_TRACK 0x02

struct StringObject;
struct ArrayObject;

typedef struct DexProto {
    u4* dexFile;     /* file the idx refers to */
    u4 protoIdx;                /* index into proto_ids table of dexFile */
} DexProto;


typedef void (*DalvikBridgeFunc)(const u4* args, void* pResult,
    const void* method, void* self);

struct Field {
   void*    clazz;          /* class in which the field is declared */
    const char*     name;
    const char*     signature;      /* e.g. "I", "[C", "Landroid/os/Debug;" */
    u4              accessFlags;
};


struct Method;
struct ClassObject;



struct Object {
    /* ptr to class object */
    struct ClassObject*    clazz;

    /*
     * A word containing either a "thin" lock or a "fat" monitor.  See
     * the comments in Sync.c for a description of its layout.
     */
    u4              lock;
};

struct InitiatingLoaderList {
    /* a list of initiating loader Objects; grown and initialized on demand */
    void**  initiatingLoaders;
    /* count of loaders in the above list */
    int       initiatingLoaderCount;
};

enum PrimitiveType {
    PRIM_NOT        = 0,       /* value is a reference type, not a primitive type */
    PRIM_VOID       = 1,
    PRIM_BOOLEAN    = 2,
    PRIM_BYTE       = 3,
    PRIM_SHORT      = 4,
    PRIM_CHAR       = 5,
    PRIM_INT        = 6,
    PRIM_LONG       = 7,
    PRIM_FLOAT      = 8,
    PRIM_DOUBLE     = 9,
} typedef PrimitiveType;

enum ClassStatus {
    CLASS_ERROR         = -1,

    CLASS_NOTREADY      = 0,
    CLASS_IDX           = 1,    /* loaded, DEX idx in super or ifaces */
    CLASS_LOADED        = 2,    /* DEX idx values resolved */
    CLASS_RESOLVED      = 3,    /* part of linking */
    CLASS_VERIFYING     = 4,    /* in the process of being verified */
    CLASS_VERIFIED      = 5,    /* logically part of linking; done pre-init */
    CLASS_INITIALIZING  = 6,    /* class init in progress */
    CLASS_INITIALIZED   = 7,    /* ready to go */
} typedef ClassStatus;

struct ClassObject {
    struct Object o; // emulate C++ inheritance, Collin
	
    /* leave space for instance data; we could access fields directly if we
       freeze the definition of java/lang/Class */
    u4              instanceData[4];

    /* UTF-8 descriptor for the class; from constant pool, or on heap
       if generated ("[C") */
    const char*     descriptor;
    char*           descriptorAlloc;

    /* access flags; low 16 bits are defined by VM spec */
    u4              accessFlags;

    /* VM-unique class serial number, nonzero, set very early */
    u4              serialNumber;

    /* DexFile from which we came; needed to resolve constant pool entries */
    /* (will be NULL for VM-generated, e.g. arrays and primitive classes) */
    void*         pDvmDex;

    /* state of class initialization */
    ClassStatus     status;

    /* if class verify fails, we must return same error on subsequent tries */
    struct ClassObject*    verifyErrorClass;

    /* threadId, used to check for recursive <clinit> invocation */
    u4              initThreadId;

    /*
     * Total object size; used when allocating storage on gc heap.  (For
     * interfaces and abstract classes this will be zero.)
     */
    size_t          objectSize;

    /* arrays only: class object for base element, for instanceof/checkcast
       (for String[][][], this will be String) */
    struct ClassObject*    elementClass;

    /* arrays only: number of dimensions, e.g. int[][] is 2 */
    int             arrayDim;
 	PrimitiveType   primitiveType;

    /* superclass, or NULL if this is java.lang.Object */
    struct ClassObject*    super;

    /* defining class loader, or NULL for the "bootstrap" system loader */
    struct Object*         classLoader;
	
	struct InitiatingLoaderList initiatingLoaderList;

    /* array of interfaces this class implements directly */
    int             interfaceCount;
    struct ClassObject**   interfaces;

    /* static, private, and <init> methods */
    int             directMethodCount;
    struct Method*         directMethods;

    /* virtual methods defined in this class; invoked through vtable */
    int             virtualMethodCount;
    struct Method*         virtualMethods;

    /*
     * Virtual method table (vtable), for use by "invoke-virtual".  The
     * vtable from the superclass is copied in, and virtual methods from
     * our class either replace those from the super or are appended.
     */
    int             vtableCount;
    struct Method**        vtable;

};
	
typedef struct Method {
	struct ClassObject *clazz;
	u4 a; // accessflags
	
	u2             methodIndex;
	
	u2              registersSize;  /* ins + locals */
    u2              outsSize;
    u2              insSize;

    /* method name, e.g. "<init>" or "eatLunch" */
    const char*     name;

    /*
     * Method prototype descriptor string (return and argument types).
     *
     * TODO: This currently must specify the DexFile as well as the proto_ids
     * index, because generated Proxy classes don't have a DexFile.  We can
     * remove the DexFile* and reduce the size of this struct if we generate
     * a DEX for proxies.
     */
    DexProto        prototype;

    /* short-form method descriptor string */
    const char*     shorty;

    /*
     * The remaining items are not used for abstract or native methods.
     * (JNI is currently hijacking "insns" as a function pointer, set
     * after the first call.  For internal-native this stays null.)
     */

    /* the actual code */
    u2*       insns;
	
	 /* cached JNI argument and return-type hints */
    int             jniArgInfo;

    /*
     * Native method ptr; could be actual function or a JNI bridge.  We
     * don't currently discriminate between DalvikBridgeFunc and
     * DalvikNativeFunc; the former takes an argument superset (i.e. two
     * extra args) which will be ignored.  If necessary we can use
     * insns==NULL to detect JNI bridge vs. internal native.
     */
    DalvikBridgeFunc  nativeFunc;

#ifdef WITH_PROFILER
    bool            inProfile;
#endif
#ifdef WITH_DEBUGGER
    short           debugBreakpointCount;
#endif

  bool fastJni;

    /*
     * JNI: true if this method has no reference arguments. This lets the JNI
     * bridge avoid scanning the shorty for direct pointers that need to be
     * converted to local references.
     *
     * TODO: replace this with a list of indexes of the reference arguments.
     */
    bool noRef;

	
} Method;

typedef void (*DalvikNativeFunc)(const u4* args, jvalue* pResult);

typedef struct DalvikNativeMethod_t {
    const char* name;
    const char* signature;
    DalvikNativeFunc  fnPtr;
} DalvikNativeMethod;

typedef void* (*dvmCreateStringFromCstr_func)(const char* utf8Str, int len, int allocFlags);
typedef void* (*dvmGetSystemClassLoader_func)(void);
typedef void* (*dvmThreadSelf_func)(void);

typedef void* (*dvmIsClassInitialized_func)(void*);
typedef void* (*dvmInitClass_func)(void*);
typedef void* (*dvmFindVirtualMethodHierByDescriptor_func)(void*,const char*, const char*);
typedef void* (*dvmFindDirectMethodByDescriptor_func)(void*,const char*, const char*);
typedef void* (*dvmIsStaticMethod_func)(void*);
typedef void* (*dvmAllocObject_func)(void*, unsigned int);
typedef void* (*dvmCallMethodV_func)(void*,void*,void*,void*,va_list);
typedef void* (*dvmCallMethodA_func)(void*,void*,void*,bool,void*,jvalue*);
typedef void* (*dvmAddToReferenceTable_func)(void*,void*);
typedef void (*dvmDumpAllClasses_func)(int);
typedef void* (*dvmFindLoadedClass_func)(const char*);

typedef void (*dvmUseJNIBridge_func)(void*, void*);

typedef void* (*dvmDecodeIndirectRef_func)(void*,void*);

typedef void* (*dvmGetCurrentJNIMethod_func)();

typedef void (*dvmLinearSetReadWrite_func)(void*,void*);

typedef void* (*dvmFindInstanceField_func)(void*,const char*,const char*);

typedef void* (*dvmSetNativeFunc_func)(void*,void*, void*);
typedef void (*dvmCallJNIMethod_func)(const u4*, void*, void*, void*);

typedef void (*dvmHashTableLock_func)(void*);
typedef void (*dvmHashTableUnlock_func)(void*);
typedef void (*dvmHashForeach_func)(void*,void*,void*);

typedef void (*dvmDumpClass_func)(void*,void*);

typedef int (*dvmInstanceof_func)(void*,void*);

struct dexstuff_t
{	
	void *dvm_hand;
	
	dvmCreateStringFromCstr_func dvmStringFromCStr_fnPtr;
	dvmGetSystemClassLoader_func dvmGetSystemClassLoader_fnPtr;
	dvmThreadSelf_func dvmThreadSelf_fnPtr;

	dvmIsClassInitialized_func dvmIsClassInitialized_fnPtr;
	dvmInitClass_func dvmInitClass_fnPtr;
	dvmFindVirtualMethodHierByDescriptor_func dvmFindVirtualMethodHierByDescriptor_fnPtr;
	dvmFindDirectMethodByDescriptor_func dvmFindDirectMethodByDescriptor_fnPtr;
	dvmIsStaticMethod_func dvmIsStaticMethod_fnPtr;
	dvmAllocObject_func dvmAllocObject_fnPtr;
	dvmCallMethodV_func dvmCallMethodV_fnPtr;
	dvmCallMethodA_func dvmCallMethodA_fnPtr;
	dvmAddToReferenceTable_func dvmAddToReferenceTable_fnPtr;
	dvmDecodeIndirectRef_func dvmDecodeIndirectRef_fnPtr;
	dvmUseJNIBridge_func dvmUseJNIBridge_fnPtr;
	dvmFindInstanceField_func dvmFindInstanceField_fnPtr;
	dvmFindLoadedClass_func dvmFindLoadedClass_fnPtr;
	dvmDumpAllClasses_func dvmDumpAllClasses_fnPtr;
	
	dvmGetCurrentJNIMethod_func dvmGetCurrentJNIMethod_fnPtr;
	dvmLinearSetReadWrite_func dvmLinearSetReadWrite_fnPtr;
	
	dvmSetNativeFunc_func dvmSetNativeFunc_fnPtr;
	dvmCallJNIMethod_func dvmCallJNIMethod_fnPtr;
	
	dvmHashTableLock_func dvmHashTableLock_fnPtr;
	dvmHashTableUnlock_func dvmHashTableUnlock_fnPtr;
	dvmHashForeach_func dvmHashForeach_fnPtr;
	
	dvmDumpClass_func dvmDumpClass_fnPtr;
	dvmInstanceof_func dvmInstanceof_fnPtr;
	
	DalvikNativeMethod *dvm_dalvik_system_DexFile;
	DalvikNativeMethod *dvm_java_lang_Class;
		
	void *gDvm; // dvm globals !
	
	int done;
};

#endif

void dexstuff_resolv_dvm(struct dexstuff_t *d);
int dexstuff_loaddex(struct dexstuff_t *d, char *path);
void* dexstuff_defineclass(struct dexstuff_t *d, char *name, int cookie);
void* getSelf(struct dexstuff_t *d);
void dalvik_dump_class(struct dexstuff_t *dex, char *clname);

