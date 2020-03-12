
cmake_minimum_required(VERSION 3.4.1)

#由于rtmp是用c 来编译的 而工程是用C++来编译的，所以要设置编译环境
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNO_CRYPTO")

add_library( # Sets the name of the library.
        rtmp
        STATIC
        amf.c
        hashswf.c
        log.c
        parseurl.c
        rtmp.c)