// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef VOLUMETEXTUREDATA_H
#define VOLUMETEXTUREDATA_H

#include <QtQuick3D/QQuick3DTextureData>
#include <QtQml/QQmlEngine>

#include <QtGui/QColor>
#include <QtCore/QByteArray>

QT_BEGIN_NAMESPACE

class VolumeTextureData : public QQuick3DTextureData
{
    Q_OBJECT
    QML_ELEMENT

public:
    VolumeTextureData();
    ~VolumeTextureData();
};

QT_END_NAMESPACE

#endif // VOLUMETEXTUREDATA_H
