cmake_minimum_required(VERSION 3.19)

project(mymuduo C CXX)

include_directories(${PROJECT_SOURCE_DIR}/easylogging++)
include_directories(${PROJECT_SOURCE_DIR}/)

aux_source_directory(${PROJECT_SOURCE_DIR}/ SRC_DIR)
aux_source_directory(${PROJECT_SOURCE_DIR}/easylogging++/ SRC_LOG)

set(CXX_FLAGS -g -Wall -std=c++11)

add_library(muduo_library SHARED ${SRC_DIR} ${SRC_LOG})
add_library(muduo_library_static STATIC ${SRC_DIR} ${SRC_LOG})

SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/sbin) # 设置可执行文件存放位置，放在项目同级的bin下
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib) # 设置静态库存放位置
# target_include_directories(muduo_library AFTER PRIVATE ${PROJECT_SOURCE_DIR}/lib/include)

add_subdirectory(tests)
