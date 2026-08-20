// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef REFUSEWRITE_H
#define REFUSEWRITE_H

#include <QtCore/qobject.h>
#include <QtCore/qqueue.h>
#include <QtCore/qvariant.h>
#include <QtCore/qcoreevent.h>
#include <QtQmlIntegration/qqmlintegration.h>

class RefuseWrite : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QVariantList things READ things WRITE setThings NOTIFY thingsChanged)

public:
    RefuseWrite(QObject *parent = nullptr) : QObject(parent) {
        m_things.enqueue(QVariantList());
    }

    QVariantList things() const
    {
        return m_things.head();
    }

    void setThings(const QVariantList &newThings)
    {
        m_things.enqueue(newThings);
        startTimer(1);
    }

    qsizetype pendingChanges() const
    {
        return m_things.size() - 1;
    }

private:
    void timerEvent(QTimerEvent *event) override
    {
        if (m_things.size() < 2) {
            killTimer(event->id());
            return;
        }

        const QVariantList old = m_things.dequeue();
        if (old != m_things.head())
            emit thingsChanged();
    }

signals:
    void thingsChanged();

private:
    QQueue<QVariantList> m_things;
};

#endif // REFUSEWRITE_H




