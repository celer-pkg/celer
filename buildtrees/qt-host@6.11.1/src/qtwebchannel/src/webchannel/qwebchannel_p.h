// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QWEBCHANNEL_P_H
#define QWEBCHANNEL_P_H

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

#include "qwebchannelglobal.h"

#include <QtCore/private/qobject_p.h>
#include <QList>

QT_BEGIN_NAMESPACE

class QJsonValue;
class QWebChannelAbstractTransport;
class QMetaObjectPublisher;

class Q_WEBCHANNEL_EXPORT QWebChannelPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWebChannel)
public:
    QList<QWebChannelAbstractTransport *> transports;
    QMetaObjectPublisher *publisher;

    void init();

    void _q_transportDestroyed(QObject* object);
};

QT_END_NAMESPACE

#endif // QWEBCHANNEL_P_H
