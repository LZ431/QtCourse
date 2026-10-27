cmake_minimum_required(VERSION 3.21)

project(KeyboardCalculator
    VERSION 1.0.0
    DESCRIPTION "Qt course experiment 1 keyboard calculator"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.9 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(KeyboardCalculator
    main.cpp
    calculatorwindow.cpp
    calculatorwindow.h
    calculatorwindow.ui
)

target_link_libraries(KeyboardCalculator PRIVATE Qt6::Widgets)

if(MINGW OR MSVC)
    target_compile_options(KeyboardCalculator PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wpedantic>
    )
endif()

set_target_properties(KeyboardCalculator PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

