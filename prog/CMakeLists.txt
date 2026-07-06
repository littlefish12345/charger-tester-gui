cmake_minimum_required(VERSION 3.20)

if(APPLE AND NOT DEFINED CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architecture for bundled ElaWidgetTools")
endif()

project(ChargerTester VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

if(APPLE AND EXISTS "$ENV{HOME}/Qt/6.6.2/macos")
    list(PREPEND CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.6.2/macos")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets SerialPort)

add_subdirectory(src)
