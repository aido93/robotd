cmake_minimum_required(VERSION 3.0.2)
project(robotd)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -pedantic -march=native -std=c++14 -pthread -D_REENTRANT -DDEBUG -DTRACE")
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
set( CMAKE_CXX_COMPILER clang++)
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
include_directories( ../include ./include ./daemon ./loader ./slaves/periphery ./slaves/test ./slaves/include)

find_package (Threads REQUIRED)
##########################################################
find_package(Boost COMPONENTS thread system REQUIRED)
if(NOT Boost_FOUND)
	message(SEND_ERROR "Failed to find boost::thread.")
	return()
else()
	include_directories(${Boost_INCLUDE_DIRS})
endif()
##########################################################
set(SOURCE_FILES
        directories.cpp
        daemon/cli_parser.cpp
        server_main.cpp
        slaves/test/connection.cpp
        slaves/test/request_handler.cpp
        #nets/async_client.cpp
        slaves/test/tcp_server.cpp
        slaves/test/reply.cpp
        )

add_executable(robotd ${SOURCE_FILES})
target_link_libraries(robotd config++ ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
