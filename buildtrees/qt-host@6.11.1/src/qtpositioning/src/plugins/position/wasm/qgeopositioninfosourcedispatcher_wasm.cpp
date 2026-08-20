// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeopositioninfosourcedispatcher_wasm_p.h"
#include "qgeopositioninfosource_wasm_p.h"


QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QGeoPositionInfoDispatcherWasm, g_dispatcherWasm)

QGeoPositionInfoDispatcherWasm::QGeoPositionInfoDispatcherWasm()
{
}

QGeoPositionInfoDispatcherWasm::~QGeoPositionInfoDispatcherWasm()
{
    m_instanceRegistry.clear();
}

quint32 QGeoPositionInfoDispatcherWasm::incrementIdOfRegisterSourceInfo()
{
    static quint32 counter = 0;
    counter++;
    return counter;
}

int QGeoPositionInfoDispatcherWasm::registerInfoSourceInstance(QGeoPositionInfoSourceWasm* source)
{
    std::lock_guard<std::mutex> lock(registryMutex);
    quint32 id = incrementIdOfRegisterSourceInfo();
    m_instanceRegistry[id] = source;
    return id;
}

void QGeoPositionInfoDispatcherWasm:: unregisterInfoSourceInstance(int instanceId)
{
    std::lock_guard<std::mutex> lock(registryMutex);
    m_instanceRegistry.erase(instanceId);
}

QGeoPositionInfoSourceWasm* QGeoPositionInfoDispatcherWasm::lookUpInfoSource(int instanceId)
{
    std::lock_guard<std::mutex> lock(registryMutex);
    auto it = m_instanceRegistry.find(instanceId);
    if (it != m_instanceRegistry.end())
        return it->second;

    return nullptr;
}

QGeoPositionInfoDispatcherWasm* QGeoPositionInfoDispatcherWasm::instance()
{
    return g_dispatcherWasm();
}

void QGeoPositionInfoDispatcherWasm::geolocationSuccess(int instanceId, val position)
{
    if (auto source = lookUpInfoSource(instanceId))
        source->geolocationSuccess(position);
}

void QGeoPositionInfoDispatcherWasm::geolocationError(int instanceId, val error)
{
    if (auto source = lookUpInfoSource(instanceId))
        source->geolocationError(error);
}

void QGeoPositionInfoDispatcherWasm::log(val value)
{
    val::global("console").call<void>("log", value);
}

EMSCRIPTEN_BINDINGS(qt_geolocation)
{
    function("qtGeolocationPositionSuccess", optional_override([](int id, val pos) {
        QGeoPositionInfoDispatcherWasm::instance()->geolocationSuccess(id, pos);
    }));

    function("qtGeolocationPositionError", optional_override([](int id, val err) {
        QGeoPositionInfoDispatcherWasm::instance()->geolocationError(id, err);
    }));

}

QT_END_NAMESPACE
