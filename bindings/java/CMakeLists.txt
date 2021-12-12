cmake_minimum_required(VERSION 3.5.1)
project(crow_java_binding)
set( CMAKE_VERBOSE_MAKEFILE on )

set(CMAKE_SHARED_LINKER_FLAGS "-static-libstdc++")

set(CMAKE_BUILD_TYPE Release)
find_package(JNI REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(SOURCE_FILES 
	crow/Crow.cpp
	crow/CrowSpamSubscriber.cpp
	crow/CrowNode.cpp
	crow/CrowPublisher.cpp
)
add_library(crowjni SHARED ${SOURCE_FILES})

target_include_directories(
	crowjni PUBLIC
	${JNI_INCLUDE_DIRS} 
	../../ 
	/home/mirmik/project/igris
	/home/mirmik/project/nos
)

target_link_directories(crowjni PUBLIC 
	${IGRIS_LIBRARY_PATH}/build-android
	${NOS_LIBRARY_PATH}/build-android
	${CROW_LIBRARY_PATH}/build-android
)
target_link_libraries(crowjni igris nos crow)

