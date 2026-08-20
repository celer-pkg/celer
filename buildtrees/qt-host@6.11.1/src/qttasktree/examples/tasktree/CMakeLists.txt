# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_internal_add_example(demo)
qt_internal_add_example(trafficlight)
if(QT_FEATURE_concurrent AND QT_FEATURE_network)
    qt_internal_add_example(imagescaling)
endif()
