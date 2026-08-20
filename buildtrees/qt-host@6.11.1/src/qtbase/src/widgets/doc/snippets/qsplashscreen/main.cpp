// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtGui>
#include <QtWidgets>
#include <stdlib.h>

//! [0]
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QPixmap pixmap(":/splash.png");
    QSplashScreen splash(pixmap);
    splash.show();
    app.processEvents();
    //...
    QMainWindow window;
    window.show();
    splash.finish(&window);
    return app.exec();
}
//! [0]

void example()
{
    //! [2]
    QScreen *screen = QGuiApplication::screens().at(1);
    QPixmap pixmap(":/splash.png");
    QSplashScreen splash(screen, pixmap);
    splash.show();
    //! [2]
}
