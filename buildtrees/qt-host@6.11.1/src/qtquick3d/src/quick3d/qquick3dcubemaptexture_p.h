// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DCUBEMAPTEXTURE_P_H
#define QQUICK3DCUBEMAPTEXTURE_P_H

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

#include <QtQuick3D/private/qquick3dtexture_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QQuick3DCubeMapTexture : public QQuick3DTexture
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CubeMapTexture)

public:
    explicit QQuick3DCubeMapTexture(QQuick3DObject *parent = nullptr);
    ~QQuick3DCubeMapTexture() override;
};

QT_END_NAMESPACE

#endif // QQUICK3DCUBEMAPTEXTURE_P_H
