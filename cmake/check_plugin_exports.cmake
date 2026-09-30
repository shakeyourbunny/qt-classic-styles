# This file is part of the qt-classic-styles Project.
# License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
#

# Fails when a plugin exports our symbols: they can bind to same-named QtWidgets ones at load time.
# Usage: cmake -DNM=<nm> -DPLUGIN=<file> -P check_plugin_exports.cmake

execute_process(COMMAND "${NM}" -D --defined-only "${PLUGIN}"
                OUTPUT_VARIABLE symbols RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "nm failed on ${PLUGIN}")
endif()
if(NOT symbols MATCHES "qt_plugin_instance")
    message(FATAL_ERROR "${PLUGIN} does not export qt_plugin_instance")
endif()

set(ours "QStyleHelper|ClassicStyleHelper|N6photon|QMotifStyle|QCDEStyle|QPlastiqueStyle|QCleanlooksStyle|QPhotonStyle|HexString")
string(REGEX MATCHALL "[^\n]*(${ours})[^\n]*" leaked "${symbols}")
if(leaked)
    string(REPLACE ";" "\n" leaked "${leaked}")
    message(FATAL_ERROR "${PLUGIN} exports our own symbols:\n${leaked}")
endif()
