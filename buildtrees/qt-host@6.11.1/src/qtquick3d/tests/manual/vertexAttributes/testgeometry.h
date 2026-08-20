// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TESTGEOMETRY_H
#define TESTGEOMETRY_H

#include <QtQuick3D/qquick3d.h>
#include <QtQuick3D/qquick3dgeometry.h>

class TestGeometry : public QQuick3DGeometry
{
    Q_OBJECT
    Q_PROPERTY(bool position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(bool normal READ normal WRITE setNormal NOTIFY normalChanged)
    Q_PROPERTY(bool texcoord0 READ texcoord0 WRITE setTexcoord0 NOTIFY texcoord0Changed)
    Q_PROPERTY(bool texcoord1 READ texcoord1 WRITE setTexcoord1 NOTIFY texcoord1Changed)
    Q_PROPERTY(bool tangent READ tangent WRITE setTangent NOTIFY tangentChanged)
    Q_PROPERTY(bool binormal READ binormal WRITE setBinormal NOTIFY binormalChanged)
    Q_PROPERTY(bool color READ color WRITE setColor NOTIFY colorChanged)

public:
    TestGeometry(QQuick3DObject *parent = nullptr);

    bool position() const;
    bool normal() const;
    bool texcoord0() const;
    bool texcoord1() const;
    bool tangent() const;
    bool binormal() const;
    bool color() const;

public Q_SLOTS:
    void setPosition(bool enable);
    void setNormal(bool enable);
    void setTexcoord0(bool enable);
    void setTexcoord1(bool enable);
    void setTangent(bool enable);
    void setBinormal(bool enable);
    void setColor(bool enable);

Q_SIGNALS:
    void positionChanged();
    void normalChanged();
    void texcoord0Changed();
    void texcoord1Changed();
    void tangentChanged();
    void binormalChanged();
    void colorChanged();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;

private:
    void updateId();

    bool m_position = false;
    bool m_normal = false;
    bool m_texcoord0 = false;
    bool m_texcoord1 = false;
    bool m_tangent = false;
    bool m_binormal = false;
    bool m_color = false;
    bool m_dirty = true;

    QByteArray m_id;
    QByteArray m_vertexBuffer;
    QByteArray m_indexBuffer;
};

#endif // TESTGEOMETRY_H
