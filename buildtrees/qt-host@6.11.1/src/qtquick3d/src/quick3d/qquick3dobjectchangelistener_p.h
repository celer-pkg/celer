// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGITEMCHANGELISTENER_P_H
#define QSSGITEMCHANGELISTENER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuick3D/qquick3dobject.h>
#include <private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QQuick3DObjectChangeListener
{
public:
    virtual ~QQuick3DObjectChangeListener() {}

    virtual void itemSiblingOrderChanged(QQuick3DObject *) {}
    virtual void itemVisibilityChanged(QQuick3DObject *) {}
    virtual void itemEnabledChanged(QQuick3DObject *) {}
    virtual void itemOpacityChanged(QQuick3DObject *) {}
    virtual void itemDestroyed(QQuick3DObject *) {}
    virtual void itemChildAdded(QQuick3DObject *, QQuick3DObject * /* child */) {}
    virtual void itemChildRemoved(QQuick3DObject *, QQuick3DObject * /* child */) {}
    virtual void itemParentChanged(QQuick3DObject *, QQuick3DObject * /* parent */) {}
};

QT_END_NAMESPACE

#endif // QSSGITEMCHANGELISTENER_P_H
