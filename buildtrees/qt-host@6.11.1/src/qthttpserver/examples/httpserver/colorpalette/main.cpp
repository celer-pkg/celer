// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "apibehavior.h"
#include "types.h"
#include "utils.h"
#include <QtCore/QCoreApplication>
#include <QtHttpServer/QHttpServer>

#define SCHEME "http"
#define HOST "127.0.0.1"
#define PORT 49425

template<typename T>
void addCrudRoutes(QHttpServer &httpServer, const QString &apiPath, CrudApi<T> &api,
                   const SessionApi &sessionApi)
{
    //! [GET paginated list example]
    httpServer.route(
            QString("%1").arg(apiPath), QHttpServerRequest::Method::Get,
            [&api](const QHttpServerRequest &request) { return api.getPaginatedList(request); });
    //! [GET paginated list example]

    //! [GET single item example]
    httpServer.route(QString("%1/<arg>").arg(apiPath), QHttpServerRequest::Method::Get,
                     [&api](qint64 itemId) { return api.getItem(itemId); });
    //! [GET single item example]

    //! [POST example]
    httpServer.route(QString("%1").arg(apiPath), QHttpServerRequest::Method::Post,
                     [&api, &sessionApi](const QHttpServerRequest &request) {
                         if (!sessionApi.authorize(request)) {
                             return QHttpServerResponse(
                                     QHttpServerResponder::StatusCode::Unauthorized);
                         }
                         return api.postItem(request);
                     });
    //! [POST example]

    httpServer.route(QString("%1/<arg>").arg(apiPath), QHttpServerRequest::Method::Put,
                     [&api, &sessionApi](qint64 itemId, const QHttpServerRequest &request) {
                         if (!sessionApi.authorize(request)) {
                             return QHttpServerResponse(
                                     QHttpServerResponder::StatusCode::Unauthorized);
                         }
                         return api.updateItem(itemId, request);
                     });

    httpServer.route(QString("%1/<arg>").arg(apiPath), QHttpServerRequest::Method::Patch,
                     [&api, &sessionApi](qint64 itemId, const QHttpServerRequest &request) {
                         if (!sessionApi.authorize(request)) {
                             return QHttpServerResponse(
                                     QHttpServerResponder::StatusCode::Unauthorized);
                         }
                         return api.updateItemFields(itemId, request);
                     });

    httpServer.route(QString("%1/<arg>").arg(apiPath), QHttpServerRequest::Method::Delete,
                     [&api, &sessionApi](qint64 itemId, const QHttpServerRequest &request) {
                         if (!sessionApi.authorize(request)) {
                             return QHttpServerResponse(
                                     QHttpServerResponder::StatusCode::Unauthorized);
                         }
                         return api.deleteItem(itemId);
                     });
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addOptions({
            { "port", QCoreApplication::translate("main", "The port the server listens on."),
              "port" },
    });
    parser.addHelpOption();
    parser.process(app);

    quint16 portArg = PORT;
    if (!parser.value("port").isEmpty())
        portArg = parser.value("port").toUShort();

    auto colorFactory = std::make_unique<ColorFactory>();
    auto colors = tryLoadFromFile<Color>(*colorFactory, ":/assets/colors.json");
    CrudApi<Color> colorsApi(std::move(colors), std::move(colorFactory));

    auto userFactory = std::make_unique<UserFactory>(SCHEME, HOST, portArg);
    auto users = tryLoadFromFile<User>(*userFactory, ":/assets/users.json");
    CrudApi<User> usersApi(std::move(users), std::move(userFactory));

    auto sessionEntryFactory = std::make_unique<SessionEntryFactory>();
    auto sessions = tryLoadFromFile<SessionEntry>(*sessionEntryFactory, ":/assets/sessions.json");
    SessionApi sessionApi(std::move(sessions), std::move(sessionEntryFactory));

    // Setup QHttpServer
    QHttpServer httpServer;
    httpServer.route("/", []() {
        return "Qt Colorpalette example server. Please see documentation for API description";
    });

    addCrudRoutes(httpServer, "/api/unknown", colorsApi, sessionApi);
    addCrudRoutes(httpServer, "/api/users", usersApi, sessionApi);

    // Login resource
    httpServer.route(
            "/api/login", QHttpServerRequest::Method::Post,
            [&sessionApi](const QHttpServerRequest &request) { return sessionApi.login(request); });

    httpServer.route("/api/register", QHttpServerRequest::Method::Post,
                     [&sessionApi](const QHttpServerRequest &request) {
                         return sessionApi.registerSession(request);
                     });

    httpServer.route("/api/logout", QHttpServerRequest::Method::Post,
                     [&sessionApi](const QHttpServerRequest &request) {
                         return sessionApi.logout(request);
                     });

    // Images resource
    httpServer.route("/img/faces/<arg>-image.jpg", QHttpServerRequest::Method::Get,
                     [](qint64 imageId, const QHttpServerRequest &) {
                         return QHttpServerResponse::fromFile(
                                 QString(":/assets/img/%1-image.jpg").arg(imageId));
                     });

    auto tcpserver = std::make_unique<QTcpServer>();
    if (!tcpserver->listen(QHostAddress::Any, portArg) || !httpServer.bind(tcpserver.get())) {
        qDebug() << QCoreApplication::translate("QHttpServerExample",
                                                "Server failed to listen on a port.");
        return 0;
    }
    quint16 port = tcpserver->serverPort();
    tcpserver.release();

    qDebug() << QCoreApplication::translate(
                        "QHttpServerExample",
                        "Running on http://127.0.0.1:%1/ (Press CTRL+C to quit)")
                        .arg(port);

    return app.exec();
}
