// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef BOXSHAPE_H
#define BOXSHAPE_H

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
#include <QtQuick3DPhysics/private/qabstractcollisionshape_p.h>
#include <QtCore/QObject>
#include <QtGui/QVector3D>
#include <QtQml/QQmlEngine>

namespace physx {
class PxBoxGeometry;
}

QT_BEGIN_NAMESPACE

class Q_QUICK3DPHYSICS_EXPORT QBoxShape : public QAbstractCollisionShape
{
    Q_OBJECT
    Q_PROPERTY(QVector3D extents READ extents WRITE setExtents NOTIFY extentsChanged)
    QML_NAMED_ELEMENT(BoxShape)
public:
    QBoxShape();
    ~QBoxShape();

    QVector3D extents() const;

    physx::PxGeometry *getPhysXGeometry() override;
    bool isStaticShape() const override { return false; }

public slots:
    void setExtents(QVector3D extents);

signals:
    void extentsChanged(QVector3D extents);

private:
    void updatePhysXGeometry();

    physx::PxBoxGeometry *m_physXGeometry = nullptr;
    QVector3D m_extents = QVector3D(100, 100, 100);
};

QT_END_NAMESPACE

#endif // BOXSHAPE_H
