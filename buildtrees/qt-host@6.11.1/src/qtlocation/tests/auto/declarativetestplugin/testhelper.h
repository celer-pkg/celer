// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <QObject>
#include <QSignalSpy>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSysInfo>

QT_BEGIN_NAMESPACE

class TestHelper: public QObject
{
    Q_OBJECT
public:
    TestHelper(QObject *parent = nullptr):QObject(parent){}
    Q_INVOKABLE bool waitForPolished(QQuickItem *item, int timeout = 10000) const
    {
        QSignalSpy spy(item->window(), &QQuickWindow::afterAnimating);
        return spy.wait(timeout);
    }

    Q_INVOKABLE int x86Bits() const
    {
        if ( QSysInfo::currentCpuArchitecture() == "x86_64" )
            return 64;
        else
            return 32;
    }
};

QT_END_NAMESPACE

#endif // TESTHELPER_H
