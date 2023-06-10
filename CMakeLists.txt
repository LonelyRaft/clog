cmake_minimum_required(VERSION 3.5)

project(clog LANGUAGES C)

set(TARGET_NAME clog)
add_executable(${TARGET_NAME}
    ".gitignore"
    "readme.md"
)

file(GLOB SRC_LIST *.c *.h)
target_sources(
    ${TARGET_NAME} PRIVATE
    ${SRC_LIST}
)
