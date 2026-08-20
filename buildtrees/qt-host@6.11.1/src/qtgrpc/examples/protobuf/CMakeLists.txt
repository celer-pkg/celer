# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

if(QT_FEATURE_qtprotobufgen AND TARGET Qt6::Network AND TARGET Qt6::Widgets)
    qt_internal_add_example(sensors)
endif()
