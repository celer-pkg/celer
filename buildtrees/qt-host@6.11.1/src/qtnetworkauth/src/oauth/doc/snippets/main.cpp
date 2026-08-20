// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "src_oauth_replyhandlers_p.h"

#include <QtWidgets/qapplication.h>

#ifdef QT_QML_LIB
#include <QtQml/qqmlapplicationengine.h>
#endif

#include <QtCore/qloggingcategory.h>
#include <QtCore/qcommandlineparser.h>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QLoggingCategory::setFilterRules(u"qt.networkauth* = true"_s);
    QApplication app(argc, argv);
#ifdef QT_QML_LIB
    QQmlApplicationEngine engine;
#endif

    QCommandLineParser parser;
    // For example: ./networkauth_oauth_snippets --agent widgets --scheme https
    QCommandLineOption agentOption({u"agent"_s},
                    u"Whether to use WebEngine 'widgets', WebEngine 'qml', or 'system' Browser"_s,
                    u"Agent"_s, u"system"_s);
    QCommandLineOption schemeOption({u"scheme"_s}, u"Whether to use 'http', 'https', 'custom', "_s
                                                   u"'deviceflow', or 'https-localhost'"_s,
                                                   u"URI scheme"_s, u"http"_s);
    parser.addOptions({{agentOption}, {schemeOption}});
    parser.addHelpOption();
    parser.process(app);
    auto agent = parser.value(agentOption);
    auto scheme = parser.value(schemeOption);

    HttpExample httpExample;
    UriSchemeExample uriSchemeExample;

    if (agent == u"qml"_s && scheme == u"http"_s) {
#ifdef QT_WEBENGINEQUICK_LIB
        QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                         []() { QCoreApplication::exit(1); }, Qt::QueuedConnection);
        engine.loadFromModule("OAuthSnippets", "MainWindow");
#else
        qWarning("QtWebEngine not available");
#endif
    } else if (agent == u"system"_s && scheme == u"http"_s) {
        httpExample.setupSystemBrowser();
    } else if (agent == u"system"_s && scheme == u"https-localhost"_s) {
        httpExample.setupSystemBrowserLocalHostHttps();
    } else if (agent == u"system"_s && scheme == u"custom"_s) {
        uriSchemeExample.setupSystemBrowserCustom();
    } else if (agent == u"widgets"_s && scheme == u"custom") {
        uriSchemeExample.setupWebEngineWidgetsCustom();
    } else if (agent == u"widgets"_s && scheme == u"https") {
        uriSchemeExample.setupWebEngineWidgetsHttps();
    } else if (agent == u"widgets"_s && scheme == u"http"_s) {
        httpExample.setupWebEngineWidgets();
    } else if (scheme == u"deviceflow"_s) {
        httpExample.setupDeviceFlow();
    } else {
        qDebug() << "Currently unsupported option combination:" << agent << scheme;
        return EXIT_FAILURE;
    }
    return app.exec();
}
