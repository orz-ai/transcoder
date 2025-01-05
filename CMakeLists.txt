cmake_minimum_required(VERSION 3.24)
project(transcoder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets Sql Network REQUIRED)

# FIXME change to your local's CMAKE_PREFIX_PATH
set(CMAKE_PREFIX_PATH "D:\\Qt\\5.15.2\\msvc2019_64")

# Include current directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Automatically collect all source and header files
file(GLOB_RECURSE SOURCES "*.cpp" "*.h")

# exclude generated files
file(GLOB_RECURSE EXCLUDED_FILES "${CMAKE_BINARY_DIR}/**/*")
list(REMOVE_ITEM SOURCES ${EXCLUDED_FILES})

add_executable(transcoder ${SOURCES})

target_link_libraries(transcoder Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Sql Qt5::Network)
