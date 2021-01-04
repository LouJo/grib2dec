cmake_minimum_required(VERSION 3.13)
project(grib2dec_apps)

add_executable(grib2dec-bin)

target_link_libraries(grib2dec-bin PUBLIC grib2dec)

target_sources(grib2dec-bin
    PRIVATE
        grib2dec.cpp
        output/output.cpp
        output/txt.cpp
)

set_target_properties(grib2dec-bin
    PROPERTIES OUTPUT_NAME grib2dec
)
