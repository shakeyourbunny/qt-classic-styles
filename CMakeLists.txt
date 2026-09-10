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
