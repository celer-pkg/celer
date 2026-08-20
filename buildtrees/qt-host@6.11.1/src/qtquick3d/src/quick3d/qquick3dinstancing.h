// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef Q_QUICK3D_INSTANCING_H
#define Q_QUICK3D_INSTANCING_H

#include <QtQuick3D/qquick3dobject.h>
#include <QtGui/QVector4D>
#include <QtGui/QQuaternion>
#include <QtGui/QColor>

QT_BEGIN_NAMESPACE

class QQuick3DInstancingPrivate;

class Q_QUICK3D_EXPORT QQuick3DInstancing : public QQuick3DObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QQuick3DInstancing)

    QML_NAMED_ELEMENT(Instancing)
    QML_UNCREATABLE("Instancing is Abstract")
    QML_ADDED_IN_VERSION(6, 2)
    Q_PROPERTY(int instanceCountOverride READ instanceCountOverride WRITE setInstanceCountOverride NOTIFY instanceCountOverrideChanged)
    Q_PROPERTY(bool hasTransparency READ hasTransparency WRITE setHasTransparency NOTIFY hasTransparencyChanged)
    Q_PROPERTY(bool depthSortingEnabled READ depthSortingEnabled WRITE setDepthSortingEnabled NOTIFY depthSortingEnabledChanged)
    Q_PROPERTY(QVector3D shadowBoundsMinimum READ shadowBoundsMinimum WRITE setShadowBoundsMinimum NOTIFY
                       shadowBoundsMinimumChanged REVISION(6, 9))
    Q_PROPERTY(QVector3D shadowBoundsMaximum READ shadowBoundsMaximum WRITE setShadowBoundsMaximum NOTIFY
                       shadowBoundsMaximumChanged REVISION(6, 9))

public:
    struct Q_QUICK3D_EXPORT InstanceTableEntry {
        QVector4D row0;
        QVector4D row1;
        QVector4D row2;
        QVector4D color;
        QVector4D instanceData;

        QVector3D getPosition() const;
        QVector3D getScale() const;
        QQuaternion getRotation() const;
        QColor getColor() const;
    };

    explicit QQuick3DInstancing(QQuick3DObject *parent = nullptr);
    ~QQuick3DInstancing() override;

    QByteArray instanceBuffer(int *instanceCount);
    int instanceCountOverride() const;
    bool hasTransparency() const;
    bool depthSortingEnabled() const;
    Q_REVISION(6, 9) QVector3D shadowBoundsMinimum() const;
    Q_REVISION(6, 9) QVector3D shadowBoundsMaximum() const;

    Q_REVISION(6, 3) Q_INVOKABLE QVector3D instancePosition(int index);
    Q_REVISION(6, 3) Q_INVOKABLE QVector3D instanceScale(int index);
    Q_REVISION(6, 3) Q_INVOKABLE QQuaternion instanceRotation(int index);
    Q_REVISION(6, 3) Q_INVOKABLE QColor instanceColor(int index);
    Q_REVISION(6, 3) Q_INVOKABLE QVector4D instanceCustomData(int index);

public Q_SLOTS:
    void setInstanceCountOverride(int instanceCountOverride);
    void setHasTransparency(bool hasTransparency);
    void setDepthSortingEnabled(bool enabled);
    Q_REVISION(6, 9) void setShadowBoundsMinimum(const QVector3D &newShadowBoundsMinimum);
    Q_REVISION(6, 9) void setShadowBoundsMaximum(const QVector3D &newShadowBoundsMinimum);

Q_SIGNALS:
    void instanceTableChanged();
    void instanceNodeDirty();
    void instanceCountOverrideChanged();
    void hasTransparencyChanged();
    void depthSortingEnabledChanged();
    Q_REVISION(6, 9) void shadowBoundsMinimumChanged();
    Q_REVISION(6, 9) void shadowBoundsMaximumChanged();

protected:
    virtual QByteArray getInstanceBuffer(int *instanceCount) = 0;
    void markDirty();
    static InstanceTableEntry calculateTableEntry(const QVector3D &position,
                          const QVector3D &scale, const QVector3D &eulerRotation,
                                                  const QColor &color, const QVector4D &customData = {});
    static InstanceTableEntry calculateTableEntryFromQuaternion(const QVector3D &position,
                          const QVector3D &scale, const QQuaternion &rotation,
                                                  const QColor &color, const QVector4D &customData = {});
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;

private:
    const InstanceTableEntry *getInstanceEntry(int index);
};

QT_END_NAMESPACE

#endif // Q_QUICK3D_INSTANCING_H
