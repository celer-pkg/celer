// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


#include <stdlib.h>
#include <stdio.h>

#include <QtCore/QCoreApplication>
#include <QtCore5Compat/qtextcodec.h>

int main(int argc, char **argv)
{
    qputenv("LC_ALL", "C");
    QCoreApplication app(argc, argv);

    QString string(QChar(0x410));
    QTextCodec *locale = QTextCodec::codecForLocale();
    QTextEncoder *encoder = locale->makeEncoder();
    QByteArray output = encoder->fromUnicode(string);
    printf("%s\n", output.data());
    delete encoder;

    return 0;
}
