// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef RECURSIVEOBJECT_H
#define RECURSIVEOBJECT_H

#include <QtQmlIntegration/qqmlintegration.h>

#include <QtCore/qobject.h>
#include <QtCore/qvariantmap.h>

class RecursiveObject : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:

    Q_INVOKABLE QVariantMap getObject() {
        return QVariantMap { {
            QLatin1String("sub"),
            QVariantMap { {QLatin1String("name"), QString::number(index++)} }
        } };
    }

private:
    int index = 0;
};

#endif // RECURSIVEOBJECT_H
