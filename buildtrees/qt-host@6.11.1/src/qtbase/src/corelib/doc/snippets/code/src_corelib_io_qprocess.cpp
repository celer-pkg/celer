// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QProcess>
#include <QDebug>
#include <QtCore/qglobal.h>

void wrapInFunction()
{

    //! [0]
    QProcess builder;
    builder.setProcessChannelMode(QProcess::MergedChannels);
    builder.start("make", QStringList() << "-j2");

    if (!builder.waitForFinished())
        qDebug() << "Make failed:" << builder.errorString();
    else
        qDebug() << "Make output:" << builder.readAll();
    //! [0]


    //! [1]
    QProcess more;
    more.start("more");
    more.write("Text to display");
    more.closeWriteChannel();
    // QProcess will emit readyRead() once "more" starts printing
    //! [1]


# if 0
    //! [2]
    command1 | command2
    //! [2]
#endif


    //! [3]
    QProcess process1;
    QProcess process2;

    process1.setStandardOutputProcess(&process2);

    process1.start("command1");
    process2.start("command2");
    //! [3]
}


#if defined(Q_OS_UNIX)

#include <unistd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/types.h>

gid_t safeGid;
uid_t safeUid;

//! [4]
void runSandboxed(const QString &name, const QStringList &arguments)
{
    QProcess proc;
    proc.setChildProcessModifier([] {
        // Drop all privileges in the child process, and enter
        // a chroot jail.
        ::setgroups(0, nullptr);
        ::chroot("/run/safedir");
        ::chdir("/");
        ::setgid(safeGid);
        ::setuid(safeUid);
        ::umask(077);
    });
    proc.start(name, arguments);
    proc.waitForFinished();
}
//! [4]
#endif

void examples()
{
    {
        //! [5]
        QProcess process;
        process.startCommand("del /s *.txt");
        // same as process.start("del", QStringList() << "/s" << "*.txt");
        //...
        //! [5]
    }

    {
        //! [6]
        QProcess process;
        process.startCommand("dir \"My Documents\"");
        //! [6]
    }

    {
        //! [7]
        QProcess process;
        process.startCommand("dir \"Epic 12\"\"\" Singles\"");
        //! [7]
    }

    {
        //! [8]
        QStringList environment = QProcess::systemEnvironment();
        // environment = {"PATH=/usr/bin:/usr/local/bin",
        //                "USER=greg", "HOME=/home/greg"}
        //! [8]
    }
}
