// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QREMOTEOBJECTSQML_P_H
#define QREMOTEOBJECTSQML_P_H

#include <QtRemoteObjects/qremoteobjectnode.h>
#include <QtRemoteObjects/qremoteobjectpendingcall.h>
#include <QtRemoteObjects/qremoteobjectsettingsstore.h>

#include <QtCore/qtimer.h>
#include <QtQml/QJSValue>
#include <QtQml/qqml.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlinfo.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

struct QtQmlRemoteObjectsResponse
{
    QJSValue promise;
    QTimer *timer;
};

// documentation updates for this class can be made in remoteobjects-qml.qdoc
class QtQmlRemoteObjects : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(QtRemoteObjects)
    QML_SINGLETON
    QML_ADDED_IN_VERSION(5, 14)

public:
    ~QtQmlRemoteObjects()
    {
        auto i = m_callbacks.begin();
        while (i != m_callbacks.end()) {
            delete i.key();
            delete i.value().timer;
            i = m_callbacks.erase(i);
        }
    }

    Q_INVOKABLE QJSValue watch(const QRemoteObjectPendingCall &reply, int timeout = 30000)
    {
        if (m_accessiblePromise.isUndefined())
            m_accessiblePromise = qmlEngine(this)->evaluate(QLatin1String(
                    "(function() { var obj = {}; obj.promise = new Promise(function(resolve, "
                    "reject) { obj.resolve = resolve; obj.reject = reject; }); return obj; })"));

        QRemoteObjectPendingCallWatcher *watcher = new QRemoteObjectPendingCallWatcher(reply);

        QJSValue promise = m_accessiblePromise.call();
        QtQmlRemoteObjectsResponse response;
        response.promise = promise;
        response.timer = new QTimer();
        response.timer->setSingleShot(true);
        m_callbacks.insert(watcher, response);

        // handle timeout
        connect(response.timer, &QTimer::timeout, this, [this, watcher]() {
            auto i = m_callbacks.find(watcher);
            if (i == m_callbacks.end()) {
                qmlWarning(this) << "could not find callback for watcher.";
                return;
            }

            QJSValue v(QLatin1String("timeout"));
            i.value().promise.property(QLatin1String("reject")).call(QJSValueList() << v);

            delete i.key();
            delete i.value().timer;
            m_callbacks.erase(i);
        });

        // handle success
        connect(watcher, &QRemoteObjectPendingCallWatcher::finished, this,
                [this](QRemoteObjectPendingCallWatcher *self) {
                    auto i = m_callbacks.find(self);
                    if (i == m_callbacks.end()) {
                        qmlWarning(this) << "could not find callback for watcher.";
                        return;
                    }
                    QJSValue v = qmlEngine(this)->toScriptValue(self->returnValue());
                    i.value().promise.property(QLatin1String("resolve")).call(QJSValueList() << v);

                    delete i.key();
                    delete i.value().timer;
                    m_callbacks.erase(i);
                });

        response.timer->start(timeout);
        return promise.property(QLatin1String("promise"));
    }

private:
    QHash<QRemoteObjectPendingCallWatcher *, QtQmlRemoteObjectsResponse> m_callbacks;
    QJSValue m_accessiblePromise;
};

struct QRemoteObjectNodeForeign
{
    Q_GADGET
    QML_FOREIGN(QRemoteObjectNode)
    QML_NAMED_ELEMENT(Node)
    QML_ADDED_IN_VERSION(5, 12)
};

struct QRemoteObjectSettingsStoreForeign
{
    Q_GADGET
    QML_FOREIGN(QRemoteObjectSettingsStore)
    QML_NAMED_ELEMENT(SettingsStore)
    QML_ADDED_IN_VERSION(5, 12)
};

struct QRemoteObjectHostForeign
{
    Q_GADGET
    QML_FOREIGN(QRemoteObjectHost)
    QML_NAMED_ELEMENT(Host)
    QML_ADDED_IN_VERSION(5, 15)
};

struct QRemoteObjectAbstractPersistedStoreForeign
{
    Q_GADGET
    QML_FOREIGN(QRemoteObjectAbstractPersistedStore)
    QML_NAMED_ELEMENT(PersistedStore)
    QML_UNCREATABLE("QRemoteObjectAbstractPersistedStore is Abstract")
    QML_ADDED_IN_VERSION(5, 12)
};

QT_END_NAMESPACE

#endif // QREMOTEOBJECTSQML_P_H
