//
// VirtualApp Native Project
//
// modified version by @rrrfff 2017/06/15
//    fix for incompatible hook with libhoudini (dalvik)
//    fix for ineffectual packageName replacement (native_setup, dalvik)
//
// original versions:
// https://github.com/rk700/VirtualHook/commit/33de8a0947491a7a7844347ea7a06e91265ddcea
// https://github.com/asLody/VirtualApp/commit/cbcd7bbef132a1c51aa059522616fcbec2a5fd74
//
#include "VMPatch.h"
#include "dalvik_vm.h"

//-------------------------------------------------------------------------

typedef void(*Bridge_DalvikBridgeFunc)(const void **, void *, const void *, void *);

typedef jobject(*Native_openDexNative)(JNIEnv *, jclass, jstring, jstring, jint);
typedef jobject(*Native_openDexNative_Art_N)(JNIEnv *, jclass, jstring, jstring, jint, jobject, jobject);

typedef jint(*Native_cameraNativeSetup_T1)(JNIEnv *, jobject, jobject, jint, jstring);
typedef jint(*Native_cameraNativeSetup_T2)(JNIEnv *, jobject, jobject, jint, jint, jstring);
typedef jint(*Native_cameraNativeSetup_T3)(JNIEnv *, jobject, jobject, jint, jint, jstring, jboolean);

typedef jint(*Native_cameraNativeSetup_T4)(JNIEnv *, jobject, jobject, jint, jstring, jboolean);

typedef jint(*Native_getCallingUid)(JNIEnv *, jclass);

typedef jint(*Native_audioRecordNativeCheckPermission)(JNIEnv *, jobject, jstring);

//-------------------------------------------------------------------------

static struct
{
	bool        isArt;
	int         nativeOffset;
	const char *hostPackageName;
	jint        apiLevel;
	jmethodID   method_onGetCallingUid;
	jmethodID   method_onOpenDexFileNative;

	void *art_work_around_app_jni_bugs;

	void *sym_IPCThreadState_self;
	void *sym_IPCThreadState_getCallingUid;
	Native_getCallingUid orig_getCallingUid;

	// workaround for libhoudini crash by @rrrfff 2017/06/14
	Method dalvik_openDexFile;
	Method dalvik_Camera_native_setup;

	int cameraMethodType;
	Bridge_DalvikBridgeFunc orig_cameraNativeSetup_dvm;
	union
	{
		void                           *ptr;
		Native_cameraNativeSetup_T1 t1;
		Native_cameraNativeSetup_T2 t2;
		Native_cameraNativeSetup_T3 t3;
		Native_cameraNativeSetup_T4 t4;
	} orig_native_cameraNativeSetupFunc;

	union
	{
		void                       *ptr;
		Native_openDexNative       beforeN;
		Native_openDexNative_Art_N afterN;
	} orig_native_openDexNative;

	Native_audioRecordNativeCheckPermission orig_native_audioRecordNativeCheckPermission;
} gOffset;

extern JavaVM *gVm;
extern jclass gClass;

//-------------------------------------------------------------------------

static jint getCallingUid(JNIEnv *env, jclass jclazz)
{
	jint uid;
	if (gOffset.isArt) {
		uid = gOffset.orig_getCallingUid(env, jclazz);
	} else {
		auto rel = reinterpret_cast<int(*)(int)>(gOffset.sym_IPCThreadState_getCallingUid);
		int self = reinterpret_cast<int(*)(void)>(gOffset.sym_IPCThreadState_self)();
		uid = rel(self);
	} //if
	uid = env->CallStaticIntMethod(gClass, gOffset.method_onGetCallingUid, uid);
	return uid;
}

//-------------------------------------------------------------------------

static jobject native_openDexNativeFunc(JNIEnv *env, jclass cls, jstring src, jstring dst, jint flag)
{
	jclass stringClass  = env->FindClass("java/lang/String");
	jobjectArray srcdst = env->NewObjectArray(2, stringClass, NULL);
	if (src) env->SetObjectArrayElement(srcdst, 0, src);
	if (dst) env->SetObjectArrayElement(srcdst, 1, dst);
	env->CallStaticVoidMethod(gClass, gOffset.method_onOpenDexFileNative, srcdst);

	jstring newsrc = static_cast<jstring>(env->GetObjectArrayElement(srcdst, 0));
	jstring newdst = static_cast<jstring>(env->GetObjectArrayElement(srcdst, 1));
//	LOGI("%s: src = %s, dst = %s", __FUNCTION__, env->GetStringUTFChars(newsrc, NULL), env->GetStringUTFChars(newdst, NULL));

	if (gOffset.isArt) {
		return gOffset.orig_native_openDexNative.beforeN(env, cls, newsrc, newdst, flag);
	} //if

	jint cookie = env->CallStaticIntMethod(cls,
										   reinterpret_cast<jmethodID>(&gOffset.dalvik_openDexFile),
										   newsrc, newdst, flag);
//	LOGI("%s: cookie = %d", __FUNCTION__, cookie);
	return reinterpret_cast<jobject>(cookie);
}

//-------------------------------------------------------------------------

static jobject native_openDexNativeFunc_art_N(JNIEnv *env, jclass jclazz, jstring javaSourceName,
											  jstring javaOutputName, jint options, jobject loader,
											  jobject elements)
{
	jclass stringClass = env->FindClass("java/lang/String");
	jobjectArray array = env->NewObjectArray(2, stringClass, NULL);

	if (javaSourceName) {
		env->SetObjectArrayElement(array, 0, javaSourceName);
	}
	if (javaOutputName) {
		env->SetObjectArrayElement(array, 1, javaOutputName);
	}
	env->CallStaticVoidMethod(gClass, gOffset.method_onOpenDexFileNative, array);

	jstring newSource = (jstring)env->GetObjectArrayElement(array, 0);
	jstring newOutput = (jstring)env->GetObjectArrayElement(array, 1);

	return gOffset.orig_native_openDexNative.afterN(env, jclazz, newSource, newOutput, options,
														loader, elements);
}

//-------------------------------------------------------------------------

static jint android_hardware_Camera_native_setup_v1(JNIEnv *env, jobject thiz, jobject camera_this,
													jint cameraId, jstring packageName)
{
	packageName = env->NewStringUTF(gOffset.hostPackageName);
	if (gOffset.isArt) {
		return gOffset.orig_native_cameraNativeSetupFunc.t1(env, thiz, camera_this,
															cameraId, packageName);
	} //if

	return env->CallVoidMethod(thiz, reinterpret_cast<jmethodID>(&gOffset.dalvik_Camera_native_setup),
							   camera_this, cameraId, packageName), 0;
}

//-------------------------------------------------------------------------

static jint android_hardware_Camera_native_setup_v2(JNIEnv *env, jobject thiz, jobject camera_this,
													jint cameraId, jint halVersion, jstring packageName)
{
	packageName = env->NewStringUTF(gOffset.hostPackageName);
	return gOffset.orig_native_cameraNativeSetupFunc.t2(env, thiz, camera_this,
														cameraId, halVersion, packageName);
}

//-------------------------------------------------------------------------

static jint android_hardware_Camera_native_setup_v3(JNIEnv *env, jobject thiz, jobject camera_this,
													jint cameraId, jint halVersion,
													jstring packageName, jboolean option) 
{
	packageName = env->NewStringUTF(gOffset.hostPackageName);
	return gOffset.orig_native_cameraNativeSetupFunc.t3(env, thiz, camera_this,
														cameraId, halVersion, packageName, option);
}

//-------------------------------------------------------------------------

static jint android_hardware_Camera_native_setup_v4(JNIEnv *env, jobject thiz, jobject camera_this, 
													jint cameraId, jstring packageName, jboolean option) 
{
	packageName = env->NewStringUTF(gOffset.hostPackageName);
	return gOffset.orig_native_cameraNativeSetupFunc.t4(env, thiz, camera_this,
														cameraId, packageName, option);
}

//-------------------------------------------------------------------------

static jint new_native_audioRecordNativeCheckPermission(JNIEnv *env, jobject thiz, jstring _packagename)
{
	jstring host = env->NewStringUTF(gOffset.hostPackageName);
	return gOffset.orig_native_audioRecordNativeCheckPermission(env, thiz, host);
}

//-------------------------------------------------------------------------

static void mark()
{
	// Do nothing
};

//-------------------------------------------------------------------------

static void measureNativeOffset(JNIEnv *env, bool isArt)
{
	JNINativeMethod markMethods[] = {
		NATIVE_METHOD(mark, "nativeMark", "()V"),
	};
	env->RegisterNatives(gClass, markMethods, NELEM(markMethods));

	size_t startAddress = reinterpret_cast<size_t>(env->GetStaticMethodID(gClass,
																		  markMethods[0].name,
																		  markMethods[0].signature));
	void *targetAddress;
	if (isArt && gOffset.art_work_around_app_jni_bugs) {
		targetAddress = gOffset.art_work_around_app_jni_bugs;
	} else {
		targetAddress = markMethods[0].fnPtr;
	} //if

	int offset = 0;
	bool found = false;
	while (offset < 100) {
		if (reinterpret_cast<void **>(startAddress + offset)[0] == targetAddress) {
			found = true;
			break;
		} //if
		offset += sizeof(u4);
	}
	if (found) {	
		if (!isArt) offset  += (sizeof(int) + sizeof(void *));
		gOffset.nativeOffset = offset;
	} else {
		LOGE("Error: Unable to find the jni function.");
	} //if
}

//-------------------------------------------------------------------------

static void replaceGetCallingUid(JNIEnv *env, jboolean isArt)
{
	jclass binder_class = env->FindClass("android/os/Binder");
	if (isArt) {
		auto md_getCallingUid = reinterpret_cast<char *>(env->GetStaticMethodID(binder_class,
																				"getCallingUid", "()I"));
		auto jniFuncPtr       = reinterpret_cast<void **>(md_getCallingUid + gOffset.nativeOffset);

		gOffset.orig_getCallingUid = reinterpret_cast<Native_getCallingUid>(*jniFuncPtr);
		*jniFuncPtr                = reinterpret_cast<void *>(getCallingUid);
	} else {
		JNINativeMethod uidMethods[] = {
			NATIVE_METHOD(getCallingUid, "getCallingUid", "()I"),
		};
		env->RegisterNatives(binder_class, uidMethods, NELEM(uidMethods));
	} //if
}

//-------------------------------------------------------------------------

static void replaceOpenDexFileMethod(JNIEnv *env, jobject javaMethod, jboolean isArt, int apiLevel)
{
	Method *md_openDexNative   = reinterpret_cast<Method *>(env->FromReflectedMethod(javaMethod));
	if (!isArt) { // workaround for libhoudini crash by @rrrfff 2017/06/14
		gOffset.dalvik_openDexFile = *md_openDexNative;
		jclass clsDexFile          = env->FindClass("dalvik/system/DexFile");
		JNINativeMethod dexMethods[] = {
			NATIVE_METHOD(native_openDexNativeFunc, md_openDexNative->name, "(Ljava/lang/String;Ljava/lang/String;I)I"),
		};
		env->RegisterNatives(clsDexFile, dexMethods, NELEM(dexMethods));
		env->DeleteLocalRef(clsDexFile);
	} else {
		void **jniFuncPtr = reinterpret_cast<void **>(reinterpret_cast<char *>(md_openDexNative) + gOffset.nativeOffset);
		gOffset.orig_native_openDexNative.ptr = *jniFuncPtr;
		if (apiLevel < ANDROID_N) {	
			*jniFuncPtr = reinterpret_cast<void *>(native_openDexNativeFunc);
		} else {
			*jniFuncPtr = reinterpret_cast<void *>(native_openDexNativeFunc_art_N);
		}
	}
}

//-------------------------------------------------------------------------

static void replaceCameraNativeSetupMethod(JNIEnv *env, jobject javaMethod, jboolean isArt, int apiLevel) 
{
	if (!javaMethod) return;
	
	size_t md_Camera_native_setup = reinterpret_cast<size_t>(env->FromReflectedMethod(javaMethod));
	if (!isArt) {
		if (gOffset.cameraMethodType > 1) {
			LOGE("%s: unexpected cameraMethodType %d", __FUNCTION__, gOffset.cameraMethodType);
		} //if
		gOffset.dalvik_Camera_native_setup = *reinterpret_cast<Method *>(md_Camera_native_setup);
		jclass clsCamera             = env->FindClass("android/hardware/Camera");
		JNINativeMethod camMethods[] = {
			NATIVE_METHOD(android_hardware_Camera_native_setup_v1, "native_setup", "(Ljava/lang/Object;ILjava/lang/String;)V"),
		};
		env->RegisterNatives(clsCamera, camMethods, NELEM(camMethods));
		env->DeleteLocalRef(clsCamera);
	} else {
		void **jniFuncPtr = reinterpret_cast<void **>(md_Camera_native_setup + gOffset.nativeOffset);
		gOffset.orig_native_cameraNativeSetupFunc.ptr = *jniFuncPtr;
		switch (gOffset.cameraMethodType) 
		{
		case 1:		
			*jniFuncPtr = reinterpret_cast<void *>(android_hardware_Camera_native_setup_v1);
			break;
		case 2:
			*jniFuncPtr = reinterpret_cast<void *>(android_hardware_Camera_native_setup_v2);
			break;
		case 3:
			*jniFuncPtr = reinterpret_cast<void *>(android_hardware_Camera_native_setup_v3);
			break;
		case 4:
			*jniFuncPtr = reinterpret_cast<void *>(android_hardware_Camera_native_setup_v4);
			break;
		}
	}
}

//-------------------------------------------------------------------------

static void replaceAudioRecordNativeCheckPermission(JNIEnv *env, jobject javaMethod, jboolean isArt, int api) 
{
	if (!javaMethod || !isArt) return;
	
	size_t md_audioRecordNativeCheckPermission = reinterpret_cast<size_t>(env->FromReflectedMethod(javaMethod));
	void **jniFuncPtr = reinterpret_cast<void **>(md_audioRecordNativeCheckPermission + gOffset.nativeOffset);
	gOffset.orig_native_audioRecordNativeCheckPermission = reinterpret_cast<Native_audioRecordNativeCheckPermission>(*jniFuncPtr);
	*jniFuncPtr       = reinterpret_cast<void *>(new_native_audioRecordNativeCheckPermission);
}

//-------------------------------------------------------------------------

static void *getVMHandle() 
{
	char so[PROP_VALUE_MAX];
	if (__system_property_get("persist.sys.dalvik.vm.lib.2", so) == 0) {
		__system_property_get("persist.sys.dalvik.vm.lib", so);
	} //if

	void *soInfo = dlopen(so, 0);
	return !soInfo ? RTLD_DEFAULT : soInfo;
}

//-------------------------------------------------------------------------

/**
 * Only called once.
 * @param javaMethod Method from Java
 * @param isArt Dalvik or Art
 * @param apiLevel Api level from Java
 */
void patchAndroidVM(JNIEnv *env, jobjectArray javaMethods, jstring packageName, jboolean isArt, jint apiLevel,
					jint cameraMethodType)
{
	gOffset.hostPackageName  = env->GetStringUTFChars(packageName, NULL);
	gOffset.isArt            = isArt;
	gOffset.apiLevel         = apiLevel;
	gOffset.cameraMethodType = cameraMethodType;
	gOffset.method_onGetCallingUid     = env->GetStaticMethodID(gClass, "onGetCallingUid", "(I)I");
	gOffset.method_onOpenDexFileNative = env->GetStaticMethodID(gClass, "onOpenDexFileNative",
																"([Ljava/lang/String;)V");
	void *soInfo = getVMHandle();
	if (isArt) {
		gOffset.art_work_around_app_jni_bugs = dlsym(soInfo, "art_work_around_app_jni_bugs");
	} else {
		// workaround for dlsym returns null when system has libhoudini enabled by @rrrfff 2017/06/14
		void *h = dlopen("/system/lib/libandroid_runtime.so", RTLD_LAZY);
		{
			gOffset.sym_IPCThreadState_self = dlsym(RTLD_DEFAULT, "_ZN7android14IPCThreadState4selfEv");
			gOffset.sym_IPCThreadState_getCallingUid = dlsym(RTLD_DEFAULT,
															 "_ZNK7android14IPCThreadState13getCallingUidEv");
			if (gOffset.sym_IPCThreadState_getCallingUid == NULL) {
				gOffset.sym_IPCThreadState_getCallingUid = dlsym(RTLD_DEFAULT,
																 "_ZN7android14IPCThreadState13getCallingUidEv");
			} //if
		}
		if (h != NULL) dlclose(h);
	} //if
	measureNativeOffset(env, isArt);
	replaceGetCallingUid(env, isArt);
	replaceOpenDexFileMethod(env, env->GetObjectArrayElement(javaMethods, OPEN_DEX), isArt,
							 apiLevel);
	if (apiLevel >= 18) { // android 4.3
		replaceCameraNativeSetupMethod(env, env->GetObjectArrayElement(javaMethods, CAMERA_SETUP),
									   isArt, apiLevel);
	} //if
	replaceAudioRecordNativeCheckPermission(env, 
											env->GetObjectArrayElement(javaMethods, VIVO_AUDIORECORD_NATIVE_CHECK_PERMISSION),
											isArt, apiLevel);
}