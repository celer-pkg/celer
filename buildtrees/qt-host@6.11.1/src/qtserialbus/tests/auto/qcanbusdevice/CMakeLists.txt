# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qcanbusdevice Test:
#####################################################################

qt_internal_add_test(tst_qcanbusdevice
    SOURCES
        tst_qcanbusdevice.cpp
    LIBRARIES
        Qt::Network
        Qt::SerialBus
)

# should be
# qt6_import_plugins(tst_qcanbusdevice INCLUDE TestCanBusPlugin)
# but qt6_import_plugins does not work here
target_link_libraries(tst_qcanbusdevice PRIVATE TestCanBusPlugin)
