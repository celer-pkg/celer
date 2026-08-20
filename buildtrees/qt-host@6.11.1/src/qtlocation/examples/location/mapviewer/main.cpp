// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>

#include <QtGui/QGuiApplication>

#if QT_CONFIG(ssl)
#include <QtNetwork/QSslSocket>
#endif

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
#if QT_CONFIG(library)
    const QString additionalLibraryPaths = qEnvironmentVariable("QTLOCATION_EXTRA_LIBRARY_PATH");
    for (const auto &p : additionalLibraryPaths.split(u':', Qt::SkipEmptyParts))
        QCoreApplication::addLibraryPath(p);
#endif

    QGuiApplication application(argc, argv);
    QCoreApplication::setApplicationName(u"QtLocation Mapviewer example"_s);
    QGuiApplication::setDesktopFileName(QCoreApplication::applicationName());

    QStringList args = QCoreApplication::arguments();
    args.removeFirst();
    if (args.contains(u"--help")) {
        std::cout << qPrintable(QCoreApplication::applicationName()) << "\n\n" << help;
        return 0;
    }

    QVariantMap parameters = parseArgs(args);
    if (!parameters.contains(u"osm.useragent"_s))
        parameters.insert(u"osm.useragent"_s, QCoreApplication::applicationName());

    QQmlApplicationEngine engine;
#if QT_CONFIG(ssl)
    engine.rootContext()->setContextProperty("supportsSsl", QSslSocket::supportsSsl());
#else
    engine.rootContext()->setContextProperty("supportsSsl", false);
#endif
    engine.loadFromModule("MapViewer", "Main");

    auto *item = engine.rootObjects().value(0);
    if (item == nullptr)
        return -1;

    QMetaObject::invokeMethod(item, "initializeProviders", QVariant::fromValue(parameters));

    return application.exec();
}
