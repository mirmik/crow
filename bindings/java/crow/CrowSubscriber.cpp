#include "crow_CrowSubscriber.h"
#include <crow/nodes/subscriber_node.h>
#include "Crow_library.h"
#include <iostream>

void subscriber_callback_helper(
    void * privarg,
    igris::buffer data)
{
	JNIEnv * env;
	int getEnvStat = g_vm->GetEnv((void **)&env, JNI_VERSION_1_6);
	if (getEnvStat == JNI_EDETACHED)
	{
		if (g_vm->AttachCurrentThread((void**)&env, NULL) != 0)
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
	jclass cls = env->GetObjectClass((jobject)privarg);
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

	env->CallVoidMethod((jobject)privarg, g_mid, jb);

	//env->ReleaseByteArrayElements(jb, (jbyte*) data.data(), JNI_ABORT);
	env->DeleteLocalRef(jb);
}


JNIEXPORT jlong JNICALL Java_crow_CrowSubscriber_nativeNew
(JNIEnv * env, jobject obj)
{
	auto ptr = new crow::subscriber(
	    igris::make_delegate(subscriber_callback_helper, env->NewGlobalRef(obj)));

	return (jlong) ptr;
}
