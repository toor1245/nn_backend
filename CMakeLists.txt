cmake_minimum_required(VERSION 3.26)
project(nn_backend C)

if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

set(CMAKE_C_STANDARD 99)

if(CMAKE_BUILD_TYPE MATCHES "Release")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -mavx2 -mfma")
endif()

include(CheckIncludeFile)
include(CheckSymbolExists)
include(GNUInstallDirs)

macro(setup_include_and_definitions TARGET_NAME)
    target_include_directories(${TARGET_NAME}
            PUBLIC  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/internal>
    )
    target_compile_definitions(${TARGET_NAME}
            PUBLIC NN_BACKEND_VALIDATION_LAYER=0
    )
endmacro()

macro(add_nn_backend_headers_and_sources HDRS_LIST_NAME SRCS_LIST_NAME)
    list(APPEND ${HDRS_LIST_NAME} ${PROJECT_SOURCE_DIR}/include/nn_backend_macros.h)
    list(APPEND ${HDRS_LIST_NAME} ${PROJECT_SOURCE_DIR}/include/cpu_features_cache_info.h)
    file(GLOB IMPL_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/nn_*.c")
    list(APPEND ${SRCS_LIST_NAME} ${IMPL_SOURCES})
endmacro()

set(NN_BACKEND_HDRS)
set(NN_BACKEND_SRCS)
add_nn_backend_headers_and_sources(NN_BACKEND_HDRS, NN_BACKEND_SRCS)


add_library(nn_backend SHARED ${NN_BACKEND_HDRS} ${NN_BACKEND_SRCS})
setup_include_and_definitions(nn_backend)
target_link_libraries(nn_backend glfw vulkan)

add_executable(check_conv2d ${PROJECT_SOURCE_DIR}/src/utils/check_conv2d.c)
target_link_libraries(check_conv2d PRIVATE nn_backend)
add_executable(nn_backend:check_conv2d ALIAS check_conv2d)
