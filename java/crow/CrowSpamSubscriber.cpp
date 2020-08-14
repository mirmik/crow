#include <jni.h>
#include <iostream>
#include "CrowSpamSubscriber.h"

#include <crow/tower.h>
#include <crow/nodes/spammer.h>

struct envobj
{
	JNIEnv * env;
	jobject obj;
};

JavaVM * g_vm;
jobject g_obj;

void spam_subscriber_helper(
    void * privarg,
    igris::buffer data)
{
	JNIEnv * env;
	int getEnvStat = g_vm->GetEnv((void **)&env, JNI_VERSION_1_6);
	if (getEnvStat == JNI_EDETACHED)
	{
		std::cout << "GetEnv: not attached" << std::endl;
		if (g_vm->AttachCurrentThread((void **) &env, NULL) != 0)
		{
			std::cout << "Failed to attach" << std::endl;
		}
	}
	else if (getEnvStat == JNI_OK)
	{
		//
	}
	else if (getEnvStat == JNI_EVERSION)
	{
		std::cout << "GetEnv: version not supported" << std::endl;
	}
	jclass cls = env->GetObjectClass(g_obj);
	if (cls == NULL)
	{
		std::cout << "Unable to class" << std::endl;
	}

	auto g_mid = env->GetMethodID(cls, "on_message", "([B)V");
	if (g_mid == NULL)
	{
		std::cout << "Unable to get method ref" << std::endl;
	}

	auto jb = env->NewByteArray(data.size());
	env->SetByteArrayRegion(jb, 0, 
		data.size(), (const jbyte *)data.data());

	env->CallVoidMethod(g_obj, g_mid, jb);
}

static
jobject getObjectFromObject(JNIEnv *env, jobject obj,
                            const char * fieldName)
{
	jfieldID fid; /* store the field ID */
	jobject i;

	/* Get a reference to obj's class */
	jclass cls = env->GetObjectClass(obj);

	/* Look for the instance field s in cls */
	fid = env->GetFieldID(cls, fieldName, "J");
	if (fid == NULL)
	{
		return 0; /* failed to find the field */
	}

	/* Read the instance field s */
	i = env->GetObjectField(obj, fid);

	return i;
}

static crow::spam_subscriber *getObject(JNIEnv *env, jobject self)
{
	jclass cls = env->GetObjectClass(self);
	if (!cls)
		env->FatalError("GetObjectClass failed");

	jfieldID nativeObjectPointerID = env->GetFieldID(cls, "instance", "J");
	if (!nativeObjectPointerID)
		env->FatalError("GetFieldID failed");

	jlong nativeObjectPointer = env->GetLongField(self, nativeObjectPointerID);
	return reinterpret_cast<crow::spam_subscriber *>(nativeObjectPointer);
}

JNIEXPORT jlong JNICALL Java_crow_CrowSpamSubscriber_nativeNew
(JNIEnv * env, jobject obj)
{
	env->GetJavaVM(&g_vm);
	//auto eo = new envobj { env, obj };

	auto ptr = new crow::spam_subscriber(
	    igris::make_delegate(spam_subscriber_helper, nullptr));

	g_obj = env->NewGlobalRef(obj);

	return (jlong) ptr;
}

JNIEXPORT void JNICALL Java_crow_CrowSpamSubscriber_bind
(JNIEnv * env, jobject jobj, jint id)
{
	auto sub = getObject(env, jobj);
	sub->bind(id);
}
