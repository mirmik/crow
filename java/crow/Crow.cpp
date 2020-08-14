#include <iostream>
#include "Crow.h"

#include <crow/tower.h>
#include <crow/gates/udpgate.h>

JNIEXPORT void JNICALL Java_crow_Crow_diagnostic
(JNIEnv *, jclass, jboolean trans, jboolean vital)
{
	crow::diagnostic_setup(trans, vital);
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
