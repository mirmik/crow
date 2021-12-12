#include <jni.h>
#include "CrowNode.h"
#include <crow/tower.h>
#include <crow/proto/node.h>

static crow::node *getObject(JNIEnv *env, jobject self)
{
	jclass cls = env->GetObjectClass(self);
	if (!cls)
		env->FatalError("GetObjectClass failed");

	jfieldID nativeObjectPointerID = env->GetFieldID(cls, "instance", "J");
	if (!nativeObjectPointerID)
		env->FatalError("GetFieldID failed");

	jlong nativeObjectPointer = env->GetLongField(self, nativeObjectPointerID);
	return reinterpret_cast<crow::node *>(nativeObjectPointer);
}

extern "C" JNIEXPORT void JNICALL Java_crow_CrowNode_bind
(JNIEnv * env, jobject jobj, jint id)
{
	auto sub = getObject(env, jobj);
	sub->bind(id);
}
