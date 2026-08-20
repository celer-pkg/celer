// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTQUICK3DPHYSICSGLOBAL_H
#define QTQUICK3DPHYSICSGLOBAL_H

#include <QtGui/qtguiglobal.h>

QT_BEGIN_NAMESPACE

#ifndef Q_QUICK3DPHYSICS_EXPORT
#    ifndef QT_STATIC
#        if defined(QT_BUILD_QUICK3DPHYSICS_LIB)
#            define Q_QUICK3DPHYSICS_EXPORT Q_DECL_EXPORT
#        else
#            define Q_QUICK3DPHYSICS_EXPORT Q_DECL_IMPORT
#        endif
#    else
#        define Q_QUICK3DPHYSICS_EXPORT
#    endif
#endif

void Q_QUICK3DPHYSICS_EXPORT qml_register_types_QtQuick3D_Physics();

QT_END_NAMESPACE

#endif // QTQUICK3DPHYSICSGLOBAL_H
