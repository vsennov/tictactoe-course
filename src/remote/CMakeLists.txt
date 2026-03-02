cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_CXX_STANDARD 17)

find_package(protobuf CONFIG REQUIRED)
include_directories()
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_library(ZMQ_LIB zmq REQUIRED)
find_path(ZMQ_H zmq.h REQUIRED)
find_path(CPPZMQ_H zmq.hpp REQUIRED)

add_library(tttremote_common client.cpp dto_utils.cpp server.cpp dto.proto)
target_link_libraries(tttremote_common protobuf::libprotobuf ${ZMQ_LIB}
                      ${TTTCORE_LIB})
target_include_directories(tttremote_common PUBLIC ${ZMQ_H} ${CPPZMQ_H})
protobuf_generate(TARGET tttremote_common)

add_executable(cli_server cli_server.cpp)
target_link_libraries(cli_server tttremote_common)

add_executable(cli_client cli_client.cpp)
target_link_libraries(cli_client tttremote_common tttplayer)
