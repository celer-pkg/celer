// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/qfile.h>
#include <QtCore/qstring.h>
#include <QtCore/qdebug.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
        return EXIT_FAILURE;

    QFile f(QString::fromUtf8(argv[1]));
    if (!f.open(QFile::ReadWrite | QFile::Text | QFile::Append))
        return EXIT_FAILURE;

    f.write("X\n");

    return EXIT_SUCCESS;
}
