// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QSCXMLECMASCRIPTPLATFORMPROPERTIES_P_H
#define QSCXMLECMASCRIPTPLATFORMPROPERTIES_P_H

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

#include "qscxmlglobals.h"

#include <QtCore/qobject.h>

QT_FORWARD_DECLARE_CLASS(QJSEngine)
QT_FORWARD_DECLARE_CLASS(QJSValue)

QT_BEGIN_NAMESPACE

class QScxmlStateMachine;
class QScxmlPlatformProperties: public QObject
{
    Q_OBJECT

    QScxmlPlatformProperties(QObject *parent);

    Q_PROPERTY(QString marks READ marks CONSTANT)

public:
    static QScxmlPlatformProperties *create(QJSEngine *engine, QScxmlStateMachine *stateMachine);
    ~QScxmlPlatformProperties();

    QJSEngine *engine() const;
    QScxmlStateMachine *stateMachine() const;
    QJSValue jsValue() const;

    QString marks() const;

    Q_INVOKABLE bool inState(const QString &stateName);

private:
    class Data;
    Data *data;
};

QT_END_NAMESPACE

#endif // QSCXMLECMASCRIPTPLATFORMPROPERTIES_P_H
