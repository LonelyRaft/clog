cmake_minimum_required(VERSION 3.5)

project(clog LANGUAGES C)
# use settings
include(settings.cmake)
# add target
set(CLOG_NAME clog)
add_library(${CLOG_NAME} SHARED "")
# add source files
message(STATUS "Current Compiler ID: ${CMAKE_C_COMPILER_ID}")
file(GLOB CLOG_HEADERS ${_INC_DIR}/*.h)
if(${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
    list(APPEND CLOG_SOURCES ${_SRC_DIR}/clog_msvc.c)
elseif(${CMAKE_C_COMPILER_ID} MATCHES GNU)
    list(APPEND CLOG_SOURCES ${_SRC_DIR}/clog.c)
else()
    message(FATAL_ERROR "Unknown Compiler ID: ${CMAKE_C_COMPILER_ID}")
endif()
target_sources(
    ${CLOG_NAME} PRIVATE
    ${CLOG_HEADERS} ${CLOG_SOURCES}
)
# set include dir
target_include_directories(
    ${CLOG_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/${_INC_DIR}
)
# test program
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(test)
endif()

