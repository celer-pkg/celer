# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

# ![0]
find_package(Qt6 REQUIRED COMPONENTS Bluetooth)
target_link_libraries(mytarget PRIVATE Qt6::Bluetooth)
# ![0]
