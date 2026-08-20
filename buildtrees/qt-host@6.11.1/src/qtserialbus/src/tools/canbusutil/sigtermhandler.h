// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:significant reason:default

#ifndef SIGTERMHANDLER_H
#define SIGTERMHANDLER_H

#include <QObject>

class SigTermHandler : public QObject
{
    Q_OBJECT
public:
    static SigTermHandler *instance();
    static void handle(int s);

    virtual ~SigTermHandler();
signals:
    void sigTermSignal();

private:
    explicit SigTermHandler(QObject *parent = nullptr);
};

#endif // SIGTERMHANDLER_H
