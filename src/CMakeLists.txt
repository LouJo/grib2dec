cmake_minimum_required(VERSION 3.13)
project(regatta_lib)

add_library(regatta SHARED)

target_include_directories(regatta INTERFACE
    ../include
)

include_directories(../include)

target_sources(regatta
    PRIVATE
        read_grib2.cpp
)

target_link_libraries(regatta)
