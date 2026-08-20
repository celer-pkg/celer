# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

if(TARGET Qt::Widgets)
    qt_internal_add_example(logfilepositionsource)
endif()
if(TARGET Qt::Quick AND TARGET Qt::Svg)
    qt_internal_add_example(satelliteinfo)
endif()
if(TARGET Qt::Network AND TARGET Qt::Quick)
    qt_internal_add_example(weatherinfo)
    set_target_properties(weatherinfo PROPERTIES UNITY_BUILD OFF)
endif()
