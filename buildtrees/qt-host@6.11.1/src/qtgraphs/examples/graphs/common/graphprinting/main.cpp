// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtGui/qguiapplication.h>
#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlengine.h>
#include <QtQuick/qquickview.h>
#include "graphprinter.h"

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickView viewer;

    viewer.setTitle(u"Graph Printing"_s);

    //! [0]
    GraphPrinter graphPrinter;
    viewer.rootContext()->setContextProperty(u"graphPrinter"_s, &graphPrinter);
    //! [0]

    viewer.setMinimumSize({1280, 720});
    viewer.loadFromModule(u"GraphPrintingExample"_s, u"Main"_s);
    viewer.setResizeMode(QQuickView::SizeRootObjectToView);
    viewer.setColor(Qt::white);
    viewer.show();

    return QCoreApplication::exec();
}
