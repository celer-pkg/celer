// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOPOSITIONDISPATCHER_WASM_H
#define QGEOPOSITIONDISPATCHER_WASM_H

#include <QObject>
#include <QGlobalStatic>
#include <unordered_map>
#include <mutex>

#include <emscripten/val.h>
#include <emscripten/bind.h>


QT_BEGIN_NAMESPACE

using namespace emscripten;

class QGeoPositionInfoSourceWasm;

class QGeoPositionInfoDispatcherWasm
{
    public:
        QGeoPositionInfoDispatcherWasm();
        ~QGeoPositionInfoDispatcherWasm();

        quint32 incrementIdOfRegisterSourceInfo();
        int registerInfoSourceInstance(QGeoPositionInfoSourceWasm* source);
        void unregisterInfoSourceInstance(int instanceId);
        QGeoPositionInfoSourceWasm* lookUpInfoSource(int instanceId);

        static QGeoPositionInfoDispatcherWasm* instance();

        void geolocationSuccess(int instanceId, val position);
        void geolocationError(int instanceId, val error);
        void log(val value);

    private:
        std::unordered_map<int, QGeoPositionInfoSourceWasm*> m_instanceRegistry;
        std::mutex registryMutex;
        Q_DISABLE_COPY(QGeoPositionInfoDispatcherWasm)
};

QT_END_NAMESPACE

#endif
