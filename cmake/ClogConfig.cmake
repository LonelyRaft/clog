
if(CMAKE_VERSION VERSION_LESS 3.5)
    message(FATAL_ERROR
        "CLog module requires at least CMake version 3.5")
endif()

get_filename_component(CLOG_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)

if(NOT TARGET CLog)
    if(${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
        set(CLOG_SHARED "${CLOG_INSTALL_PREFIX}/${_BIN_DIR}/clog.dll")
        set(CLOG_IMPLIB "${CLOG_INSTALL_PREFIX}/${_LIB_DIR}/clog.lib")
    elseif(${CMAKE_C_COMPILER_ID} STREQUAL GNU)
        set(CLOG_SHARED "${CLOG_INSTALL_PREFIX}/${_BIN_DIR}/libclog.dll")
        set(CLOG_IMPLIB "${CLOG_INSTALL_PREFIX}/${_LIB_DIR}/libclog.dll.a")
    else()
        message(FATAL_ERROR "Unknown Compiler ID: ${CMAKE_C_COMPILER_ID}")
    endif()

    if(NOT EXISTS ${CLOG_SHARED})
        message(FATAL_ERROR "Not Found CLog Shared Library File: ${CLOG_SHARED}")
    endif()

    if(NOT EXISTS ${CLOG_IMPLIB})
        message(FATAL_ERROR "Not Found CLog Import Library File: ${CLOG_IMPLIB}")
    endif()

    set(CLOG_INCDIR "${CLOG_INSTALL_PREFIX}/${_INC_DIR}")

    if(NOT EXISTS ${CLOG_INCDIR})
        message(FATAL_ERROR "Not Found CLog Include Directory: ${CLOG_INCDIR}")
    endif()

    if(TARGET CLog)
        return()
    endif()

    add_library(CLog SHARED IMPORTED)
    set_target_properties(CLog PROPERTIES IMPORTED_LOCATION ${CLOG_SHARED})
    set_target_properties(CLog PROPERTIES IMPORTED_IMPLIB ${CLOG_IMPLIB})
    set_target_properties(CLog PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CLOG_INCDIR})
endif()
