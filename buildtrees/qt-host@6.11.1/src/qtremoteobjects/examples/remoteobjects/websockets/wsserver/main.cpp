// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include <QTreeView>
#include <QApplication>
#include <QRemoteObjectNode>
#include <QTimer>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QWebSocket>
#include <QWebSocketServer>

#ifndef QT_NO_SSL
# include <QFile>
# include <QSslConfiguration>
# include <QSslKey>
#endif

#include "websocketiodevice.h"

struct TimerHandler : public QObject
{
    Q_OBJECT
public:
    QStandardItemModel *model;
public Q_SLOTS:
    void changeData() {
        Q_ASSERT(model);
        Q_ASSERT(model->rowCount() > 50);
        Q_ASSERT(model->columnCount() > 1);
        for (int i = 10; i < 50; ++i)
            model->setData(model->index(i, 1), QColor(Qt::blue), Qt::BackgroundRole);
    }
    void insertData() {
        Q_ASSERT(model);
        Q_ASSERT(model->rowCount() > 50);
        Q_ASSERT(model->columnCount() > 1);
        model->insertRows(2, 9);
        for (int i = 2; i < 11; ++i) {
            model->setData(model->index(i, 1), QColor(Qt::green), Qt::BackgroundRole);
            model->setData(model->index(i, 1), QLatin1String("InsertedRow"), Qt::DisplayRole);
        }
    }
    void removeData() {
        model->removeRows(2, 4);
    }

    void changeFlags() {
        QStandardItem *item = model->item(0, 0);
        item->setEnabled(false);
        item = item->child(0, 0);
        item->setFlags(item->flags() & Qt::ItemIsSelectable);
    }

    void moveData() {
        model->moveRows(QModelIndex(), 2, 4, QModelIndex(), 10);
    }
};

QList<QStandardItem*> addChild(int numChildren, int nestingLevel)
{
    QList<QStandardItem*> result;
    if (nestingLevel == 0)
        return result;
    for (int i = 0; i < numChildren; ++i) {
        QStandardItem *child = new QStandardItem(QStringLiteral("Child num %1, nesting Level %2").arg(i+1).arg(nestingLevel));
        if (i == 0)
            child->appendRow(addChild(numChildren, nestingLevel -1));
        result.push_back(child);
    }
    return result;
}

//! [0]
int main(int argc, char *argv[])
{
    QLoggingCategory::setFilterRules("qt.remoteobjects.debug=false\n"
                                     "qt.remoteobjects.warning=false");
    QApplication app(argc, argv);

    const int modelSize = 100000;
    QStringList list;
    QStandardItemModel sourceModel;
    QStringList hHeaderList;
    hHeaderList << QStringLiteral("First Column with spacing") << QStringLiteral("Second Column with spacing");
    sourceModel.setHorizontalHeaderLabels(hHeaderList);
    list.reserve(modelSize);
    for (int i = 0; i < modelSize; ++i) {
        QStandardItem *firstItem = new QStandardItem(QStringLiteral("FancyTextNumber %1").arg(i));
        if (i == 0)
            firstItem->appendRow(addChild(2, 2));
        QStandardItem *secondItem = new QStandardItem(QStringLiteral("FancyRow2TextNumber %1").arg(i));
        if (i % 2 == 0)
            firstItem->setBackground(Qt::red);
        QList<QStandardItem*> row;
        row << firstItem << secondItem;
        sourceModel.invisibleRootItem()->appendRow(row);
        //sourceModel.appendRow(row);
        list << QStringLiteral("FancyTextNumber %1").arg(i);
    }
    //! [0]

    // Needed by QMLModelViewClient
    QHash<int,QByteArray> roleNames = {
        {Qt::DisplayRole, "_text"},
        {Qt::BackgroundRole, "_color"}
    };
    sourceModel.setItemRoleNames(roleNames);

    QList<int> roles;
    roles << Qt::DisplayRole << Qt::BackgroundRole;

    //! [1]
    QWebSocketServer webSockServer{QStringLiteral("WS QtRO"), QWebSocketServer::NonSecureMode};
    webSockServer.listen(QHostAddress::Any, 8088);

    QRemoteObjectHost hostNode;
    hostNode.setHostUrl(webSockServer.serverAddress().toString(), QRemoteObjectHost::AllowExternalRegistration);

    hostNode.enableRemoting(&sourceModel, QStringLiteral("RemoteModel"), roles);
    //! [1]

    //! [2]
    QObject::connect(&webSockServer, &QWebSocketServer::newConnection, &hostNode, [&hostNode, &webSockServer]{
        while (auto conn = webSockServer.nextPendingConnection()) {
#ifndef QT_NO_SSL
            // Always use secure connections when available
            QSslConfiguration sslConf;
            QFile certFile(QStringLiteral(":/sslcert/server.crt"));
            if (!certFile.open(QIODevice::ReadOnly))
                qFatal("Can't open client.crt file");
            sslConf.setLocalCertificate(QSslCertificate{certFile.readAll()});

            QFile keyFile(QStringLiteral(":/sslcert/server.key"));
            if (!keyFile.open(QIODevice::ReadOnly))
                qFatal("Can't open client.key file");
            sslConf.setPrivateKey(QSslKey{keyFile.readAll(), QSsl::Rsa});

            sslConf.setPeerVerifyMode(QSslSocket::VerifyPeer);
            conn->setSslConfiguration(sslConf);
            QObject::connect(conn, &QWebSocket::sslErrors, conn, &QWebSocket::deleteLater);
#endif
            QObject::connect(conn, &QWebSocket::disconnected, conn, &QWebSocket::deleteLater);
            QObject::connect(conn, &QWebSocket::errorOccurred, conn, &QWebSocket::deleteLater);
            auto ioDevice = new WebSocketIoDevice(conn);
            QObject::connect(conn, &QWebSocket::destroyed, ioDevice, &WebSocketIoDevice::deleteLater);
            hostNode.addHostSideConnection(ioDevice);
        }
    });
    //! [2]

    //! [3]
    QTreeView view;
    view.setWindowTitle(QStringLiteral("SourceView"));
    view.setModel(&sourceModel);
    view.show();
    TimerHandler handler;
    handler.model = &sourceModel;
    QTimer::singleShot(5000, &handler, &TimerHandler::changeData);
    QTimer::singleShot(10000, &handler, &TimerHandler::insertData);
    QTimer::singleShot(11000, &handler, &TimerHandler::changeFlags);
    QTimer::singleShot(12000, &handler, &TimerHandler::removeData);
    QTimer::singleShot(13000, &handler, &TimerHandler::moveData);

    return app.exec();
    //! [3]
}

#include "main.moc"
