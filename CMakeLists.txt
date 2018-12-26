cmake_minimum_required(VERSION 2.10)
project(thread_demo)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(MASTER_PATH ${CMAKE_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${MASTER_PATH}/cmake/")

find_package(Glog REQUIRED)

find_package(Boost
            COMPONENTS filesystem system
            REQUIRED
            )

include_directories(include)
include_directories(${GLOG_INCLUDE_DIRS})

link_libraries(${GLOG_LIBRARIES}
            ${Boost_LIBRARIES})

add_executable(thread_demo main.cpp serial-lib.cpp)

