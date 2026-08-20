// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DRENDEREXTENSIONS_H
#define QQUICK3DRENDEREXTENSIONS_H

#include <QtCore/qobject.h>
#include <QtQuick3D/qquick3dobject.h>

QT_BEGIN_NAMESPACE

class QSSGRenderer;
class QSSGLayerRenderData;
class QQuick3DObjectPrivate;

class Q_QUICK3D_EXPORT QQuick3DRenderExtension : public QQuick3DObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(RenderExtension)
    QML_UNCREATABLE("RenderExtension is an abstract type")
    QML_ADDED_IN_VERSION(6, 6)
public:
    explicit QQuick3DRenderExtension(QQuick3DObject *parent = nullptr);
    virtual ~QQuick3DRenderExtension();

    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;

protected:
    explicit QQuick3DRenderExtension(QQuick3DObjectPrivate &dd, QQuick3DObject *parent = nullptr);
};

QT_END_NAMESPACE

#endif // QQUICK3DRENDEREXTENSIONS_H
