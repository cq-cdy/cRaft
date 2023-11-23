cmake_minimum_required(VERSION 3.2)

project(HelloWorld C CXX)

include(common.cmake)
# 设置项目编译头文件搜索路径 -I
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/src)
# # Protos Directory
file(GLOB PROTO_INPUT "./src/protos/*.proto")
get_filename_component(protos_h_cc "./protos_h_cc" ABSOLUTE)
get_filename_component(protos_h_cc "./src/rpc" ABSOLUTE)

message(protos_h_cc = ${protos_h_cc})

# Set the output paths
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# Go through every proto file for processing
foreach(hw_proto ${PROTO_INPUT})
  get_filename_component(hw_proto_path "${hw_proto}" PATH)

  get_filename_component(base_name_we ${hw_proto} NAME_WE)

  set(hw_proto_srcs "${protos_h_cc}/${base_name_we}.pb.cc")
  set(hw_proto_hdrs "${protos_h_cc}/${base_name_we}.pb.h")
  set(hw_grpc_srcs "${protos_h_cc}/${base_name_we}.grpc.pb.cc")
  set(hw_grpc_hdrs "${protos_h_cc}/${base_name_we}.grpc.pb.h")

  add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${protos_h_cc}"
        --cpp_out "${protos_h_cc}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
      DEPENDS "${hw_proto}")

  set(hw_proto_srcs_all ${hw_proto_srcs_all} ${hw_proto_srcs})
  set(hw_proto_hdrs_all ${hw_proto_hdrs_all} ${hw_proto_hdrs})
  set(hw_grpc_srcs_all ${hw_grpc_srcs_all} ${hw_grpc_srcs})
  set(hw_grpc_hdrs_all ${hw_grpc_hdrs_all} ${hw_grpc_hdrs})

endforeach()
aux_source_directory(./src/raft/ RAFT_SRC)
set(hw_grpc_srcs_all ${hw_grpc_srcs_all} ${hw_grpc_srcs}
        src/raft/abstractPersist.cc)

message(raft src = ${RAFT_SRC})
message(raft h = ${RAFT_H})
# # Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# hw_grpc_proto
add_library(hw_grpc_proto
  ${hw_grpc_srcs_all}
  ${hw_grpc_hdrs_all}
  ${hw_proto_srcs_all}
  ${hw_proto_hdrs_all}
        ${RAFT_SRC}
)

target_link_libraries(hw_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# Targets greeter_[async_](client|server)
foreach(_target server client)
  add_executable(${_target} "${_target}.cc")
  target_link_libraries(${_target}

    hw_grpc_proto
    absl::flags
    absl::flags_parse
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    libgo
    dl
    pthread)
endforeach()