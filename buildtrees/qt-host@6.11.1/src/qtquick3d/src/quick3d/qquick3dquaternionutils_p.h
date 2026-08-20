// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DQUATERNIONUTILS_H
#define QQUICK3DQUATERNIONUTILS_H

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

#include <QtQuick3D/private/qtquick3dglobal_p.h>

#include <QtCore/QObject>
#include <QtGui/QQuaternion>
#include <QtGui/QVector3D>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QQuick3DQuaternionUtils : public QObject
{
    Q_OBJECT

    QML_NAMED_ELEMENT(Quaternion)
    QML_SINGLETON

public:
    explicit QQuick3DQuaternionUtils(QObject *parent = nullptr);

    Q_INVOKABLE static QQuaternion fromAxesAndAngles(const QVector3D &axis1,
                                                     float angle1,
                                                     const QVector3D &axis2,
                                                     float angle2,
                                                     const QVector3D &axis3,
                                                     float angle3);
    Q_INVOKABLE static QQuaternion fromAxesAndAngles(const QVector3D &axis1,
                                                     float angle1,
                                                     const QVector3D &axis2,
                                                     float angle2);
    Q_INVOKABLE static QQuaternion fromAxisAndAngle(float x, float y, float z, float angle);
    Q_INVOKABLE static QQuaternion fromAxisAndAngle(const QVector3D &axis, float angle);
    Q_INVOKABLE static QQuaternion fromEulerAngles(float x, float y, float z);
    Q_INVOKABLE static QQuaternion fromEulerAngles(const QVector3D &eulerAngles);

    Q_INVOKABLE static QQuaternion lookAt(const QVector3D &sourcePosition,
                                          const QVector3D &targetPosition,
                                          const QVector3D &forwardDirection = QVector3D(0, 0, -1),
                                          const QVector3D &upDirection = QVector3D(0, 1, 0));

};

QT_END_NAMESPACE

#endif // QQUICK3DQUATERNIONUTILS_H
