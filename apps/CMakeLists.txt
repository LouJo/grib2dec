cmake_minimum_required(VERSION 3.13)
project(regatta_apps)

add_executable(regatta-bin)

target_link_libraries(regatta-bin PUBLIC regatta)

target_sources(regatta-bin
    PRIVATE
        regatta.cpp
)

set_target_properties(regatta-bin
    PROPERTIES OUTPUT_NAME regatta
)
