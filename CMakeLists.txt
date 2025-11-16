cmake_minimum_required(VERSION 3.20)
project(transcoder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets Network REQUIRED)

# FIXME change to your local's CMAKE_PREFIX_PATH
set(CMAKE_PREFIX_PATH "D:\\Qt\\5.15.2\\msvc2019_64")

# Include current directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 手动列出源文件，避免包含自动生成的MOC文件
set(SOURCES
    main.cpp
    transcoder.cpp
    transcodetask.cpp
    transcodetaskmanager.cpp
    renamedialog.cpp
    settingdialog.cpp
    selecteddirsdialog.cpp
    configmanager.cpp
    utils/httpclient.cpp
    utils/ffmpegutils.cpp
)

set(HEADERS
    transcoder.h
    transcodetask.h
    transcodetaskmanager.h
    renamedialog.h
    settingdialog.h
    selecteddirsdialog.h
    configmanager.h
    utils/httpclient.h
    utils/ffmpegutils.h
)

set(UI_FILES
    transcoder.ui
    renamedialog.ui
    settingdialog.ui
    selecteddirsdialog.ui
)

set(RESOURCES
    resources.qrc
)

add_executable(transcoder ${SOURCES} ${HEADERS} ${UI_FILES} ${RESOURCES})

target_link_libraries(transcoder Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network)
