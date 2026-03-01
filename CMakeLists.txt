cmake_minimum_required(VERSION 3.20)
project(IntraSend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Xml Svg SvgWidgets Network WebSockets)
qt_standard_project_setup()

# 设置库文件搜索路径
set(QFANCYUI_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(QFANCYUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/QWidget-FancyUI/Fancy)

# 检查库文件是否存在
if (EXISTS ${QFANCYUI_LIB_DIR}/libQFancyUI.a AND EXISTS ${QFANCYUI_LIB_DIR}/QFancyUI.dll)
    message(STATUS "Found QFancyUI library files")
else()
    message(FATAL_ERROR "QFancyUI library files not found in ${QFANCYUI_LIB_DIR}")
endif()

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
)

# 添加资源文件
qt_add_resources(${PROJECT_NAME} "resources"
    FILES
    "images.qrc"
    # "qss.qrc"
)

# 添加头文件搜索路径
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${QFANCYUI_INCLUDE_DIR}  # QFancyUI 头文件路径
    ${CMAKE_CURRENT_SOURCE_DIR}/src/QWidget-FancyUI/include/magic_enum
)

# 添加库文件
add_library(QFancyUI SHARED IMPORTED)

# 设置导入库的属性
set_target_properties(QFancyUI PROPERTIES
    IMPORTED_LOCATION ${QFANCYUI_LIB_DIR}/QFancyUI.dll
    IMPORTED_IMPLIB ${QFANCYUI_LIB_DIR}/libQFancyUI.a
    INTERFACE_INCLUDE_DIRECTORIES ${QFANCYUI_INCLUDE_DIR}
)

# Windows 特定：添加 DLL 复制步骤
if (WIN32)
    # 在构建后复制 DLL 到输出目录
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QFANCYUI_LIB_DIR}/QFancyUId.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
        COMMENT "Copying QFancyUI DLL to output directory"
    )
endif()

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Xml
    Qt6::Svg
    Qt6::SvgWidgets
    Qt6::Network
    Qt6::WebSockets
    QFancyUI  # 链接 QFancyUI 库
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

# 设置可执行文件输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
