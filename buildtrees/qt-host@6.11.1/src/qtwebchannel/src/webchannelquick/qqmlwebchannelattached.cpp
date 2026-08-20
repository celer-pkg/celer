// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author
// Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qqmlwebchannelattached_p.h"

QT_USE_NAMESPACE

QQmlWebChannelAttached::QQmlWebChannelAttached(QObject *parent) : QObject(parent) { }

QQmlWebChannelAttached::~QQmlWebChannelAttached() { }

/*!
    \qmlattachedproperty string WebChannel::id

    \brief The identifier under which an object, registered to a WebChannel, is known to remote
    clients.

    This property must be set for every object that should be published over the WebChannel.
    While no restrictions are enforced on the format of the id, it is usually a good idea to
    choose a string that is also a valid JavaScript identifier.
*/
QString QQmlWebChannelAttached::id() const
{
    return m_id;
}

void QQmlWebChannelAttached::setId(const QString &id)
{
    if (id != m_id) {
        m_id = id;
        emit idChanged(id);
    }
}
