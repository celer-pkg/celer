// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author
// Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQMLWEBCHANNEL_H
#define QQMLWEBCHANNEL_H

#include <QtWebChannelQuick/qwebchannelquickglobal.h>
#include <QtWebChannel/qwebchannel.h>

#include <QtQml/qqml.h>
#include <QtQml/QQmlListProperty>

QT_BEGIN_NAMESPACE

class QQmlWebChannelPrivate;
class QQmlWebChannelAttached;
class Q_WEBCHANNELQUICK_EXPORT QQmlWebChannel : public QWebChannel
{
    Q_OBJECT
    Q_DISABLE_COPY(QQmlWebChannel)

    Q_PROPERTY(QQmlListProperty<QObject> transports READ transports)
    Q_PROPERTY(QQmlListProperty<QObject> registeredObjects READ registeredObjects)
    QML_NAMED_ELEMENT(WebChannel)
    QML_ATTACHED(QQmlWebChannelAttached)
    QML_ADDED_IN_VERSION(1, 0)
public:
    explicit QQmlWebChannel(QObject *parent = nullptr);
    virtual ~QQmlWebChannel();

    Q_INVOKABLE void registerObjects(const QVariantMap &objects);
    QQmlListProperty<QObject> registeredObjects();

    QQmlListProperty<QObject> transports();

    static QQmlWebChannelAttached *qmlAttachedProperties(QObject *obj);

    Q_INVOKABLE void connectTo(QObject *transport);
    Q_INVOKABLE void disconnectFrom(QObject *transport);

private:
    Q_DECLARE_PRIVATE(QQmlWebChannel)
    Q_PRIVATE_SLOT(d_func(), void _q_objectIdChanged(const QString &newId))

    static void registeredObjects_append(QQmlListProperty<QObject> *prop, QObject *item);
    static qsizetype registeredObjects_count(QQmlListProperty<QObject> *prop);
    static QObject *registeredObjects_at(QQmlListProperty<QObject> *prop, qsizetype index);
    static void registeredObjects_clear(QQmlListProperty<QObject> *prop);

    static void transports_append(QQmlListProperty<QObject> *prop, QObject *item);
    static qsizetype transports_count(QQmlListProperty<QObject> *prop);
    static QObject *transports_at(QQmlListProperty<QObject> *prop, qsizetype index);
    static void transports_clear(QQmlListProperty<QObject> *prop);
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQmlWebChannel)
QML_DECLARE_TYPEINFO(QQmlWebChannel, QML_HAS_ATTACHED_PROPERTIES)

#endif // QQMLWEBCHANNEL_H
