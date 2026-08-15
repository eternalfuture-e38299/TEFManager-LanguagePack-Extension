cmake_minimum_required(VERSION 3.28.3)
project(languagepack_extension)

set(CMAKE_CXX_STANDARD 20)

add_library(languagepack_extension SHARED
        src/core.cpp
        includes/tefkernel-cpp-wrapper/tefkernel/tef_api_imp.c
        src/LanguagePack.cpp
        src/miniz.c
        src/LanguageManager.cpp
)

if (ANDROID)
    target_link_libraries(languagepack_extension PRIVATE log)
endif ()

include_directories(includes)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR ANDROID)
        add_custom_command(TARGET languagepack_extension POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Stripping symbols from languagepack_extension..."
                COMMAND ${CMAKE_STRIP} --strip-all $<TARGET_FILE:languagepack_extension>
                COMMENT "Stripping symbols from languagepack_extension library using strip command"
                VERBATIM
        )
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        # macOS 使用不同的 strip 选项
        add_custom_command(TARGET languagepack_extension POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Stripping symbols from languagepack_extension..."
                COMMAND ${CMAKE_STRIP} -S $<TARGET_FILE:languagepack_extension>
                COMMENT "Stripping symbols from languagepack_extension library using strip command"
                VERBATIM
        )
    endif()
endif()


if(ANDROID)
    set(SYSTEM_NAME "android")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(SYSTEM_NAME "linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(SYSTEM_NAME "mac")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(SYSTEM_NAME "windows")
else()
    set(SYSTEM_NAME "unknown")
endif()

# 获取架构

if(MINGW OR WIN32)
    # Windows: 通过编译器名称判断架构
    if(CMAKE_C_COMPILER MATCHES "x86_64-w64-mingw32" OR CMAKE_CXX_COMPILER MATCHES "x86_64-w64-mingw32")
        set(ARCH "x64")
    elseif(CMAKE_C_COMPILER MATCHES "i686-w64-mingw32" OR CMAKE_CXX_COMPILER MATCHES "i686-w64-mingw32")
        set(ARCH "x86")
    else()
        # 备用方案：通过指针大小判断
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(ARCH "x64")
        else()
            set(ARCH "x86")
        endif()
    endif()
else()
    # 非 Android 平台使用简化的架构名称
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
        set(ARCH "arm64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7|armv7a|ARMv7|arm")
        set(ARCH "arm")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64|x64")
        set(ARCH "x64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i686|i386|x86")
        set(ARCH "x86")
    else()
        set(ARCH "${CMAKE_SYSTEM_PROCESSOR}")
    endif()
endif()

#[[
message(STATUS "========== DEBUG ==========")
message(STATUS "SYSTEM_NAME: ${SYSTEM_NAME}")
message(STATUS "ARCH: ${ARCH}")
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "WIN32: ${WIN32}")
message(STATUS "MINGW: ${MINGW}")
message(STATUS "============================")]]


# 设置输出文件名

set_target_properties(languagepack_extension PROPERTIES
        OUTPUT_NAME "module.${SYSTEM_NAME}.${ARCH}"
)
