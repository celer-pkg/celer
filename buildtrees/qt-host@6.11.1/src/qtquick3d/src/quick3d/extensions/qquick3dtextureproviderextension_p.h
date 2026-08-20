// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DTEXTUREPROVIDEREXTENSION_P_H
#define QQUICK3DTEXTUREPROVIDEREXTENSION_P_H

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

#include <QtQuick3D/QQuick3DTextureProviderExtension>
#include <QtQuick3D/private/qquick3dobject_p.h>

QT_BEGIN_NAMESPACE

class QQuick3DTextureProviderExtensionPrivate : public QQuick3DObjectPrivate
{
public:
    QQuick3DTextureProviderExtensionPrivate();

    QQuick3DTextureProviderExtension::SamplerHint samplerHint = QQuick3DTextureProviderExtension::SamplerHint::Sampler2D;
};

QT_END_NAMESPACE

#endif // QQUICK3DTEXTUREPROVIDEREXTENSION_P_H
