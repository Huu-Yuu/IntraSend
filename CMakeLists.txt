cmake_minimum_required(VERSION 3.20)
project(IntraSend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Xml Svg SvgWidgets Network WebSockets)
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

# UI 文件 - 包含 .ui 文件
file(GLOB_RECURSE UI_HEADERS
    "src/new_ui/*.h"
    "src/new_ui/*.hpp"
)
file(GLOB_RECURSE UI_SOURCES
    "src/new_ui/*.cpp"
)
file(GLOB_RECURSE UI_FORMS
    "src/new_ui/*.ui"
)

# FancyUI 文件 - 排除 .qrc 文件
file(GLOB_RECURSE FANCYUI_HEADERS
    "src/QWidget-FancyUI/Fancy/*.h"
)
file(GLOB_RECURSE FANCYUI_SOURCES
    "src/QWidget-FancyUI/Fancy/*.cpp"
)

# 创建可执行文件
qt_add_executable(${PROJECT_NAME}
    WIN32
    MACOSX_BUNDLE
    src/main.cpp
    ${CORE_HEADERS}
    ${CORE_SOURCES}
    ${UI_HEADERS}
    ${UI_SOURCES}
    ${UI_FORMS}
    ${FANCYUI_HEADERS}
    ${FANCYUI_SOURCES}
)
# 添加所有资源文件
qt_add_resources(${PROJECT_NAME} "resources"
    FILES
        # 第一个资源文件（原来在 FancyUI_SOURCES 中的）
        "src/QWidget-FancyUI/resource/FancyResource.qrc"
        # 第二个资源文件
        "qss.qrc"
)


# 添加头文件搜索路径
target_include_directories(${PROJECT_NAME} PRIVATE
    # 这会让 src/ 成为根目录，所有子目录都会被搜索
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/QWidget-FancyUI/Fancy
    ${CMAKE_CURRENT_SOURCE_DIR}/src/QWidget-FancyUI/include/magic_enum
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Xml
    Qt6::Svg
    Qt6::SvgWidgets
    Qt6::Network
    Qt6::WebSockets
)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            user32.lib
            dwmapi.lib
            advapi32.lib
    )
endif()

# 可选：添加 MOC 自动处理
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# 为 Windows 设置图标
if (WIN32)
    set(WIN32_ICON "${CMAKE_CURRENT_SOURCE_DIR}/app.ico")
    set_target_properties(${PROJECT_NAME} PROPERTIES RC_ICONS "${WIN32_ICON}")
endif()
