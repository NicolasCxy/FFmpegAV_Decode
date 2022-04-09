cmake_minimum_required(VERSION 3.10.2)

project("ffmpageAudioPlay")
include_directories("include")
#添加一个系统查找目录
#添加系统环境变量
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}")

aux_source_directory(. SOURCE)
add_library(native-lib
        SHARED
        ${SOURCE})

find_library(log-lib
        log)

target_link_libraries(native-lib
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        swresample
        swscale
        android
        OpenSLES
        ${log-lib})