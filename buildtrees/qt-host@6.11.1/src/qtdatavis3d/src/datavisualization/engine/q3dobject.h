// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_Q3DOBJECT_H
#define QTDATAVIS3D_Q3DOBJECT_H

#include <QtDataVisualization/qdatavisualizationglobal.h>
#include <QtCore/QObject>
#include <QtGui/QVector3D>

Q_MOC_INCLUDE(<QtDataVisualization/q3dscene.h>)

QT_BEGIN_NAMESPACE

class Q3DObjectPrivate;
class Q3DScene;

class Q_DATAVISUALIZATION_EXPORT Q3DObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Q3DScene* parentScene READ parentScene)
    Q_PROPERTY(QVector3D position READ position WRITE setPosition NOTIFY positionChanged)

public:
    explicit Q3DObject(QObject *parent = nullptr);
    virtual ~Q3DObject();

    virtual void copyValuesFrom(const Q3DObject &source);

    Q3DScene *parentScene();

    QVector3D position() const;
    void setPosition(const QVector3D &position);

Q_SIGNALS:
    void positionChanged(const QVector3D &position);

protected:
    void setDirty(bool dirty);
    bool isDirty() const;

private:
    QScopedPointer<Q3DObjectPrivate> d_ptr;

    Q_DISABLE_COPY(Q3DObject)

    friend class Q3DScenePrivate;
};

QT_END_NAMESPACE

#endif
