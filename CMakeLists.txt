cmake_minimum_required(VERSION 3.5.1)
project(crow)
set( CMAKE_VERBOSE_MAKEFILE on )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CC_STANDARD 11)

set (PLATFORM_SOURCES )

if (WIN32 OR WIN64)
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS")
endif()

file(GLOB HEADERS 
	crow/*.h
)

file(GLOB SOURCES 
	crow/src/*.cpp
	crow/src/*.c
	crow/src/variants/allocation_malloc.cpp
	crow/src/variants/print.cpp
	crow/src/variants/warn.cpp
	crow/src/variants/stdtime.cpp
	crow/pubsub/*.cpp
	crow/nodes/*.cpp
	crow/proto/*.cpp
	crow/gates/*.cpp
	crow/gates/chardev/*.cpp
	crow/brocker/*.cpp
)

file(GLOB SOURCES_TEST 
	${SOURCES}
	tests/*.cpp
)

set(CROW_SOURCES ${SOURCES} PARENT_SCOPE)

add_library(crow SHARED ${SOURCES} ${PLATFORM_SOURCES})
target_include_directories(crow PRIVATE .)

add_executable(crow_test ${SOURCES_TEST})
target_include_directories(crow_test PRIVATE . tests)
target_link_libraries(crow_test igris nos pthread)

if (UNIX AND NOT APPLE)
	install(TARGETS ${PROJECT_NAME}
		LIBRARY DESTINATION /usr/lib
	)

	install(DIRECTORY ${PROJECT_NAME}
    	    DESTINATION /usr/local/include
    	    PATTERN "*HIDE*" EXCLUDE
   	)
endif()


if (APPLE)
	install(TARGETS ${PROJECT_NAME}
		LIBRARY DESTINATION /usr/local/lib
	)

	install(DIRECTORY ${PROJECT_NAME}
    	    DESTINATION /usr/local/include
    	    PATTERN "*HIDE*" EXCLUDE
   	)
endif()

enable_testing()
add_test(NAME crow_test COMMAND crow_test)
