// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGLIGHTMAPPER_H
#define QSSGLIGHTMAPPER_H

//
//  W A R N I N G
//  -------------
//
// This file is part of the QtQuick3D API, with limited compatibility guarantees.
// Usage of this API may make your code source and binary incompatible with
// future versions of Qt.
//

#include <QtQuick3DRuntimeRender/qtquick3druntimerenderexports.h>
#include <QString>

QT_BEGIN_NAMESPACE

struct QSSGLightmapperOptions
{
    float opacityThreshold = 0.5f;
    float bias = 0.005f;
    bool useAdaptiveBias = true;
    bool indirectLightEnabled = true;
    int indirectLightSamples = 256;
    int indirectLightWorkgroupSize = 32;
    int indirectLightBounces = 3;
    float indirectLightFactor = 1.0f;
    QString source = QStringLiteral("lightmaps.bin");
    float sigma = 8.f;
    float texelsPerUnit = 1.f;
};

QT_END_NAMESPACE

#endif // QSSGLIGHTMAPPER_H
