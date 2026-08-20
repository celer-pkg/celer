# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(TARGET Qt::Bluetooth)
    add_subdirectory(qlowenergycontroller)
    add_subdirectory(qlowenergycontroller_peripheral)
    add_subdirectory(examples/btscanner)
endif()

