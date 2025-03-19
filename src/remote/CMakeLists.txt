cmake_minimum_required(VERSION 3.20.0)

find_package(Protobuf REQUIRED)
find_library(ABSL_ADD_1 absl_log_internal_check_op)
find_library(ABSL_ADD_2 absl_log_internal_message)
include_directories()
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_library(ZMQ_LIB zmq)
find_path(ZMQ_H zmq.h)
find_path(CPPZMQ_H zmq.hpp)

if(NOT ZMQ_LIB)
  message(FATAL_ERROR "cannot find zmq library")
endif()

protobuf_generate_cpp(PROTO_DTO_SRC PROTO_DTO_HDRS dto.proto)

add_executable(
  cli_server
  cli_server.cpp
  client.cpp
  dto_utils.cpp
  server.cpp
  ${PROTO_DTO_SRC}
  ${PROTO_DTO_HDRS}
  ${ZMQ_H}
  ${CPPZMQ_H}
  ${Protobuf_INCLUDE_DIRS})
target_link_libraries(cli_server ${Protobuf_LIBRARIES} ${ZMQ_LIB} ${ABSL_ADD_1}
                      ${ABSL_ADD_2} ${TTTCORE_LIB})
target_include_directories(cli_server PUBLIC ${ZMQ_H} ${CPPZMQ_H}
                                             ${Protobuf_INCLUDE_DIRS})

add_executable(
  cli_client
  cli_client.cpp
  client.cpp
  dto_utils.cpp
  server.cpp
  ${PROTO_DTO_SRC}
  ${PROTO_DTO_HDRS}
  ${ZMQ_H}
  ${CPPZMQ_H}
  ${Protobuf_INCLUDE_DIRS})
target_link_libraries(
  cli_client
  ${Protobuf_LIBRARIES}
  ${ZMQ_LIB}
  ${ABSL_ADD_1}
  ${ABSL_ADD_2}
  ${TTTCORE_LIB}
  tttplayer)
target_include_directories(cli_client PUBLIC ${ZMQ_H} ${CPPZMQ_H}
                                             ${Protobuf_INCLUDE_DIRS})
