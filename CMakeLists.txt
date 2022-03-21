cmake_minimum_required(VERSION 3.1.0)
project(c64pp VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Widgets Multimedia REQUIRED)


file(GLOB_RECURSE c64emu_src src/emu/*.cpp src/emu/*.h)
add_library(c64emu_lib ${c64emu_src})

file(GLOB_RECURSE c64emu_gui_src src/gui/*.cpp src/gui/*.h src/gui/*.ui)
file(GLOB_RECURSE c64emu_res data/*.qrc)

add_executable(c64pp src/main.cpp ${c64emu_gui_src} ${c64emu_res})
target_link_libraries(c64pp PUBLIC c64emu_lib Qt6::Widgets Qt6::Multimedia)

set_property(TARGET c64pp PROPERTY AUTOMOC ON)
set_property(TARGET c64pp PROPERTY AUTORCC ON)
set_property(TARGET c64pp PROPERTY AUTOUIC ON)
