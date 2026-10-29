cmake_minimum_required(VERSION 3.21)

project(SimpleTextEditor
    VERSION 1.0.0
    DESCRIPTION "Qt course experiment 2 simple text editor"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.9 REQUIRED COMPONENTS Widgets Test)

qt_standard_project_setup()

set(EDITOR_CORE_SOURCES
    codeeditor.cpp
    codeeditor.h
)

qt_add_executable(SimpleTextEditor
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    finddialog.cpp
    finddialog.h
    finddialog.ui
    replacedialog.cpp
    replacedialog.h
    replacedialog.ui
    aboutdialog.cpp
    aboutdialog.h
    aboutdialog.ui
    ${EDITOR_CORE_SOURCES}
    resources.qrc
)

target_link_libraries(SimpleTextEditor PRIVATE Qt6::Widgets)
target_include_directories(SimpleTextEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MINGW OR MSVC)
    target_compile_options(SimpleTextEditor PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wpedantic>
    )
endif()

set_target_properties(SimpleTextEditor PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

include(CTest)

if(BUILD_TESTING)
    qt_add_executable(TextEditorTests
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_editor.cpp
        codeeditor.cpp
        codeeditor.h
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        finddialog.cpp
        finddialog.h
        finddialog.ui
        replacedialog.cpp
        replacedialog.h
        replacedialog.ui
        aboutdialog.cpp
        aboutdialog.h
        aboutdialog.ui
        resources.qrc
    )

    target_link_libraries(TextEditorTests PRIVATE
        Qt6::Widgets
        Qt6::Test
    )
    target_include_directories(TextEditorTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_target_properties(TextEditorTests PROPERTIES
        WIN32_EXECUTABLE FALSE
    )

    if(MINGW OR MSVC)
        target_compile_options(TextEditorTests PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wpedantic>
        )
    endif()

    add_test(NAME text_editor_tests COMMAND TextEditorTests)
    set_tests_properties(text_editor_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        TIMEOUT 30
    )
endif()

qt_add_executable(CaptureEditorScreenshots
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/capture_screenshots.cpp
    codeeditor.cpp
    codeeditor.h
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    finddialog.cpp
    finddialog.h
    finddialog.ui
    replacedialog.cpp
    replacedialog.h
    replacedialog.ui
    aboutdialog.cpp
    aboutdialog.h
    aboutdialog.ui
    resources.qrc
)

target_link_libraries(CaptureEditorScreenshots PRIVATE Qt6::Widgets)
target_include_directories(CaptureEditorScreenshots PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

