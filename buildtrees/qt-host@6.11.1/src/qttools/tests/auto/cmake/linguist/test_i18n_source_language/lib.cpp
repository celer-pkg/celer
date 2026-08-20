// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QCoreApplication>
#include <QDebug>

void printStuff()
{
    qDebug()
        << QObject::tr("We must not see this in the source language's .ts file.")
        << QObject::tr("%n argument(s) passed", "", 156);
}
