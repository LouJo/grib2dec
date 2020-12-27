cmake_minimum_required(VERSION 3.13)
project(regatta_apps)

add_executable(rgta)

target_link_libraries(rgta PUBLIC regatta)

target_sources(rgta
    PRIVATE
        regatta.cpp
)
