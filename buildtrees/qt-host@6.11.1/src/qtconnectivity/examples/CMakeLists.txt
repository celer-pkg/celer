# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_examples_build_begin(EXTERNAL_BUILD)

if(TARGET Qt6::Bluetooth)
    add_subdirectory(bluetooth)
endif()
if(TARGET Qt6::Nfc)
    add_subdirectory(nfc)
endif()

qt_examples_build_end()
