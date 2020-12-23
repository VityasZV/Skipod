cmake_minimum_required(VERSION 3.6)
project(First)

find_package(MPI REQUIRED)
find_package(Boost 1.65.0 REQUIRED)
find_package(Boost 1.65.0 COMPONENTS system filesystem)
include_directories(${MPI_INCLUDE_PATH})
SET(CMAKE_C_COMPILER mpicc)
SET(CMAKE_CXX_COMPILER mpicxx)
set(CMAKE_CXX_STANDARD 17)


add_executable(First main.cpp)

target_include_directories(First PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(First PRIVATE ${Boost_LIBRARIES})