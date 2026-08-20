// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qsensorplugin.h"

QT_BEGIN_NAMESPACE

/*!
    \class QSensorPluginInterface
    \ingroup sensors_backend
    \inmodule QtSensors
    \since 5.1
    \brief The QSensorPluginInterface class is the pure virtual interface to sensor plugins.

    The QSensorPluginInterface class is implemented in sensor plugins to register sensor
    backends with QSensorManager.

    \sa {Creating a sensor plugin}
*/

/*!
    \internal
*/
QSensorPluginInterface::~QSensorPluginInterface()
{
}

/*!
    \fn QSensorPluginInterface::registerSensors()

    This function is called when the plugin is loaded. The plugin should register
    sensor backends by calling QSensorManager::registerBackend(). Any backends
    that utilise other sensors should be registered in the
    QSensorPluginInterface::sensorsChanged() method instead.

    \sa {Creating a sensor plugin}
*/

/*!
    \class QSensorChangesInterface
    \ingroup sensors_backend
    \inmodule QtSensors
    \since 5.1
    \brief The QSensorChangesInterface class is the pure virtual interface to sensor plugins.

    The QSensorChangesInterface class is implemented in sensor plugins to receive notification
    that registered sensor backends have changed.

    \sa {Creating a sensor plugin}
*/

/*!
    \internal
*/
QSensorChangesInterface::~QSensorChangesInterface()
{
}

/*!
    \fn QSensorChangesInterface::sensorsChanged()

    This function is called when the registered backends have changed.
    Any backends that depend on the presence of other sensors should be
    registered or unregistered in here.

    \sa {Creating a sensor plugin}
*/

QT_END_NAMESPACE
