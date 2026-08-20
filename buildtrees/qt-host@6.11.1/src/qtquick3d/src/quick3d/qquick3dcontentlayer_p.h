// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DCONTENTLAYER_P_H
#define QQUICK3DCONTENTLAYER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qtquick3dglobal_p.h"

#include <QtCore/qobject.h>

#include <QtQmlIntegration/qqmlintegration.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QQuick3DContentLayer : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ContentLayer)
    QML_ADDED_IN_VERSION(6, 11)
    QML_SINGLETON
public:
    explicit QQuick3DContentLayer(QObject *parent = nullptr);
    ~QQuick3DContentLayer() override;

    enum LayerFlag : quint32 {
        LayerNone = 0x0,
        Layer0 = 0x1, // Layer0 is reserved for the main layer.
        Layer1 = 0x2,
        Layer2 = 0x4,
        Layer3 = 0x8,
        Layer4 = 0x10,
        Layer5 = 0x20,
        Layer6 = 0x40,
        Layer7 = 0x80,
        Layer8 = 0x100,
        Layer9 = 0x200,
        Layer10 = 0x400,
        Layer11 = 0x800,
        Layer12 = 0x1000,
        Layer13 = 0x2000,
        Layer14 = 0x4000,
        Layer15 = 0x8000,
        Layer16 = 0x10000,
        Layer17 = 0x20000,
        Layer18 = 0x40000,
        Layer19 = 0x80000,
        Layer20 = 0x100000,
        Layer21 = 0x200000,
        Layer22 = 0x400000,
        Layer23 = 0x800000,
        // All layers from Layer1 to Layer23 are reserved for user-defined layers.
        LayerAll = 0xFFFFFF,
        // Layers 24 and above are reserved for internal usage.
        Layer24 = 0x1000000,
        Layer25 = 0x2000000,
        Layer26 = 0x4000000,
        Layer27 = 0x8000000,
        Layer28 = 0x10000000,
        Layer29 = 0x20000000,
        Layer30 = 0x40000000,
        Layer31 = 0x80000000,
        ReservedLayerMask = 0xFF000000
    };
    Q_DECLARE_FLAGS(LayerFlags, LayerFlag)
    Q_FLAG(LayerFlags)
};

QT_END_NAMESPACE

#endif // QQUICK3DCONTENTLAYER_P_H
