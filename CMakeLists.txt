cmake_minimum_required(VERSION 3.0)

project(mymuduo C CXX)

include_directories(${PROJECT_SOURCE_DIR}/easylogging++)
include_directories(${PROJECT_SOURCE_DIR}/)

aux_source_directory(${PROJECT_SOURCE_DIR}/ SRC_DIR)
aux_source_directory(${PROJECT_SOURCE_DIR}/easylogging++/ SRC_LOG)

set(CXX_FLAGS -g -Wall -std=c++11)

add_library(muduo_library SHARED ${SRC_DIR} ${SRC_LOG})
add_library(muduo_library_static STATIC ${SRC_DIR} ${SRC_LOG})

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
# set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
# target_include_directories(muduo_library AFTER PRIVATE ${PROJECT_SOURCE_DIR}/lib/include)


