cmake_minimum_required(VERSION 3.8)
project(counterword)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
file(WRITE "cmake-build-debug/config.txt" "infile=\"example.txt\"\nout_by_a=\"res_a.txt\"\nout_by_n=\"res_n.txt\"\nthreads=5")
add_executable(counterword ${SOURCE_FILES})