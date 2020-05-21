# CMakeList.txt : CMake project for VehicleRouting, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("VehicleRouting")

# Add source to this project's executable.
add_executable (VehicleRouting  "ga.h"  "ga.cpp" "runner.cpp" "thread_pool.h" "problem.h" "problem.cpp" "util.h"  "cws.h" "loss_metrics.h" "genetic_coding.h" "cws.cpp")

# TODO: Add tests and install targets if needed.
