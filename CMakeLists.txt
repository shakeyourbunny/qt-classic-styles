# This file is part of the qt-classic-styles Project.
# License: LGPL-2.1-only (inherited from Qt). Contact: qt-classic-styles-project@trinity2k.net
#

cmake_minimum_required(VERSION 3.24)
project(qt-classic-styles VERSION 0.1.0 LANGUAGES CXX)

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
install(TARGETS qmotifstyle DESTINATION ${QT6_INSTALL_PLUGINS}/styles)

qt_add_plugin(qplastiquestyle CLASS_NAME QPlastiqueStylePlugin)
target_sources(qplastiquestyle PRIVATE
    src/plastique/qplastiquestyle.cpp src/plastique/qplastiquestyle.h
    src/plastique/plugin.cpp
)
target_link_libraries(qplastiquestyle PRIVATE Qt6::Widgets classic-styles-shared)
target_include_directories(qplastiquestyle PRIVATE src/plastique)
install(TARGETS qplastiquestyle DESTINATION ${QT6_INSTALL_PLUGINS}/styles)

qt_add_plugin(qcleanlooksstyle CLASS_NAME QCleanlooksStylePlugin)
target_sources(qcleanlooksstyle PRIVATE
    src/cleanlooks/qcleanlooksstyle.cpp src/cleanlooks/qcleanlooksstyle.h
    src/cleanlooks/plugin.cpp
)
target_link_libraries(qcleanlooksstyle PRIVATE Qt6::Widgets classic-styles-shared)
target_include_directories(qcleanlooksstyle PRIVATE src/cleanlooks)
install(TARGETS qcleanlooksstyle DESTINATION ${QT6_INSTALL_PLUGINS}/styles)

add_library(photon-core STATIC
    src/photon/photonpainter.cpp src/photon/photonpainter.h
    src/photon/qphotonstyle.cpp src/photon/qphotonstyle.h
)
set_target_properties(photon-core PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(photon-core PUBLIC Qt6::Widgets)
target_include_directories(photon-core PUBLIC src/photon)
target_compile_options(photon-core PRIVATE -Wall -Wextra -Wpedantic)

qt_add_plugin(qphotonstyle CLASS_NAME QPhotonStylePlugin)
target_sources(qphotonstyle PRIVATE src/photon/plugin.cpp src/photon/photon.json)
target_link_libraries(qphotonstyle PRIVATE photon-core)
target_compile_options(qphotonstyle PRIVATE -Wall -Wextra -Wpedantic)
# Tests load the plugin through QT_PLUGIN_PATH=<build>/plugins.
set_target_properties(qphotonstyle PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/styles)
install(TARGETS qphotonstyle DESTINATION ${QT6_INSTALL_PLUGINS}/styles)

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

    # Not a test: renders reference-shaped scenes for visual comparison.
    qt_add_executable(photon_showcase tests/photon/photon_showcase.cpp)
    target_link_libraries(photon_showcase PRIVATE photon-core Qt6::Test)
endif()
