// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtScxml/qscxmltabledata.h>
#include "qscxmlc.h"

#include <QCoreApplication>
#include <QStringList>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setApplicationVersion(QString::fromLatin1("%1 (Qt %2)").arg(
                            QString::number(Q_QSCXMLC_OUTPUT_REVISION),
                            QString::fromLatin1(QT_VERSION_STR)));
    return run(QCoreApplication::arguments());
}
