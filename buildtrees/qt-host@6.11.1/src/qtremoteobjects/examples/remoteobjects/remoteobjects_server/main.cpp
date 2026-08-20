// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "timemodel.h"

#include <QCoreApplication>
/*
* http://stackoverflow.com/questions/7404163/windows-handling-ctrlc-in-different-thread
*/

void SigIntHandler()
{
    qDebug()<<"Ctrl-C received.  Quitting.";
    qApp->quit();
}

#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX) || defined(Q_OS_QNX)
  #include <signal.h>

  void unix_handler(int s)
  {
    if (s==SIGINT)
        SigIntHandler();
  }

#elif defined(Q_OS_WIN32)
  #include <windows.h>

  BOOL WINAPI WinHandler(DWORD CEvent)
  {
    switch (CEvent)
    {
    case CTRL_C_EVENT:
        SigIntHandler();
        break;
    }
    return TRUE;
  }
#endif

//![0]
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    #if defined(Q_OS_UNIX) || defined(Q_OS_LINUX) || defined(Q_OS_QNX)
        signal(SIGINT, &unix_handler);
    #elif defined(Q_OS_WIN32)
        SetConsoleCtrlHandler((PHANDLER_ROUTINE)WinHandler, TRUE);
    #endif
    QRemoteObjectHost node(QUrl(QStringLiteral("local:replica")),QUrl(QStringLiteral("local:registry")));
    QRemoteObjectRegistryHost node2(QUrl(QStringLiteral("local:registry")));
    MinuteTimer timer;
    node2.enableRemoting(&timer);

    Q_UNUSED(timer)
    return app.exec();
}
//![0]
