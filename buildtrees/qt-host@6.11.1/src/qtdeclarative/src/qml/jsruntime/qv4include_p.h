// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QV4INCLUDE_P_H
#define QV4INCLUDE_P_H

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

#include <QtCore/qobject.h>
#include <QtCore/qurl.h>
#include <QtCore/qpointer.h>

#include <private/qqmlnotifyingblob_p.h>
#include <private/qv4context_p.h>
#include <private/qv4persistent_p.h>
#include <private/qv4value_p.h>

QT_BEGIN_NAMESPACE

class QV4Include : public QQmlNotifyingBlob::Callback
{
public:
    enum Status {
        Ok = 0,
        Loading = 1,
        NetworkError = 2,
        Exception = 3
    };

    static QJSValue method_include(QV4::ExecutionEngine *engine, const QUrl &url,
                                   const QJSValue &callbackFunction);

    void ready(QQmlNotifyingBlob *) final;

private:
    static void callback(QV4::FunctionObject *callback, QV4::Object *result);
    static QV4::ReturnedValue resultValue(QV4::ExecutionEngine *v4);
    static void populateResultValue(
            QV4::Object *o, Status status, const QString &statusText = QString());
    static void processScriptBlob(
            QQmlScriptBlob *scriptBlob, QV4::Object *result, QV4::FunctionObject *callbackFunction,
            QV4::QmlContext *qmlContext);

    QV4Include(
            QV4::Object *result, QV4::FunctionObject *callbackFunction, QV4::QmlContext *qmlContext);

    QV4::PersistentValue m_callbackFunction;
    QV4::PersistentValue m_resultObject;
    QV4::PersistentValue m_qmlContext;
};

QT_END_NAMESPACE

#endif

