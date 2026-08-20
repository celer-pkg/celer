// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TILEGEOMETRY_H
#define TILEGEOMETRY_H

#include <QtQuick3D/QQuick3DGeometry>

class TileGeometry : public QQuick3DGeometry
{
    Q_OBJECT
    QML_NAMED_ELEMENT(TileGeometry)

    Q_PROPERTY(bool hasHole READ hasHole WRITE setHasHole NOTIFY hasHoleChanged FINAL)

public:
    TileGeometry();

    bool hasHole();
    void setHasHole(bool v);

signals:
    void hasHoleChanged(bool v);

private:
    void createFloor();
    bool m_hasHole = false;
};

#endif
