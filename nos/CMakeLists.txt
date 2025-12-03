cmake_minimum_required(VERSION 3.5.1)
project(nos LANGUAGES C CXX)

# Настройки стандартов
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Опция: собирать ли тесты
option(NOS_BUILD_TESTS "Build nos tests" ON)

# =========================
# Сбор исходников библиотеки
# =========================

file(GLOB HEADERS
    nos/*.h
    nos/**/*.h
)

file(GLOB NOS_SOURCES CONFIGURE_DEPENDS
    nos/*.cpp
    nos/print/*.cpp
    nos/fprint/*.cpp
    nos/check/*.cpp
    nos/util/osutil.cpp
    nos/util/trace.cpp
    nos/util/numconvert.cpp
    nos/util/nos_numconvert.cpp
    nos/util/string.cpp
    nos/meta/*.cpp
    nos/input/*.cpp
    nos/io/impls/current_ostream_stdout.cpp
    nos/convert/*.cpp
    nos/io/*.cpp
    nos/trent/*.cpp
    nos/log/*.cpp
)

if(UNIX)
    file(GLOB UNIX_SOURCES CONFIGURE_DEPENDS
        nos/util/osutil_unix.cpp
        nos/inet/*.cpp
        nos/log/*.cpp
    )
    list(APPEND NOS_SOURCES ${UNIX_SOURCES})
endif()

if(WIN32)
    file(GLOB WIN_SOURCES CONFIGURE_DEPENDS
        nos/util/osutil_windows.cpp
        nos/inet/*.cpp
    )
    list(APPEND NOS_SOURCES ${WIN_SOURCES})
endif()

# =========================
# Библиотека nos
# =========================

add_library(nos SHARED ${NOS_SOURCES})
target_include_directories(nos
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Флаги компилятора, похожие на licant
if(MSVC)
    # Под MSVC свои флаги, если надо, но сейчас оставим дефолт.
else()
    target_compile_options(nos PRIVATE
        -Weffc++
        -Wall
        -Wextra
    )
endif()

# =========================
# Тесты
# =========================

if(NOS_BUILD_TESTS)
    enable_testing()

    file(GLOB SOURCES_TEST
        tests/*.cpp
        tests/trent/*.cpp
    )

    if(UNIX)
        file(GLOB UNIX_TEST_SOURCES CONFIGURE_DEPENDS
            tests/unix/*.cpp
        )
        list(APPEND SOURCES_TEST ${UNIX_TEST_SOURCES})
    endif()

    add_executable(nos_test
        ${SOURCES_TEST}
        ${NOS_SOURCES} # как у тебя в licant: тесты линкуются вместе с lib
    )

    target_include_directories(nos_test
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    if(NOT MSVC)
        target_compile_options(nos_test PRIVATE
            -Weffc++
            -Wall
            -Wextra
            -Werror=all
            -Werror=pedantic
            -Wno-gnu-zero-variadic-macro-arguments
        )
    endif()

    if(UNIX AND NOT APPLE)
        target_link_libraries(nos_test PRIVATE pthread)
    endif()

    if(WIN32)
        target_link_libraries(nos PRIVATE ws2_32)
        target_link_libraries(nos_test PRIVATE ws2_32)
    endif()

    add_test(NAME nos_test COMMAND nos_test)
endif()

# =========================
# Установка (install)
# =========================

if(UNIX AND NOT APPLE)
    install(TARGETS nos
        LIBRARY DESTINATION /usr/lib
    )

    install(DIRECTORY nos
        DESTINATION /usr/local/include
        PATTERN "*HIDE*" EXCLUDE
    )
elseif(APPLE)
    install(TARGETS nos
        LIBRARY DESTINATION /usr/local/lib
    )

    install(DIRECTORY nos
        DESTINATION /usr/local/include
        PATTERN "*HIDE*" EXCLUDE
    )
elseif(WIN32)
    install(TARGETS nos
        EXPORT nos
        LIBRARY DESTINATION /
    )

    install(EXPORT nos
        NAMESPACE nos::
        FILE nos-config.cmake
        DESTINATION lib/cmake/nos
    )

    install(DIRECTORY nos
        DESTINATION "C:/Program Files (x86)/nos/include"
        PATTERN "*HIDE*" EXCLUDE
    )
endif()
