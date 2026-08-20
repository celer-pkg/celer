# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## tst_compiled Test:
#####################################################################

if(WASM)
    return()
endif()

qt_internal_add_test(tst_compiled
    SOURCES
        tst_compiled.cpp
    LIBRARIES
        Qt::Gui
        Qt::Qml
        Qt::Scxml
)

# Resources:
set(tst_compiled_resource_files
    "submachineA.scxml"
    "submachineB.scxml"
    "topmachine.scxml"
)

qt_internal_add_resource(tst_compiled "tst_compiled"
    PREFIX
        "/"
    FILES
        ${tst_compiled_resource_files}
)


# Statecharts:
qt6_add_statecharts(tst_compiled
    ids1.scxml
    eventnames1.scxml
    eventnames2.scxml
    statemachineunicodename.scxml
    anonymousstate.scxml
    submachineunicodename.scxml
    datainnulldatamodel.scxml
    initialhistory.scxml
    connection.scxml
    topmachine.scxml
    historyState.scxml
)

#### Keys ignored in scope 1:.:.:compiled.pro:<TRUE>:
# TEMPLATE = "app"
