// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author
// Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQMLWEBCHANNELATTACHED_H
#define QQMLWEBCHANNELATTACHED_H

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

#include <QObject>
#include <QtQml/qqml.h>
#include <QtWebChannelQuick/qwebchannelquickglobal.h>

QT_BEGIN_NAMESPACE

class Q_WEBCHANNELQUICK_EXPORT QQmlWebChannelAttached : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged FINAL)
    QML_ANONYMOUS
    QML_ADDED_IN_VERSION(1, 0)
public:
    explicit QQmlWebChannelAttached(QObject *parent = 0);
    virtual ~QQmlWebChannelAttached();

    QString id() const;
    void setId(const QString &id);

Q_SIGNALS:
    void idChanged(const QString &id);

private:
    QString m_id;
};

QT_END_NAMESPACE

#endif // QQMLWEBCHANNELATTACHED_H
