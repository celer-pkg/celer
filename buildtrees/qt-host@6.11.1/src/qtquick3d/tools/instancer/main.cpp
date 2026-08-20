// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtQuick3D/private/qquick3dinstancing_p.h>

extern void qt_writeInstanceTable(QIODevice *out, QQuick3DInstancing &instanceTable);

int main(int argc, char **argv)
{
    QQuick3DFileInstancing instanceTable;
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s INFILE [OUTFILE]\n", argv[0]);
        return -1;
    }
    QString inFilename = QString::fromLocal8Bit(argv[1]);
    if (!instanceTable.loadFromXmlFile(inFilename)) {
        fprintf(stderr, "Could not read instance table %s\n", argv[1]);
        return -2;
    }

    QString outFilename = argc > 2 ? QString::fromLocal8Bit(argv[2]) : inFilename + QStringLiteral(".bin");
    QFile outFile(outFilename);
    if (!outFile.open(QFile::WriteOnly)) {
        fprintf(stderr, "Could not open %s for writing.\n", qPrintable(outFilename));
        return -2;
    }

    int instanceCount = instanceTable.writeToBinaryFile(&outFile);

    outFile.close();

    if (instanceCount < 0) {
        fprintf(stderr, "Could not write instance table.\n");
        return -3;
    }

    fprintf(stderr, "Wrote %d instances to %s.\n", instanceCount, qPrintable(outFilename));

    return 0;
}
