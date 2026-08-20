// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef RHIRENDERINGEXTENSIONS_H
#define RHIRENDERINGEXTENSIONS_H

#include <QtQuick3D/qquick3drenderextensions.h>
#include <QtQmlIntegration>
#include <rhi/qrhi.h>

class ConsumerRenderer;
class ProducerExtension;

class MyExtension : public QQuick3DRenderExtension
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(float x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(float y READ y WRITE setY NOTIFY yChanged)
    Q_PROPERTY(float z READ z WRITE setZ NOTIFY zChanged)

    Q_PROPERTY(float subSceneRotation READ subSceneRotation WRITE setSubSceneRotation NOTIFY subSceneRotationChanged)

public:
    MyExtension(QQuick3DObject *parent = nullptr);

    float x() const { return m_x; }
    void setX(float value)
    {
        if (m_x != value) {
            m_x = value;
            emit xChanged();
            markDirty(PositionDirty);
        }
    }

    float y() const { return m_y; }
    void setY(float value)
    {
        if (m_y != value) {
            m_y = value;
            emit yChanged();
            markDirty(PositionDirty);
        }
    }

    float z() const { return m_z; }
    void setZ(float value)
    {
        if (m_z != value) {
            m_z = value;
            emit zChanged();
            markDirty(PositionDirty);
        }
    }

    float subSceneRotation() const { return m_subSceneRotation; }
    void setSubSceneRotation(float value)
    {
        if (m_subSceneRotation != value) {
            m_subSceneRotation = value;
            emit subSceneRotationChanged();
            markDirty(SubSceneRotationDirty);
        }
    }

signals:
    void xChanged();
    void yChanged();
    void zChanged();
    void subSceneRotationChanged();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;

private:
    enum Dirty : quint8
    {
        PositionDirty = 1 << 0,
        SubSceneRotationDirty = 1 << 1
    };
    using DirtyT = std::underlying_type_t<Dirty>;
    void markDirty(Dirty v);
    DirtyT m_dirtyFlag = 0;

    float m_x = 0.0f;
    float m_y = 0.0f;
    float m_z = 0.0f;

    float m_subSceneRotation = 0.0f;

    ProducerExtension *m_producer = nullptr;
};

#endif // RHIRENDERINGEXTENSIONS_H
