cmake_minimum_required(VERSION 3.20)
project(IntraSend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

qt_add_executable(IntraSend
    WIN32 MACOSX_BUNDLE
    main.cpp
    widget.h
    widget.cpp
    widget.ui
    mymessagebox.h
    mymessagebox.cpp
    mymessagebox.ui
    appinit.h
    appinit.cpp
)

qt_add_resources(IntraSend "qss"
    PREFIX "/"
    FILES
        qss.qrc
)

target_link_libraries(IntraSend PRIVATE
    Qt::Core
    Qt::Widgets
)
