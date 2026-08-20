// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef NONINDEXTRIANGLESTOPOLOGY_H
#define NONINDEXTRIANGLESTOPOLOGY_H

#include <QtQuick3D/QQuick3DGeometry>

class NonIndexTrianglesTopology : public QQuick3DGeometry
{
    Q_OBJECT
    QML_NAMED_ELEMENT(NonIndexTrianglesTopology)
public:
    NonIndexTrianglesTopology();

private:
    void updateData();
};

#endif // NONINDEXTRIANGLESTOPOLOGY_H
