project(sftpplugpgsql)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_FLAGS 
${CMAKE_CXX_FLAGS} 
"-Wall -pedantic -D_GLIBCXX_CONCEPT_CHECKS -std=gnu++14 -pthread")

add_subdirectory(./src)


