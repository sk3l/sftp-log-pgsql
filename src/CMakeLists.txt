cmake_minimum_required(VERSION 3.1)

set (CMAKE_CXX_STANDARD 14)              
set_property(TARGET ${TARGET_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

set(SOURCES sftp-pgsql-plugin.cpp)
set(INCLUDES sftp-plugin.h sftp-callback.h)

#include_directories(${CMAKE_SOURCE_DIR})
link_directories(/usr/lib/x86_64-linux-gnu)

message("SOURCES => ${SOURCES}")
message("INCLUDES => ${INCLUDES}")

set(LIBNAME ${PROJECT_NAME})
set(LIBNAME_S "${PROJECT_NAME}_s")

#target_link_libraries (${LIBNAME_S} ${CMAKE_THREAD_LIBS_INIT})
add_library(${LIBNAME_S} STATIC ${SOURCES})
target_link_libraries(${LIBNAME_S} pqxx)

#target_link_libraries (${LIBNAME} ${CMAKE_THREAD_LIBS_INIT})
add_library(${LIBNAME} SHARED ${SOURCES} ${INCLUDES})
target_link_libraries(${LIBNAME} pqxx)
