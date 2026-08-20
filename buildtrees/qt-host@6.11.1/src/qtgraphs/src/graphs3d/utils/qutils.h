// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QUTILS_H
#define QTGRAPHS_QUTILS_H

#include <QtGraphs/qgraphsglobal.h>
#include <QtCore/qcoreapplication.h>
#include <QtGui/qsurfaceformat.h>
#include <QtQuick3D/qquick3d.h>


QT_BEGIN_NAMESPACE

[[maybe_unused]] static inline QSurfaceFormat qDefaultSurfaceFormat(bool antialias)
{
    QSurfaceFormat surfaceFormat;

    if (antialias)
        surfaceFormat = QQuick3D::idealSurfaceFormat(8);
    else
        surfaceFormat = QQuick3D::idealSurfaceFormat();

    return surfaceFormat;
}

QT_END_NAMESPACE

#endif
