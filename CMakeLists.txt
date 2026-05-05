cmake_minimum_required(VERSION 3.16)

# ---------------- 基础 ----------------
get_filename_component(PROJECT_FOLDER_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
set(APP_NAME ${PROJECT_FOLDER_NAME})

project(${APP_NAME}
        VERSION 1.0.0
        LANGUAGES CXX
)

# ---------------- 版本信息 ----------------
set(VERSION_SUFFIX "" CACHE STRING "版本后缀 (beta/rc等)")

if (VERSION_SUFFIX)
    set(PROJECT_FULL_VERSION "${PROJECT_VERSION}-${VERSION_SUFFIX}")
else ()
    set(PROJECT_FULL_VERSION "${PROJECT_VERSION}")
endif ()

# ---------------- 构建信息 ----------------
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")

# ---------------- Git 信息 ----------------
find_package(Git QUIET)

set(GIT_HASH "unknown")
set(GIT_BRANCH "unknown")

if (GIT_FOUND)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )

    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )
endif ()

# ---------------- Qt6 配置 ----------------
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
        Core
        Gui
        Widgets
        PrintSupport
)

# ---------------- C++ 标准 ----------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------- 源文件 ----------------
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
        src/*.cpp
        src/*.h
        src/*.ui
        src/*.qrc
)

# IDE 目录结构
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_FILES})

# ---------------- 可执行程序 ----------------
add_executable(csvReader ${SRC_FILES}
        src/widget/MainWindow.cpp
        src/widget/MainWindow.h
        src/widget/MainWindow.ui
        src/utils/qcustomplot.cpp
        src/utils/qcustomplot.h
        main.cpp)

# ---------------- 链接 Qt ----------------
target_link_libraries(csvReader PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::PrintSupport
)

# ---------------- include 路径 ----------------
target_include_directories(csvReader PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
)

# ---------------- 编译选项 ----------------
target_compile_options(csvReader PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:
        -Wall -Wextra -Wpedantic
        >
)

# ---------------- 平台配置 ----------------
if (WIN32)
    target_compile_definitions(csvReader PRIVATE
            WIN32_LEAN_AND_MEAN
            _WIN32_WINNT=0x0A00
    )

    # Debug有控制台，Release无控制台
    set_target_properties(csvReader PROPERTIES
            WIN32_EXECUTABLE $<CONFIG:Release>
    )
endif ()

# ---------------- 构建信息注入 ----------------
target_compile_definitions(csvReader PRIVATE
        BUILD_GIT_HASH="${GIT_HASH}"
        BUILD_GIT_BRANCH="${GIT_BRANCH}"
        BUILD_TIMESTAMP="${BUILD_TIMESTAMP}"
        PROJECT_FULL_VERSION="${PROJECT_FULL_VERSION}"
)

# ---------------- 自动创建目录 ----------------
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(SUBDIRS
        "${SRC_DIR}/resource"
        "${SRC_DIR}/utils"
        "${SRC_DIR}/widget"
)

foreach (dir ${SUBDIRS})
    if (NOT EXISTS ${dir})
        file(MAKE_DIRECTORY ${dir})
    endif ()
endforeach ()