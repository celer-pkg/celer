// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore/QUrl>
#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include <QGuiApplication>
#include <QStyleHints>
#include <QScreen>
#include <QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtWebView/QtWebView>

using namespace Qt::StringLiterals;

// Workaround: As of Qt 5.4 QtQuick does not expose QUrl::fromUserInput.
class Utils : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    Q_INVOKABLE static QUrl fromUserInput(const QString &userInput);
};

QUrl Utils::fromUserInput(const QString &userInput)
{
    if (!userInput.isEmpty()) {
        if (const QUrl result = QUrl::fromUserInput(userInput); result.isValid())
            return result;
    }
    return QUrl::fromUserInput("about:blank"_L1);
}

#include "main.moc"

int main(int argc, char *argv[])
{
//! [0]
    QtWebView::initialize();
    QGuiApplication app(argc, argv);
//! [0]
    QGuiApplication::setApplicationDisplayName(QCoreApplication::translate("main",
                                                                           "QtWebView Example"));
    QCommandLineParser parser;
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    parser.setApplicationDescription(QGuiApplication::applicationDisplayName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("url"_L1, "The initial URL to open."_L1);
    parser.process(QCoreApplication::arguments());
    const QString initialUrl = parser.positionalArguments().value(0, "https://www.qt.io"_L1);

    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();
    context->setContextProperty("utils"_L1, new Utils(&engine));
    context->setContextProperty("initialUrl"_L1,
                                Utils::fromUserInput(initialUrl));

    QRect geometry = QGuiApplication::primaryScreen()->availableGeometry();
    if (!QGuiApplication::styleHints()->showIsFullScreen()) {
        const QSize size = geometry.size() * 4 / 5;
        const QSize offset = (geometry.size() - size) / 2;
        const QPoint pos = geometry.topLeft() + QPoint(offset.width(), offset.height());
        geometry = QRect(pos, size);
    }

    engine.setInitialProperties(QVariantMap{{"x"_L1, geometry.x()},
                                            {"y"_L1, geometry.y()},
                                            {"width"_L1, geometry.width()},
                                            {"height"_L1, geometry.height()}});

    engine.load(QUrl("qrc:/main.qml"_L1));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
