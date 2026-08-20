// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TESTWEBCHANNEL_H
#define TESTWEBCHANNEL_H

#include <QtWebChannelQuick/qqmlwebchannel.h>

QT_BEGIN_NAMESPACE

class TestWebChannel : public QQmlWebChannel
{
    Q_OBJECT

public:
    explicit TestWebChannel(QObject *parent = 0);
    virtual ~TestWebChannel();

    Q_INVOKABLE bool clientIsIdle() const;
};

QT_END_NAMESPACE

#endif // TESTWEBCHANNEL_H
