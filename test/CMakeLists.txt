
project(clog_test LANGUAGES C)

find_package(CLog REQUIRED)

add_executable(clog_test main.c)

target_link_libraries(clog_test PRIVATE CLog)

