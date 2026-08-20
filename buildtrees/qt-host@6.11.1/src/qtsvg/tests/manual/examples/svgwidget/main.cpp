// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QStringList>
#include <QTabWidget>
#include <QSvgWidget>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("SVG Widget");
    QGuiApplication::setApplicationDisplayName(QCoreApplication::applicationName());
    QCoreApplication::setOrganizationName("QtProject");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription("Qt SVG Widget");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file to open.");
    parser.process(app);

    const QString fileName = parser.positionalArguments().value(0, QLatin1String(":/files/bubbles.svg"));

    QTabWidget top;

    auto widget = new QSvgWidget(&top);
    top.addTab(widget, "Bubbles");
    widget->load(fileName);

    auto otherWidget = new QSvgWidget(&top);
    top.addTab(otherWidget, "Spheres");
    otherWidget->load(QLatin1String(":/files/spheres.svg"));

    top.show();
    return app.exec();
}
