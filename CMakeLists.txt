cmake_minimum_required(VERSION 3.1.0)
project(c64pp VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Widgets Multimedia REQUIRED)

file(GLOB_RECURSE emu_src src/emu/*.cpp src/emu/*.h)
add_library(emu_lib OBJECT ${emu_src})
target_link_libraries(emu_lib PUBLIC Qt6::Widgets)

file(GLOB_RECURSE tools_src src/tools/*.cpp src/tools/*.h)
add_library(tools_lib OBJECT ${tools_src})
target_link_libraries(tools_lib PUBLIC emu_lib)

file(GLOB_RECURSE gui_src src/gui/*.cpp src/gui/*.h src/gui/*.ui)
file(GLOB_RECURSE gui_res data/*.qrc data/*.rc)
add_library(gui_lib ${gui_src})
target_link_libraries(gui_lib PUBLIC Qt6::Widgets Qt6::Multimedia)
set_property(TARGET gui_lib PROPERTY AUTOMOC ON)
set_property(TARGET gui_lib PROPERTY AUTOUIC ON)

add_executable(C64++ src/main.cpp ${gui_res} $<TARGET_OBJECTS:emu_lib> $<TARGET_OBJECTS:tools_lib>)
set_property(TARGET C64++ PROPERTY AUTORCC ON)
target_link_libraries(C64++ PUBLIC gui_lib)
