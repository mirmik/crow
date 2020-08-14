cmake_minimum_required(VERSION 2.8)
project(crow_java_binding)

set(CMAKE_BUILD_TYPE Release)

find_package(JNI REQUIRED)

include_directories(${JNI_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(SOURCE_FILES 
	Crow.cpp
)

link_libraries(crowjni crow)

add_library(crowjni SHARED ${SOURCE_FILES})