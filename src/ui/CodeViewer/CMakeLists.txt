cmake_minimum_required(VERSION 3.16)
project(CodeViewer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(CodeViewer
    main.cpp
    SyntaxHighlighter.h
    SyntaxHighlighter.cpp
    CodeViewer.h
    CodeViewer.cpp
)

target_link_libraries(CodeViewer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

set_target_properties(CodeViewer PROPERTIES
    WIN32_EXECUTABLE TRUE
)
