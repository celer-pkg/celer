// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGSKIN_H
#define QSSGSKIN_H

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

#include <QtQuick3D/qquick3dobject.h>
#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtCore/qlist.h>
#include <QtCore/qhash.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QQuick3DSkin : public QQuick3DObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QQuick3DNode> joints READ joints)
    Q_PROPERTY(QList<QMatrix4x4> inverseBindPoses READ inverseBindPoses WRITE setInverseBindPoses NOTIFY inverseBindPosesChanged)

    QML_NAMED_ELEMENT(Skin)

public:
    explicit QQuick3DSkin(QQuick3DObject *parent = nullptr);
    ~QQuick3DSkin() override;

    QQmlListProperty<QQuick3DNode> joints();
    QList<QMatrix4x4> inverseBindPoses() const;

public Q_SLOTS:
    void setInverseBindPoses(const QList<QMatrix4x4> &poses);

Q_SIGNALS:
    void inverseBindPosesChanged();

private:
    static void qmlAppendJoint(QQmlListProperty<QQuick3DNode> *list, QQuick3DNode *joint);
    static QQuick3DNode *qmlJointAt(QQmlListProperty<QQuick3DNode> *list, qsizetype index);
    static qsizetype qmlJointsCount(QQmlListProperty<QQuick3DNode> *list);
    static void qmlClearJoints(QQmlListProperty<QQuick3DNode> *list);

    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;

    QVector<QQuick3DNode *> m_joints;
    QByteArray m_boneData;
    QList<QMatrix4x4> m_inverseBindPoses;
    using JointConnections = std::pair<QMetaObject::Connection, QMetaObject::Connection>;
    QHash<QObject *, JointConnections> m_jointsConnections;
    QSet<QQuick3DNode *> m_dirtyJoints;
    QSet<QQuick3DNode *> m_removedJoints;
    int m_updatedByNewInverseBindPoses = 0;
};

QT_END_NAMESPACE

#endif // QSSGSKIN_H
