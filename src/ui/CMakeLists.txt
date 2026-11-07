cmake_minimum_required(VERSION 3.19)
project(NoteFlow LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets WebSockets PrintSupport)

qt_standard_project_setup()

qt_add_executable(NoteFlow
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    channelitemwidget.cpp
    channelitemwidget.h
    chatview.cpp
    chatview.h
    connectdialog.cpp
    connectdialog.h
    Message.h
    messagebubble.cpp
    messagebubble.h
    wsclient.cpp
    wsclient.h
    fileEditor.cpp
    fileEditor.h
    fileitemwidget.h
    fileitemwidget.cpp
)

# QScintilla: pass -DQSCINTILLA_ROOT=<QScintilla_src-x.y.z> if it isn't installed into Qt
set(QSCINTILLA_ROOT "" CACHE PATH "Path to a built QScintilla source tree")
find_path(QSCINTILLA_INCLUDE_DIR Qsci/qsciscintilla.h
    HINTS "${QSCINTILLA_ROOT}/src" "${QT6_INSTALL_PREFIX}/include")
find_library(QSCINTILLA_LIBRARY
    NAMES libqscintilla2_qt6.a qscintilla2_qt6
    HINTS "${QSCINTILLA_ROOT}/src" "${QT6_INSTALL_PREFIX}/lib"
    PATH_SUFFIXES release)
if(NOT QSCINTILLA_INCLUDE_DIR OR NOT QSCINTILLA_LIBRARY)
    message(FATAL_ERROR "QScintilla not found. Set QSCINTILLA_ROOT or QSCINTILLA_LIBRARY/QSCINTILLA_INCLUDE_DIR.")
endif()

add_library(QScintilla::QScintilla UNKNOWN IMPORTED)
set_target_properties(QScintilla::QScintilla PROPERTIES
    IMPORTED_LOCATION "${QSCINTILLA_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${QSCINTILLA_INCLUDE_DIR}"
    INTERFACE_LINK_LIBRARIES "Qt6::Widgets;Qt6::PrintSupport")

target_link_libraries(NoteFlow
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt6::Gui
        Qt6::PrintSupport
        Qt6::WebSockets
        QScintilla::QScintilla
)

install(TARGETS NoteFlow
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET NoteFlow
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
