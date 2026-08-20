// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef Q_QUICK3D_INSTANCING_P_H
#define Q_QUICK3D_INSTANCING_P_H

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

#include <QtQuick3D/qquick3dinstancing.h>
#include <QtQuick3D/private/qquick3dobject_p.h>

#include <QtGui/qvector3d.h>

QT_BEGIN_NAMESPACE

class QQuick3DInstancingPrivate : public QQuick3DObjectPrivate
{
public:
    QQuick3DInstancingPrivate();
    int m_instanceCountOverride = -1;
    int m_instanceCount = 0;
    bool m_hasTransparency = false;
    bool m_instanceDataChanged = true;
    bool m_instanceCountOverrideChanged = false;
    bool m_depthSortingEnabled = false;
    QVector3D m_shadowBoundsMinimum = QVector3D(1, 1, 1);
    QVector3D m_shadowBoundsMaximum = QVector3D(-1, -1, -1);
};

class Q_QUICK3D_EXPORT QQuick3DInstanceListEntry : public QQuick3DObject
{
    Q_OBJECT

    QML_ADDED_IN_VERSION(6, 2)
    Q_PROPERTY(QVector3D position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QVector3D scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(QVector3D eulerRotation READ eulerRotation WRITE setEulerRotation NOTIFY eulerRotationChanged)
    Q_PROPERTY(QQuaternion rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QVector4D customData READ customData WRITE setCustomData NOTIFY customDataChanged)
    QML_NAMED_ELEMENT(InstanceListEntry)

public:
    explicit QQuick3DInstanceListEntry(QQuick3DObject *parent = nullptr);
    ~QQuick3DInstanceListEntry() override {}

    QVector3D position() const
    {
        return m_position;
    }
    QVector3D scale() const
    {
        return m_scale;
    }

    QVector3D eulerRotation() const
    {
        return m_eulerRotation;
    }

    QQuaternion rotation() const
    {
        return m_rotation;
    }

    QColor color() const
    {
        return m_color;
    }

    QVector4D customData() const
    {
        return m_customData;
    }

public Q_SLOTS:
    void setPosition(QVector3D position);
    void setScale(QVector3D scale);
    void setEulerRotation(QVector3D eulerRotation);
    void setRotation(QQuaternion rotation);
    void setColor(QColor color);
    void setCustomData(QVector4D customData);

Q_SIGNALS:
    void positionChanged();
    void scaleChanged();
    void eulerRotationChanged();
    void rotationChanged();
    void colorChanged();
    void customDataChanged();
    void changed();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *) override
    {
        return nullptr;
    }

private:
    QVector3D m_position;
    QVector3D m_scale = {1, 1, 1};
    QVector3D m_eulerRotation;
    QQuaternion m_rotation;
    QColor m_color = Qt::white;
    QVector4D m_customData;
    bool m_useEulerRotation = true;
    friend class QQuick3DInstanceList;
};

class Q_QUICK3D_EXPORT QQuick3DInstanceList : public QQuick3DInstancing
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QQuick3DInstanceListEntry> instances READ instances)
    Q_PROPERTY(int instanceCount READ instanceCount NOTIFY instanceCountChanged)
    QML_NAMED_ELEMENT(InstanceList)
    QML_ADDED_IN_VERSION(6, 2)

    Q_CLASSINFO("DefaultProperty", "instances")

public:
    explicit QQuick3DInstanceList(QQuick3DObject *parent = nullptr);
    ~QQuick3DInstanceList() override;

    QByteArray getInstanceBuffer(int *instanceCount) override;
    QQmlListProperty<QQuick3DInstanceListEntry> instances();
    int instanceCount() const;

Q_SIGNALS:
    void instanceCountChanged();

private Q_SLOTS:
    void handleInstanceChange();
    void onInstanceDestroyed(QObject *object);

private:
    void generateInstanceData();

    static void qmlAppendInstanceListEntry(QQmlListProperty<QQuick3DInstanceListEntry> *list, QQuick3DInstanceListEntry *material);
    static QQuick3DInstanceListEntry *qmlInstanceListEntryAt(QQmlListProperty<QQuick3DInstanceListEntry> *list, qsizetype index);
    static qsizetype qmlInstanceListEntriesCount(QQmlListProperty<QQuick3DInstanceListEntry> *list);
    static void qmlClearInstanceListEntries(QQmlListProperty<QQuick3DInstanceListEntry> *list);

    bool m_dirty = true;
    QByteArray m_instanceData;
    QList<QQuick3DInstanceListEntry *> m_instances;
};

class Q_QUICK3D_EXPORT QQuick3DFileInstancing : public QQuick3DInstancing
{
    Q_OBJECT
    QML_NAMED_ELEMENT(FileInstancing)
    QML_ADDED_IN_VERSION(6, 2)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(int instanceCount READ instanceCount NOTIFY instanceCountChanged)

public:
    explicit QQuick3DFileInstancing(QQuick3DObject *parent = nullptr);
    ~QQuick3DFileInstancing() override;

    const QUrl &source() const;
    void setSource(const QUrl &newSource);

    bool loadFromBinaryFile(const QString &filename);
    bool loadFromXmlFile(const QString &filename);
    int writeToBinaryFile(QIODevice *out);

    int instanceCount() const;

Q_SIGNALS:
    void instanceCountChanged();
    void sourceChanged();

protected:
    QByteArray getInstanceBuffer(int *instanceCount) override;

private:
    bool loadFromFile(const QUrl &source);

private:
    int m_instanceCount = 0;
    QByteArray m_instanceData;
    QFile *m_dataFile = nullptr;
    bool m_dirty = true;
    QUrl m_source;
};

QT_END_NAMESPACE

#endif
