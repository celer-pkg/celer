// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "redditmodel.h"

#include <QCommandLineParser>

#include <QApplication>
#include <QListView>

using namespace Qt::StringLiterals;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCommandLineParser parser;

    const QCommandLineOption clientId(QStringList() << "i"_L1 << "client-id"_L1,
                                      "Specifies the application client id"_L1, "client_id"_L1);

    parser.addOptions({clientId});
    parser.process(app);

    if (parser.isSet(clientId)) {
        QListView view;
        RedditModel model(parser.value(clientId));
        view.setModel(&model);
        view.show();
        return app.exec();
    } else {
        parser.showHelp();
    }
    return 0;
}
