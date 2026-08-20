// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef INDEXTRIANGLESTOPOLOGY_H
#define INDEXTRIANGLESTOPOLOGY_H

#include <QtQuick3D/QQuick3DGeometry>

class IndexTrianglesTopology : public QQuick3DGeometry
{
    Q_OBJECT
    QML_NAMED_ELEMENT(IndexTrianglesTopology)
public:
    IndexTrianglesTopology();

private:
    void updateData();
};

#endif // INDEXTRIANGLESTOPOLOGY_H
