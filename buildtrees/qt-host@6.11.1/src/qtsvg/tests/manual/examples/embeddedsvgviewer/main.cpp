// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>
#include <QDebug>

#include "embeddedsvgviewer.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QString filePath;

    if (argc == 1)
      filePath = QLatin1String(":/files/default.svg");
    else if (argc == 2)
      filePath = argv[1];
    else {
        qDebug() << QLatin1String("Please specify an svg file!");
        return -1;
    }

    EmbeddedSvgViewer viewer(filePath);

    viewer.showFullScreen();

#ifdef QT_KEYPAD_NAVIGATION
    QApplication::setNavigationMode(Qt::NavigationModeCursorAuto);
#endif
    return app.exec();
} 
