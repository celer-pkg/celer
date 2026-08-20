// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef POINTSTOPOLOGY_H
#define POINTSTOPOLOGY_H

#include <QtQuick3D/QQuick3DGeometry>

class PointsTopology : public QQuick3DGeometry
{
    Q_OBJECT
    QML_NAMED_ELEMENT(PointsTopology)
public:
    PointsTopology();

private:
    void updateData();
};

#endif // POINTSTOPOLOGY_H
