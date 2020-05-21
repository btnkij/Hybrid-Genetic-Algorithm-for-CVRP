
cmake_minimum_required (VERSION 3.8)

project ("VehicleRouting")

set(SRC "${CMAKE_SOURCE_DIR}/src")

add_executable (VehicleRouting  
"${SRC}/ga.h"  
"${SRC}/ga.cpp" 
"${SRC}/runner.cpp" 
"${SRC}/thread_pool.h" 
"${SRC}/problem.h" 
"${SRC}/problem.cpp" 
"${SRC}/util.h"  
"${SRC}/cws.h" 
"${SRC}/loss_metrics.h" 
"${SRC}/genetic_coding.h" 
"${SRC}/cws.cpp")
