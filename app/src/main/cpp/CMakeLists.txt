
cmake_minimum_required(VERSION 3.4.1)

add_subdirectory(librtmp)
include_directories(include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/../../../libs/${ANDROID_ABI}")

add_library( # Sets the name of the library.
        native-lib
        SHARED
        native-lib.cpp
        VideoChannel.cpp
        AudioChannel.cpp
        queue.h)

find_library( # Sets the name of the path variable.
        log-lib
        log)
target_link_libraries( # Specifies the target library.
        native-lib
        rtmp
        x264
        faac
        ${log-lib})