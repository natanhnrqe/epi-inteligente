cmake_minimum_required(VERSION 3.20)

project(epi_simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(epi_simulator
        src/main.cpp
)

if (WIN32)
    target_link_libraries(epi_simulator PRIVATE ws2_32)
endif()