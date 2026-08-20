// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>

#include <QtGui/QGuiApplication>

#include <iostream>

using namespace Qt::StringLiterals;

static const char *help = R"(Usage:
--plugin.<parameter_name> <parameter_value>    -  Sets parameter = value for plugin
)";

static QVariantMap parseArgs(QStringList args)
{
    QVariantMap parameters;
    while (!args.isEmpty()) {
        QString param = args.takeFirst();
        if (param.startsWith(u"--plugin.")) {
            param.remove(0, 9);
            if (args.isEmpty() || args.constFirst().startsWith(u"--")) {
                parameters.insert(param, QVariant(true));
            } else {
                QString value = args.takeFirst();
                if (value == u"true" || value == u"on" || value == u"enabled") {
                    parameters.insert(param, QVariant(true));
                } else if (value == u"false" || value == u"off"
                           || value == u"disable") {
                    parameters.insert(param, QVariant(false));
                } else {
                    parameters.insert(param, QVariant(value));
                }
            }
        }
    }
    return parameters;
}

int main(int argc, char *argv[])
{
    QGuiApplication application(argc, argv);

    QStringList args = QCoreApplication::arguments();
    args.removeFirst();
    if (args.contains(u"--help")) {
        std::cout << qPrintable(QCoreApplication::applicationName()) << "\n\n" << help;
        return 0;
    }

    QVariantMap parameters = parseArgs(args);

    QQmlApplicationEngine engine;
    engine.addImportPath(u":/imports"_s);
    engine.load(QUrl(u"qrc:///places.qml"_s));
    QObject::connect(&engine, &QQmlApplicationEngine::quit,
                     qApp, QCoreApplication::quit);

    auto *item = engine.rootObjects().value(0);
    if (item == nullptr)
        return -1;

    QMetaObject::invokeMethod(item, "initializeProviders", QVariant::fromValue(parameters));

    return application.exec();
}
