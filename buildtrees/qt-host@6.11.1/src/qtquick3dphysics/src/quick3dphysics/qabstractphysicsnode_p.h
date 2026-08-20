// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef ABSTRACTPHYSICSNODE_H
#define ABSTRACTPHYSICSNODE_H

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

#include <QtQuick3DPhysics/qtquick3dphysicsglobal.h>
#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlListProperty>
#include <QtQuick3DPhysics/private/qabstractcollisionshape_p.h>

namespace physx {
class PxTransform;
class PxShape;
}

QT_BEGIN_NAMESPACE

class QAbstractPhysXNode;

class Q_QUICK3DPHYSICS_EXPORT QAbstractPhysicsNode : public QQuick3DNode
{
    Q_OBJECT
    Q_PROPERTY(
            QQmlListProperty<QAbstractCollisionShape> collisionShapes READ collisionShapes CONSTANT)
    Q_PROPERTY(bool sendContactReports READ sendContactReports WRITE setSendContactReports NOTIFY
                       sendContactReportsChanged)
    Q_PROPERTY(bool receiveContactReports READ receiveContactReports WRITE setReceiveContactReports
                       NOTIFY receiveContactReportsChanged)
    Q_PROPERTY(bool sendTriggerReports READ sendTriggerReports WRITE setSendTriggerReports NOTIFY
                       sendTriggerReportsChanged REVISION(6, 5))
    Q_PROPERTY(bool receiveTriggerReports READ receiveTriggerReports WRITE setReceiveTriggerReports
                       NOTIFY receiveTriggerReportsChanged REVISION(6, 5))
    Q_PROPERTY(int filterGroup READ filterGroup WRITE setfilterGroup NOTIFY filterGroupChanged
                       REVISION(6, 7))
    Q_PROPERTY(int filterIgnoreGroups READ filterIgnoreGroups WRITE setFilterIgnoreGroups NOTIFY
                       filterIgnoreGroupsChanged REVISION(6, 7));

    QML_NAMED_ELEMENT(PhysicsNode)
    QML_UNCREATABLE("abstract interface")
public:
    QAbstractPhysicsNode();
    ~QAbstractPhysicsNode() override;

    QQmlListProperty<QAbstractCollisionShape> collisionShapes();
    const QVector<QAbstractCollisionShape *> &getCollisionShapesList() const;

    void updateFromPhysicsTransform(const physx::PxTransform &transform);

    void registerContact(QAbstractPhysicsNode *body, const QVector<QVector3D> &positions,
                         const QVector<QVector3D> &impulses, const QVector<QVector3D> &normals);

    bool sendContactReports() const;
    void setSendContactReports(bool sendContactReports);

    bool receiveContactReports() const;
    void setReceiveContactReports(bool receiveContactReports);

    Q_REVISION(6, 5) bool sendTriggerReports() const;
    Q_REVISION(6, 5) void setSendTriggerReports(bool sendTriggerReports);

    Q_REVISION(6, 5) bool receiveTriggerReports() const;
    Q_REVISION(6, 5) void setReceiveTriggerReports(bool receiveTriggerReports);

    bool hasStaticShapes() const { return m_hasStaticShapes; }

    virtual QAbstractPhysXNode *createPhysXBackend() = 0;

    Q_REVISION(6, 7) int filterGroup() const;
    Q_REVISION(6, 7) void setfilterGroup(int newfilterGroup);

    Q_REVISION(6, 7) int filterIgnoreGroups() const;
    Q_REVISION(6, 7) void setFilterIgnoreGroups(int newFilterIgnoreGroups);

private Q_SLOTS:
    void onShapeDestroyed(QObject *object);
    void onShapeNeedsRebuild(QObject *object);

Q_SIGNALS:
    void bodyContact(QAbstractPhysicsNode *body, const QVector<QVector3D> &positions,
                     const QVector<QVector3D> &impulses, const QVector<QVector3D> &normals);
    void sendContactReportsChanged(float sendContactReports);
    void receiveContactReportsChanged(float receiveContactReports);
    Q_REVISION(6, 5) void sendTriggerReportsChanged(float sendTriggerReports);
    Q_REVISION(6, 5) void receiveTriggerReportsChanged(float receiveTriggerReports);
    Q_REVISION(6, 5) void enteredTriggerBody(QAbstractPhysicsNode *body);
    Q_REVISION(6, 5) void exitedTriggerBody(QAbstractPhysicsNode *body);
    Q_REVISION(6, 7) void filterGroupChanged();
    Q_REVISION(6, 7) void filterIgnoreGroupsChanged();

private:
    static void qmlAppendShape(QQmlListProperty<QAbstractCollisionShape> *list,
                               QAbstractCollisionShape *shape);
    static QAbstractCollisionShape *qmlShapeAt(QQmlListProperty<QAbstractCollisionShape> *list,
                                               qsizetype index);
    static qsizetype qmlShapeCount(QQmlListProperty<QAbstractCollisionShape> *list);
    static void qmlClearShapes(QQmlListProperty<QAbstractCollisionShape> *list);

    QVector<QAbstractCollisionShape *> m_collisionShapes;
    bool m_shapesDirty = false;
    bool m_sendContactReports = false;
    bool m_receiveContactReports = false;
    bool m_sendTriggerReports = false;
    bool m_receiveTriggerReports = false;
    bool m_hasStaticShapes = false;
    int m_filterGroup = 0;
    int m_filterIgnoreGroups = 0;
    bool m_filtersDirty = false;

    friend class QAbstractPhysXNode;
    friend class QPhysicsWorld; // for register/deregister TODO: cleaner mechanism
    friend class SimulationEventCallback;
    QAbstractPhysXNode *m_backendObject = nullptr;
};

QT_END_NAMESPACE

#endif // ABSTRACTPHYSICSNODE_H
