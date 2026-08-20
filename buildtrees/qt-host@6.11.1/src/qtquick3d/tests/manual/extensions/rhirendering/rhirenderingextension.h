// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef RHIRENDERINGEXTENSION_H
#define RHIRENDERINGEXTENSION_H

#include <QtQuick3D/qquick3drenderextensions.h>
#include <QtQmlIntegration>

class RhiRenderingExtension : public QQuick3DRenderExtension
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(float x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(float y READ y WRITE setY NOTIFY yChanged)
    Q_PROPERTY(float z READ z WRITE setZ NOTIFY zChanged)

    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)

public:
    enum Mode {
        Underlay,
        Overlay
    };
    Q_ENUM(Mode)

    Mode mode() const { return m_mode; }
    void setMode(Mode m)
    {
        if (m_mode != m) {
            m_mode = m;
            emit modeChanged();
            markDirty(ModeDirty);
        }
    }

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

signals:
    void xChanged();
    void yChanged();
    void zChanged();
    void modeChanged();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;

private:
    enum Dirty : quint8
    {
        PositionDirty = 1 << 0,
        ModeDirty = 1 << 1
    };
    using DirtyT = std::underlying_type_t<Dirty>;
    void markDirty(Dirty v);
    DirtyT m_dirtyFlag = 0;

    Mode m_mode = Overlay;
    float m_x = 0.0f;
    float m_y = 0.0f;
    float m_z = 0.0f;
};

#endif // RHIRENDERINGEXTENSION_H
