#include <iostream>
#include "crow_Crow.h"

#include <crow/tower.h>
#include <crow/gates/udpgate.h>

JavaVM * g_vm;

JNIEXPORT void JNICALL Java_crow_Crow_diagnostic
(JNIEnv *, jclass, jboolean trans)
{
	crow::diagnostic_setup(trans);
}

JNIEXPORT void JNICALL Java_crow_Crow_start_1spin
(JNIEnv *, jclass)
{
	crow::start_spin();
}


JNIEXPORT void JNICALL Java_crow_Crow_join_1spin
(JNIEnv *, jclass)
{
	crow::join_spin();
}

JNIEXPORT void JNICALL Java_crow_Crow_create_1udpgate
(JNIEnv *, jclass, jint id, jint port)
{
	crow::create_udpgate(id, port);
}


JNIEXPORT void JNICALL Java_crow_Crow_init_1crow_1library
(JNIEnv * env, jclass)
{
	env->GetJavaVM(&g_vm);
}
