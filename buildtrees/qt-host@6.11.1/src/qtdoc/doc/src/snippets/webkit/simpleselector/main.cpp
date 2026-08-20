// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [main program]
#include <QtWidgets>
#include "window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Window window;
    window.setUrl(QUrl("http://www.webkit.org"));
    window.show();
    return app.exec();
}
//! [main program]
