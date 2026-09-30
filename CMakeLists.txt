# This file is part of the qt-classic-styles Project.
# License: LGPL-2.1-only (inherited from Qt). Contact: qt-classic-styles-project@trinity2k.net
#

cmake_minimum_required(VERSION 3.24)
project(qt-classic-styles VERSION 0.1.0 LANGUAGES CXX)

# Release metadata next to the version: a release bumps these three lines together.
set(QT_CLASSIC_STYLES_PART_NUMBER_BASE "SJ-PKG-0010")
set(QT_CLASSIC_STYLES_RELEASE_DATE "20260910")
set(QT_CLASSIC_STYLES_PART_NUMBER "${QT_CLASSIC_STYLES_PART_NUMBER_BASE}-${QT_CLASSIC_STYLES_RELEASE_DATE}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

add_library(classic-styles-shared STATIC
    src/shared/qstylehelper.cpp
    src/shared/qstylehelper_p.h
    src/shared/qstylecache_p.h
    src/shared/qhexstring_p.h
)
# static lib is linked into shared-object plugins, so it must be PIC
set_target_properties(classic-styles-shared PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(classic-styles-shared PRIVATE Qt6::Widgets)
target_include_directories(classic-styles-shared PUBLIC src/shared)

qt_add_plugin(qmotifstyle CLASS_NAME QMotifStylePlugin)
target_sources(qmotifstyle PRIVATE
    src/motif/qmotifstyle.cpp src/motif/qmotifstyle.h
    src/motif/qcdestyle.cpp src/motif/qcdestyle.h
    src/motif/plugin.cpp
)
target_link_libraries(qmotifstyle PRIVATE Qt6::Widgets classic-styles-shared)
target_include_directories(qmotifstyle PRIVATE src/motif)
set_target_properties(qmotifstyle PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/styles)
install(TARGETS qmotifstyle DESTINATION ${QT6_INSTALL_PLUGINS}/styles)

qt_add_plugin(qplastiquestyle CLASS_NAME QPlastiqueStylePlugin)
target_sources(qplastiquestyle PRIVATE
    src/plastique/qplastiquestyle.cpp src/plastique/qplastiquestyle.h
    src/plastique/plugin.cpp
)
target_link_libraries(qplastiquestyle PRIVATE Qt6::Widgets classic-styles-shared)
target_include_directories(qplastiquestyle PRIVATE src/plastique)
set_target_properties(qplastiquestyle PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/styles)
install(TARGETS qplastiquestyle DESTINATION ${QT6_INSTALL_PLUGINS}/styles)

qt_add_plugin(qcleanlooksstyle CLASS_NAME QCleanlooksStylePlugin)
target_sources(qcleanlooksstyle PRIVATE
    src/cleanlooks/qcleanlooksstyle.cpp src/cleanlooks/qcleanlooksstyle.h
    src/cleanlooks/plugin.cpp
)
target_link_libraries(qcleanlooksstyle PRIVATE Qt6::Widgets classic-styles-shared)
target_include_directories(qcleanlooksstyle PRIVATE src/cleanlooks)
set_target_properties(qcleanlooksstyle PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/styles)
install(TARGETS qcleanlooksstyle DESTINATION ${QT6_INSTALL_PLUGINS}/styles)

add_library(photon-core STATIC
    src/photon/photonpainter.cpp src/photon/photonpainter.h
    src/photon/qphotonstyle.cpp src/photon/qphotonstyle.h
)
set_target_properties(photon-core PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(photon-core PUBLIC Qt6::Widgets PRIVATE classic-styles-shared)
target_include_directories(photon-core PUBLIC src/photon)
target_compile_options(photon-core PRIVATE -Wall -Wextra -Wpedantic)

qt_add_plugin(qphotonstyle CLASS_NAME QPhotonStylePlugin)
target_sources(qphotonstyle PRIVATE src/photon/plugin.cpp src/photon/photon.json)
target_link_libraries(qphotonstyle PRIVATE photon-core)
target_compile_options(qphotonstyle PRIVATE -Wall -Wextra -Wpedantic)
# Tests load the plugin through QT_PLUGIN_PATH=<build>/plugins.
set_target_properties(qphotonstyle PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/styles)
install(TARGETS qphotonstyle DESTINATION ${QT6_INSTALL_PLUGINS}/styles)

set(style_targets classic-styles-shared photon-core
    qmotifstyle qplastiquestyle qcleanlooksstyle qphotonstyle)
foreach(target ${style_targets})
    # A plugin exports only its Qt entry points.
    set_target_properties(${target} PROPERTIES
        CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)
endforeach()
foreach(target classic-styles-shared qmotifstyle qplastiquestyle qcleanlooksstyle)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
endforeach()

# Interactive widget gallery for visual testing. Loads styles via QStyleFactory
# at runtime; point QT_PLUGIN_PATH at the build directory to see all five.
configure_file(cmake/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/qtclassicstyles/version.h @ONLY)

add_library(widgetdemo-options STATIC
    src/widgetdemo/demooptions.cpp src/widgetdemo/demooptions.h)
target_link_libraries(widgetdemo-options PUBLIC Qt6::Core)
target_include_directories(widgetdemo-options PUBLIC
    src/widgetdemo ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_compile_options(widgetdemo-options PRIVATE -Wall -Wextra -Wpedantic)

qt_add_executable(widgetdemo src/widgetdemo/main.cpp)
target_link_libraries(widgetdemo PRIVATE Qt6::Widgets widgetdemo-options)
target_compile_options(widgetdemo PRIVATE -Wall -Wextra -Wpedantic)

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)

    qt_add_executable(tst_photonplugin tests/photon/tst_photonplugin.cpp)
    target_link_libraries(tst_photonplugin PRIVATE Qt6::Widgets Qt6::Test)
    add_test(NAME tst_photonplugin COMMAND tst_photonplugin)
    set_tests_properties(tst_photonplugin PROPERTIES ENVIRONMENT
        "QT_QPA_PLATFORM=offscreen;QT_PLUGIN_PATH=${CMAKE_BINARY_DIR}/plugins")

    qt_add_executable(tst_photonpainter tests/photon/tst_photonpainter.cpp)
    target_link_libraries(tst_photonpainter PRIVATE photon-core Qt6::Test)
    add_test(NAME tst_photonpainter COMMAND tst_photonpainter)
    set_tests_properties(tst_photonpainter PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    qt_add_executable(tst_photonstyle tests/photon/tst_photonstyle.cpp)
    target_link_libraries(tst_photonstyle PRIVATE photon-core Qt6::Test)
    add_test(NAME tst_photonstyle COMMAND tst_photonstyle)
    set_tests_properties(tst_photonstyle PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    qt_add_executable(tst_demooptions tests/widgetdemo/tst_demooptions.cpp)
    target_link_libraries(tst_demooptions PRIVATE widgetdemo-options Qt6::Test)
    add_test(NAME tst_demooptions COMMAND tst_demooptions)

    qt_add_executable(tst_widgetdemo_screenshot tests/widgetdemo/tst_widgetdemo_screenshot.cpp)
    target_link_libraries(tst_widgetdemo_screenshot PRIVATE Qt6::Widgets Qt6::Test)
    target_compile_definitions(tst_widgetdemo_screenshot PRIVATE
        WIDGETDEMO_PATH="$<TARGET_FILE:widgetdemo>")
    # Test-only style plugin that fails to create its style; never installed.
    qt_add_plugin(brokenstyleplugin CLASS_NAME BrokenStylePlugin)
    target_sources(brokenstyleplugin PRIVATE tests/widgetdemo/brokenstyleplugin.cpp
                                             tests/widgetdemo/brokenstyle.json)
    target_link_libraries(brokenstyleplugin PRIVATE Qt6::Widgets)
    set_target_properties(brokenstyleplugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/testplugins/styles)

    add_dependencies(tst_widgetdemo_screenshot widgetdemo brokenstyleplugin
        qmotifstyle qplastiquestyle qcleanlooksstyle qphotonstyle)
    add_test(NAME tst_widgetdemo_screenshot COMMAND tst_widgetdemo_screenshot)
    # path_list_append joins with the host's separator (';' on Windows).
    set_tests_properties(tst_widgetdemo_screenshot PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        ENVIRONMENT_MODIFICATION
            "QT_PLUGIN_PATH=set:${CMAKE_BINARY_DIR}/plugins;QT_PLUGIN_PATH=path_list_append:${CMAKE_BINARY_DIR}/testplugins")

    qt_add_executable(tst_stylehelper tests/shared/tst_stylehelper.cpp)
    target_link_libraries(tst_stylehelper PRIVATE classic-styles-shared Qt6::Widgets Qt6::Test)
    add_test(NAME tst_stylehelper COMMAND tst_stylehelper)
    set_tests_properties(tst_stylehelper PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    qt_add_executable(tst_portfixes tests/ports/tst_portfixes.cpp)
    target_link_libraries(tst_portfixes PRIVATE classic-styles-shared Qt6::Widgets Qt6::Test)
    add_dependencies(tst_portfixes qmotifstyle qplastiquestyle qcleanlooksstyle qphotonstyle)
    add_test(NAME tst_portfixes COMMAND tst_portfixes)
    set_tests_properties(tst_portfixes PROPERTIES ENVIRONMENT
        "QT_QPA_PLATFORM=offscreen;QT_PLUGIN_PATH=${CMAKE_BINARY_DIR}/plugins")

    qt_add_executable(tst_hostile_options tests/robustness/tst_hostile_options.cpp)
    target_link_libraries(tst_hostile_options PRIVATE Qt6::Widgets Qt6::Test)
    add_dependencies(tst_hostile_options qmotifstyle qplastiquestyle qcleanlooksstyle qphotonstyle)
    add_test(NAME tst_hostile_options COMMAND tst_hostile_options)
    set_tests_properties(tst_hostile_options PROPERTIES ENVIRONMENT
        "QT_QPA_PLATFORM=offscreen;QT_PLUGIN_PATH=${CMAKE_BINARY_DIR}/plugins")

    if(CMAKE_NM AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
        foreach(plugin qmotifstyle qplastiquestyle qcleanlooksstyle qphotonstyle)
            add_test(NAME exports_${plugin}
                     COMMAND ${CMAKE_COMMAND} -DNM=${CMAKE_NM}
                             -DPLUGIN=$<TARGET_FILE:${plugin}>
                             -P ${CMAKE_SOURCE_DIR}/cmake/check_plugin_exports.cmake)
        endforeach()
    endif()

    # Not a test: renders reference-shaped scenes for visual comparison.
    qt_add_executable(photon_showcase tests/photon/photon_showcase.cpp)
    target_link_libraries(photon_showcase PRIVATE photon-core Qt6::Test)
endif()
