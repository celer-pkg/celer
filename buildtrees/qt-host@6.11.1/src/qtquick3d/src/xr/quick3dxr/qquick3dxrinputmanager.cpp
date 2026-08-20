// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dxrinputmanager_p.h"

#if defined(Q_OS_VISIONOS)
# include "visionos/qquick3dxrinputmanager_visionos_p.h"
#else
# include "openxr/qopenxrinputmanager_p.h"
#endif

#include "qquick3dxrcontroller_p.h"

QT_BEGIN_NAMESPACE

QQuick3DXrInputManager *QQuick3DXrInputManager::instance()
{
    static QQuick3DXrInputManager instance;
    return &instance;
}

QQuick3DXrHandInput *QQuick3DXrInputManager::leftHandInput() const
{
    Q_D(const QQuick3DXrInputManager);
    return d->leftHandInput();
}

QQuick3DXrHandInput *QQuick3DXrInputManager::rightHandInput() const
{
    Q_D(const QQuick3DXrInputManager);
    return d->rightHandInput();
}

void QQuick3DXrInputManager::registerController(QQuick3DXrController *controller)
{
    Q_D(QQuick3DXrInputManager);

    connect(controller, &QObject::destroyed, this, [this](QObject *obj) { unregisterController(static_cast<QQuick3DXrController *>(obj)); });

    d->registerController(controller);
}

void QQuick3DXrInputManager::unregisterController(QQuick3DXrController *controller)
{
    Q_D(QQuick3DXrInputManager);
    d->unregisterController(controller);
}

bool QQuick3DXrInputManager::isValid() const
{
    Q_D(const QQuick3DXrInputManager);
    return d->isValid();
}

QQuick3DXrInputManager::QQuick3DXrInputManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new QQuick3DXrInputManagerPrivate(*this))
{

}

QQuick3DXrInputManager::~QQuick3DXrInputManager()
{

}

QT_END_NAMESPACE
