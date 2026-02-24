cmake_minimum_required(VERSION 3.20)
project(IntraSend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network WebSockets)
qt_standard_project_setup()

# 递归查找 src/core/ 目录下的所有 .h 和 .cpp 文件
file(GLOB_RECURSE CORE_HEADERS
    "src/core/*.h"
    "src/core/*.hpp"
)
file(GLOB_RECURSE CORE_SOURCES
    "src/core/*.cpp"
    "src/core/*.cxx"
)
file(GLOB_RECURSE UI_HEADERS
    "src/new_ui/*.h"
    "src/new_ui/*.hpp"
    "src/new_ui/*.ui"
)
file(GLOB_RECURSE UI_SOURCES
    "src/new_ui/*.cpp"
    "src/new_ui/*.cxx"
)


qt_add_executable(IntraSend
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/ui/widget.h
    src/ui/widget.cpp
    src/ui/widget.ui
    src/ui/mymessagebox.h
    src/ui/mymessagebox.cpp
    src/ui/mymessagebox.ui
    src/ui/appinit.h
    src/ui/appinit.cpp
    ${CORE_HEADERS}
    ${CORE_SOURCES}
    ${UI_HEADERS}
    ${UI_SOURCES}
)

# 添加头文件搜索路径
target_include_directories(IntraSend PRIVATE
    # 这会让 src/ 成为根目录，所有子目录都会被搜索
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/core

    ${OPENSSL_ROOT_DIR}  # OpenSSL 头文件目录
)

qt_add_resources(IntraSend "qss"
    PREFIX "/"
    FILES
        qss.qrc
)

target_link_libraries(IntraSend PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::Network
    Qt::WebSockets
)
