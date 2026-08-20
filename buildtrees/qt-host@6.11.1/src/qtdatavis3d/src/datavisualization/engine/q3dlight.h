// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_Q3DLIGHT_H
#define QTDATAVIS3D_Q3DLIGHT_H

#include <QtDataVisualization/q3dobject.h>

QT_BEGIN_NAMESPACE

class Q3DLightPrivate;

class Q_DATAVISUALIZATION_EXPORT Q3DLight : public Q3DObject
{
    Q_OBJECT
    Q_PROPERTY(bool autoPosition READ isAutoPosition WRITE setAutoPosition NOTIFY autoPositionChanged REVISION(1, 3))

public:
    explicit Q3DLight(QObject *parent = nullptr);
    virtual ~Q3DLight();

    void setAutoPosition(bool enabled);
    bool isAutoPosition();

Q_SIGNALS:
    Q_REVISION(1, 3) void autoPositionChanged(bool autoPosition);

private:
    QScopedPointer<Q3DLightPrivate> d_ptr;

    Q_DISABLE_COPY(Q3DLight)

    friend class Q3DLightPrivate;
    friend class Q3DScenePrivate;
};

QT_END_NAMESPACE

#endif
