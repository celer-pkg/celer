# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_plugin(QGeoPositionInfoSourceFactoryWasmPlugin
    OUTPUT_NAME qtposition_wasm
    CLASS_NAME QGeoPositionInfoSourceFactoryWasm
    PLUGIN_TYPE position
    SOURCES
        qgeopositioninfosource_wasm.cpp qgeopositioninfosource_wasm_p.h
        qgeopositioninfosourcefactory_wasm.cpp qgeopositioninfosourcefactory_wasm.h
        qgeopositioninfosourcedispatcher_wasm.cpp qgeopositioninfosourcedispatcher_wasm_p.h
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Positioning
)
