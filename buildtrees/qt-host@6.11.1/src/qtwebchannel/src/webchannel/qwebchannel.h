// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QWEBCHANNEL_H
#define QWEBCHANNEL_H

#include <QtCore/QObject>
#include <QtCore/QJsonValue>

#include <QtWebChannel/qwebchannelglobal.h>

class tst_bench_QWebChannel;

QT_BEGIN_NAMESPACE

class QWebChannelPrivate;
class QWebChannelAbstractTransport;

class Q_WEBCHANNEL_EXPORT QWebChannel : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QWebChannel)
    Q_PROPERTY(bool blockUpdates READ blockUpdates WRITE setBlockUpdates NOTIFY blockUpdatesChanged
                       BINDABLE bindableBlockUpdates)
    Q_PROPERTY(int propertyUpdateInterval READ propertyUpdateInterval WRITE
                       setPropertyUpdateInterval BINDABLE bindablePropertyUpdateInterval)
public:
    explicit QWebChannel(QObject *parent = nullptr);
    ~QWebChannel();

    void registerObjects(const QHash<QString, QObject*> &objects);
    QHash<QString, QObject*> registeredObjects() const;
    Q_INVOKABLE void registerObject(const QString &id, QObject *object);
    Q_INVOKABLE void deregisterObject(QObject *object);

    bool blockUpdates() const;
    void setBlockUpdates(bool block);
    QBindable<bool> bindableBlockUpdates();

    int propertyUpdateInterval() const;
    void setPropertyUpdateInterval(int ms);
    QBindable<int> bindablePropertyUpdateInterval();

Q_SIGNALS:
    void blockUpdatesChanged(bool block);

public Q_SLOTS:
    void connectTo(QWebChannelAbstractTransport *transport);
    void disconnectFrom(QWebChannelAbstractTransport *transport);

private:
    Q_DECLARE_PRIVATE(QWebChannel)
    QWebChannel(QWebChannelPrivate &dd, QObject *parent = nullptr);
    Q_PRIVATE_SLOT(d_func(), void _q_transportDestroyed(QObject*))

    friend class QMetaObjectPublisher;
    friend class QQmlWebChannel;
    friend class TestWebChannel;
    friend class ::tst_bench_QWebChannel;
};

QT_END_NAMESPACE

#endif // QWEBCHANNEL_H

