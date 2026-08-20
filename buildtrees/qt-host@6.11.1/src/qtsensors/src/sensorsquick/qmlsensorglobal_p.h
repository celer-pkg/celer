// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QMLSENSORGLOBAL_P_H
#define QMLSENSORGLOBAL_P_H

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

#include "qsensorsquickglobal_p.h"
#include <QtQml/qqml.h>
#include <QObject>
#include <QStringList>

QT_BEGIN_NAMESPACE

class QSensor;

class Q_SENSORSQUICK_EXPORT QmlSensorGlobal : public QObject
{
    Q_OBJECT
public:
    explicit QmlSensorGlobal(QObject *parent = 0);
    ~QmlSensorGlobal();

    Q_INVOKABLE QStringList sensorTypes() const;
    Q_INVOKABLE QStringList sensorsForType(const QString &type) const;
    Q_INVOKABLE QString defaultSensorForType(const QString &type) const;
    QML_NAMED_ELEMENT(QmlSensors)
    QML_SINGLETON
    QML_ADDED_IN_VERSION(5,0)

Q_SIGNALS:
    void availableSensorsChanged();

private:
    QSensor *m_sensor;
};

QT_END_NAMESPACE

#endif
