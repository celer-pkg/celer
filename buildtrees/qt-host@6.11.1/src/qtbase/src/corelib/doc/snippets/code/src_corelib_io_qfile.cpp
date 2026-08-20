// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QFile>
#include <QDir>

void example()
{
    //! [0]
    QFile file;
    QDir::setCurrent("/tmp");
    file.setFileName("readme.txt");
    QDir::setCurrent("/home");
    file.open(QIODevice::ReadOnly);      // opens "/home/readme.txt" under Unix
    //! [0]
}


//! [3]
#include <stdio.h>

void printError(const char* msg)
{
    QFile file;
    file.open(stderr, QIODevice::WriteOnly);
    file.write(msg, qstrlen(msg));        // write to stderr
    file.close();
}
//! [3]


#if 0
//! [4]
CONFIG += console
//! [4]
#endif
