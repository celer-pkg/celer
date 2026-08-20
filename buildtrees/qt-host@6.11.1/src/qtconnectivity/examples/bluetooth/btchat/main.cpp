// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "chat.h"

#include <QLoggingCategory>

#include <QApplication>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    // QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    QApplication app(argc, argv);

    QIcon::setThemeName(u"btchat"_s);

    Chat d;
    QObject::connect(&d, &Chat::accepted, &app, &QApplication::quit);

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    d.showMaximized();
#else
    d.show();
#endif

    app.exec();

    return 0;
}

