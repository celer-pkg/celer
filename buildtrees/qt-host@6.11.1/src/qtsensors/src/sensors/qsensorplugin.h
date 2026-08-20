// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QSENSORPLUGIN_H
#define QSENSORPLUGIN_H

#include <QtSensors/qsensorsglobal.h>

#include <QtCore/qplugin.h>

QT_BEGIN_NAMESPACE

class Q_SENSORS_EXPORT QSensorPluginInterface
{
public:
    virtual void registerSensors() = 0;
protected:
    virtual ~QSensorPluginInterface();
};

class Q_SENSORS_EXPORT QSensorChangesInterface
{
public:
    virtual void sensorsChanged() = 0;
protected:
    virtual ~QSensorChangesInterface();
};

Q_DECLARE_INTERFACE(QSensorPluginInterface, "com.qt-project.Qt.QSensorPluginInterface/1.0")
Q_DECLARE_INTERFACE(QSensorChangesInterface, "com.qt-project.Qt.QSensorChangesInterface/5.0")

QT_END_NAMESPACE

#endif

