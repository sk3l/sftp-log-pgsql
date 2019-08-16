cmake_minimum_required(VERSION 3.1)

set (CMAKE_CXX_STANDARD 14)              
set_property(TARGET ${TARGET_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

set(SOURCES_LOG sftp-pgsql-plugin-log.cpp)
set(SOURCES_STORE sftp-pgsql-plugin-store.cpp)
set(INCLUDES sftp-plugin.h sftp-callback.h sftp-common.h)

#include_directories(${CMAKE_SOURCE_DIR})
link_directories(/usr/lib/x86_64-linux-gnu)

message("SOURCES => ${SOURCES}")
message("INCLUDES => ${INCLUDES}")

set(LIBNAME_LOG "${PROJECT_NAME}-log")
set(LIBNAME_LOG_S "${PROJECT_NAME}-log_s")
set(LIBNAME_STORE "${PROJECT_NAME}-store")
set(LIBNAME_STORE_S "${PROJECT_NAME}-store_s")

#target_link_libraries (${LIBNAME_S} ${CMAKE_THREAD_LIBS_INIT})
add_library(${LIBNAME_LOG_S} STATIC ${SOURCES_LOG})
target_link_libraries(${LIBNAME_LOG_S} pqxx)
add_library(${LIBNAME_LOG} SHARED ${SOURCES_LOG} ${INCLUDES})
target_link_libraries(${LIBNAME_LOG} pqxx)

add_library(${LIBNAME_STORE_S} STATIC ${SOURCES_STORE})
target_link_libraries(${LIBNAME_STORE_S} pqxx)
add_library(${LIBNAME_STORE} SHARED ${SOURCES_STORE} ${INCLUDES})
target_link_libraries(${LIBNAME_STORE} pqxx)
