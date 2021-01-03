cmake_minimum_required(VERSION 3.13)
project(grib2dec)

add_library(grib2dec SHARED)

target_include_directories(grib2dec INTERFACE
    ../include
)

include_directories(../include)

target_sources(grib2dec
    PRIVATE
        data.cpp
        read_grib2.cpp
        sections.cpp
)

target_compile_options(grib2dec PRIVATE -Wall)

target_link_libraries(grib2dec)
