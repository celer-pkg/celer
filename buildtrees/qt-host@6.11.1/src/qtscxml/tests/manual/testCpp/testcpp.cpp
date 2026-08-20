// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QElapsedTimer>
#include "out.h"

#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QElapsedTimer t;
    t.start();

    StateMachine sm;
    sm.init();

    std::cout << "instantiation:" << t.elapsed() << " ms" << std::endl;
    return 0;
}
