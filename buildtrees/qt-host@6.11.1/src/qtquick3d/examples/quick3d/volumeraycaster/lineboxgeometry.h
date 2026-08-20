// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef LINEBOXGEOMETRY_H
#define LINEBOXGEOMETRY_H

#include <QQuick3DGeometry>

class LineBoxGeometry : public QQuick3DGeometry
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LineBoxGeometry)

public:
    LineBoxGeometry();
};

#endif
