// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default



#ifndef QQUICK3DTEXTUREDATA_P_H
#define QQUICK3DTEXTUREDATA_P_H

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

#include <QtCore/QSize>

#include <QtQuick3D/QQuick3DTextureData>
#include <QtQuick3D/private/qquick3dobject_p.h>

QT_BEGIN_NAMESPACE

class QQuick3DTextureDataPrivate : public QQuick3DObjectPrivate
{
public:
    QQuick3DTextureDataPrivate();

    QByteArray textureData;
    QSize size;
    int depth = 0;
    QQuick3DTextureData::Format format = QQuick3DTextureData::RGBA8;
    bool hasTransparency = false;
    bool textureDataDirty = false;
};

QT_END_NAMESPACE

#endif // QQUICK3DTEXTUREDATA_P_H
