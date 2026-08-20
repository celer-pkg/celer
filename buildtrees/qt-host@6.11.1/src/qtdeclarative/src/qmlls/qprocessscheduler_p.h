// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPROCESSSCHEDULER_P_H
#define QPROCESSSCHEDULER_P_H

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

#include <QtCore/qobject.h>
#include <QtCore/qprocess.h>
#include <QtCore/qlist.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qqueue.h>

QT_BEGIN_NAMESPACE
namespace QmlLsp {

class QProcessScheduler : public QObject
{
    Q_OBJECT
public:
    using Id = QByteArray;
    struct Command
    {
        QString program;
        QStringList arguments;
        QProcessEnvironment customEnvironment = QProcessEnvironment::systemEnvironment();

        friend bool comparesEqual(const Command &a, const Command &b) noexcept
        {
            return a.program == b.program && a.arguments == b.arguments
                    && a.customEnvironment == b.customEnvironment;
        }
        Q_DECLARE_EQUALITY_COMPARABLE(Command)
    };

    using QueueElement = std::variant<Command, Id>;

    QProcessScheduler();
    ~QProcessScheduler();

Q_SIGNALS:
    void done(const Id &id);

public Q_SLOTS:
    void schedule(const QList<Command> &commands, const Id &id);

private Q_SLOTS:
    void processNext();
    void onErrorOccurred(QProcess::ProcessError error);

private:
    QQueue<QueueElement> m_queue;
    QProcess m_process;
    bool m_isRunning = false;
};

} // namespace QmlLsp

QT_END_NAMESPACE

#endif // QPROCESSSCHEDULER_P_H
