// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPICKRESULT_P_H
#define QQUICK3DPICKRESULT_P_H

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
#include <QObject>
#include <QtGui/QVector2D>
#include <QtGui/QVector3D>
#include "qquick3dmodel_p.h"

QT_BEGIN_NAMESPACE
class QQuick3DModel;

namespace QQuick3DPickResultEnums
{
Q_NAMESPACE_EXPORT(Q_QUICK3D_EXPORT)

QML_NAMED_ELEMENT(PickResult)
QML_ADDED_IN_VERSION(6, 8)

enum HitType {
    Null,
    Model,
    Item,
};
Q_ENUM_NS(HitType)

};


class Q_QUICK3D_EXPORT QQuick3DPickResult
{
    Q_GADGET
    Q_PROPERTY(QQuick3DModel* objectHit READ objectHit CONSTANT)
    Q_PROPERTY(float distance READ distance CONSTANT)
    Q_PROPERTY(QVector2D uvPosition READ uvPosition CONSTANT)
    Q_PROPERTY(QVector3D scenePosition READ scenePosition CONSTANT)
    Q_PROPERTY(QVector3D position READ position CONSTANT)
    Q_PROPERTY(QVector3D normal READ normal CONSTANT)
    Q_PROPERTY(QVector3D sceneNormal READ sceneNormal CONSTANT)
    Q_PROPERTY(int instanceIndex READ instanceIndex CONSTANT)
    Q_PROPERTY(QQuickItem *itemHit READ itemHit CONSTANT REVISION(6, 8))
    Q_PROPERTY(QQuick3DPickResultEnums::HitType hitType READ hitType CONSTANT REVISION(6, 8))
    QML_VALUE_TYPE(pickResult)
public:

    QQuick3DPickResult();
    explicit QQuick3DPickResult(QQuick3DModel *hitObject,
                                float distanceFromCamera,
                                const QVector2D &uvPosition,
                                const QVector3D &scenePosition,
                                const QVector3D &position,
                                const QVector3D &normal,
                                const QVector3D &sceneNormal,
                                int instanceIndex,
                                QQuickItem *itemHit = 0);

    explicit QQuick3DPickResult(QQuickItem *itemHit,
                                float distanceFromCamera,
                                const QVector2D &uvPosition,
                                const QVector3D &scenePosition,
                                const QVector3D &position,
                                const QVector3D &sceneNormal);
    QQuick3DModel *objectHit() const;
    float distance() const;
    QVector2D uvPosition() const;
    QVector3D scenePosition() const;
    QVector3D position() const;
    QVector3D normal() const;
    QVector3D sceneNormal() const;
    int instanceIndex() const;
    Q_REVISION(6, 8) QQuickItem *itemHit() const;
    Q_REVISION(6, 8) QQuick3DPickResultEnums::HitType hitType() const;

private:
    QQuick3DModel *m_objectHit;
    float m_distance;
    QVector2D m_uvPosition;
    QVector3D m_scenePosition;
    QVector3D m_position;
    QVector3D m_normal;
    QVector3D m_sceneNormal;
    int m_instanceIndex;
    QQuickItem *m_itemHit;
    QQuick3DPickResultEnums::HitType m_hitType;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QQuick3DPickResult)

#endif // QQUICK3DPICKRESULT_P_H
