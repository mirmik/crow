#include "CrowPublisher.h"
#include <jni.h>
#include <crow/tower.h>
#include <crow/nodes/publisher_node.h>

extern "C" JNIEXPORT jlong JNICALL Java_crow_CrowPublisher_nativeNew
(JNIEnv * env, jobject obj)
{
	auto ptr = new crow::publisher();
	return (jlong) ptr;
}