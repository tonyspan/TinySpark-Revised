# Minimum CMake required
cmake_minimum_required(VERSION 3.15)

# Project
project(TinySpark-Revised)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -std=c++17 -O3")

# Threads
find_package(Threads REQUIRED)

# Protobuf
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)

# Protobuf-compiler
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

# gRPC
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
set(_GRPC_GRPCPP gRPC::grpc++)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
set(_GRPC_PY_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_python_plugin>)

# Proto file(s)
# Ref: https://github.com/google/or-tools/blob/a0a56698ba8fd07b7f84aee4fc45d891a8cd9828/cmake/cpp.cmake#L234-L279
file(GLOB_RECURSE PROTOS "${PROJECT_SOURCE_DIR}/Protos/*.proto")

set(PROTO_SRCS_CPP)
set(PROTO_HDRS_CPP)
set(GRPC_SRCS_CPP)
set(GRPC_HDRS_CPP)

set(PROTO_SRCS_PY)
set(GRPC_SRCS_PY)

foreach(_file ${PROTOS})

    get_filename_component(PROTO_NAME ${_file} NAME_WE)
    
    get_filename_component(PROTO_PATH "Protos/${PROTO_NAME}.proto" ABSOLUTE)
    get_filename_component(PROTO_PATH_TILL "${PROTO_PATH}" PATH)

    set(PROTO_SRC_CPP "${PROJECT_SOURCE_DIR}/GenProtos/cpp/${PROTO_NAME}.pb.cc")
    set(PROTO_HDR_CPP "${PROJECT_SOURCE_DIR}/GenProtos/cpp/${PROTO_NAME}.pb.h")
    set(GRPC_SRC_CPP "${PROJECT_SOURCE_DIR}/GenProtos/cpp/${PROTO_NAME}.grpc.pb.cc")
    set(GRPC_HDR_CPP "${PROJECT_SOURCE_DIR}/GenProtos/cpp/${PROTO_NAME}.grpc.pb.h")

    set(PROTO_SRC_PY "${PROJECT_SOURCE_DIR}/GenProtos/py/${PROTO_NAME}_pb2.py")
    set(GRPC_SRC_PY "${PROJECT_SOURCE_DIR}/GenProtos/py/${PROTO_NAME}_pb2_grpc.py")

    # Generate sources
    add_custom_command(
        OUTPUT "${PROTO_SRC_CPP}" "${PROTO_HDR_CPP}" "${GRPC_SRC_CPP}" "${GRPC_HDR_CPP}" "${PROTO_SRC_PY}" "${GRPC_SRC_PY}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${PROJECT_SOURCE_DIR}/GenProtos/cpp/"
        --cpp_out "${PROJECT_SOURCE_DIR}/GenProtos/cpp/"
        -I "${PROTO_PATH_TILL}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${PROTO_PATH}"
        DEPENDS "${PROTO_PATH}"

        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_python_out "${PROJECT_SOURCE_DIR}/GenProtos/py"
        --python_out "${PROJECT_SOURCE_DIR}/GenProtos/py"
        -I "${PROTO_PATH_TILL}"
        --plugin=protoc-gen-grpc_python="${_GRPC_PY_PLUGIN_EXECUTABLE}"
        "${PROTO_PATH}"
    )
    list(APPEND PROTO_SRCS_CPP ${PROTO_SRC_CPP})
    list(APPEND PROTO_HDRS_CPP ${PROTO_HDR_CPP})
    list(APPEND GRPC_SRCS_CPP ${GRPC_SRC_CPP})
    list(APPEND GRPC_HDRS_CPP ${GRPC_HDR_CPP})

    list(APPEND PROTO_SRCS_PY ${PROTO_SRCS_PY})
    list(APPEND GRPC_SRCS_PY ${GRPC_SRC_PY})
    
endforeach()

# Include generated *.pb.h files
include_directories("${PROJECT_SOURCE_DIR}/GenProtos/")

# Include "Backend"
include_directories("${PROJECT_SOURCE_DIR}/Core/")
add_subdirectory("${PROJECT_SOURCE_DIR}/Core/")

# Targets (driver|worker)
foreach(_target
    Worker Driver)
    add_executable(${_target} "App/${_target}.cc"
    ${PROTO_SRCS_CPP}
    ${GRPC_SRCS_CPP})
    target_link_libraries(${_target}
    Backend
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    pthread)
    add_dependencies(${_target} Backend)
    
    # Pre-Compiled header file(s)
    target_precompile_headers(${_target} PRIVATE Core/pch.h)
endforeach()