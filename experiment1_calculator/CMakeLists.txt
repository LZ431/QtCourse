cmake_minimum_required(VERSION 3.21)

project(KeyboardCalculator
    VERSION 1.0.0
    DESCRIPTION "Qt course experiment 1 keyboard calculator"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.9 REQUIRED COMPONENTS Widgets Test)

qt_standard_project_setup()

qt_add_executable(KeyboardCalculator
    main.cpp
    calculatorlogic.cpp
    calculatorlogic.h
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

include(CTest)

if(BUILD_TESTING)
    qt_add_executable(CalculatorTests
        tests/test_calculator.cpp
        calculatorlogic.cpp
        calculatorlogic.h
        calculatorwindow.cpp
        calculatorwindow.h
        calculatorwindow.ui
    )

    target_link_libraries(CalculatorTests PRIVATE
        Qt6::Widgets
        Qt6::Test
    )
    target_include_directories(CalculatorTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_target_properties(CalculatorTests PROPERTIES
        WIN32_EXECUTABLE FALSE
    )

    if(MINGW OR MSVC)
        target_compile_options(CalculatorTests PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wpedantic>
        )
    endif()

    add_test(NAME calculator_tests COMMAND CalculatorTests)
    set_tests_properties(calculator_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endif()

qt_add_executable(CaptureScreenshots
    tools/capture_screenshots.cpp
    calculatorwindow.cpp
    calculatorwindow.h
    calculatorwindow.ui
    calculatorlogic.cpp
    calculatorlogic.h
)

target_link_libraries(CaptureScreenshots PRIVATE Qt6::Widgets)
target_include_directories(CaptureScreenshots PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)


