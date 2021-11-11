cmake_minimum_required(VERSION 3.1.0)
project(c64pp VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

file(GLOB_RECURSE c64emu_src src/*.cpp src/*.h src/*.ui)

add_executable(c64pp ${c64emu_src})

target_link_libraries(c64pp PRIVATE Qt6::Widgets)