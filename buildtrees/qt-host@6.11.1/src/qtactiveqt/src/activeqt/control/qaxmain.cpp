// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#include <qapplication.h>
#include <qaxfactory.h>

int main(int argc, char **argv)
{
    QT_USE_NAMESPACE
    QAxFactory::startServer();
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    return app.exec();
}
