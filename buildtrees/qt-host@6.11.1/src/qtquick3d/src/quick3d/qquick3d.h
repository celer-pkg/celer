// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef Q_QUICK3D_H
#define Q_QUICK3D_H

#include <QtGui/qsurfaceformat.h>
#include <QtQuick3D/qtquick3dglobal.h>

QT_BEGIN_NAMESPACE

class QQuick3D {
public:
static Q_QUICK3D_EXPORT QSurfaceFormat idealSurfaceFormat(int samples = -1);
};

QT_END_NAMESPACE

#endif
