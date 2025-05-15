cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
include_directories()
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_library(ZMQ_LIB zmq REQUIRED)
find_path(ZMQ_H zmq.h REQUIRED)
find_path(CPPZMQ_H zmq.hpp REQUIRED)

protobuf_generate_cpp(PROTO_DTO_SRC PROTO_DTO_HDRS dto.proto)

# hack for linking missing abseil .so libs
foreach(ADD_LIB $ENV{PB_ADDITIONAL_LIBS})
  find_library(PB_ADD_LIB ${ADD_LIB} NO_CACHE)
  set(Protobuf_LIBRARIES ${Protobuf_LIBRARIES} ${PB_ADD_LIB})
  unset(PB_ADD_LIB)
endforeach()

add_library(tttremote_common client.cpp dto_utils.cpp server.cpp
                             ${PROTO_DTO_SRC} ${PROTO_DTO_HDRS})
target_link_libraries(tttremote_common ${Protobuf_LIBRARIES} ${ZMQ_LIB}
                      ${TTTCORE_LIB})
target_include_directories(tttremote_common PUBLIC ${ZMQ_H} ${CPPZMQ_H}
                                                   ${Protobuf_INCLUDE_DIRS})

add_executable(cli_server cli_server.cpp)
target_link_libraries(cli_server tttremote_common)

add_executable(cli_client cli_client.cpp)
target_link_libraries(cli_client tttremote_common tttplayer)
