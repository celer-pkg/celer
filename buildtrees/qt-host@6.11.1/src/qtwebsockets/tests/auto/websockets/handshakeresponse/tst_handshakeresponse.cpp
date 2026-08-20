// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QtTest/QtTest>
#include <QtTest/qtestcase.h>
#include <QtCore/QDebug>
#include <QtCore/QByteArray>
#include <QtCore/QRegularExpression>
#include <QtCore/QtEndian>

#include "private/qwebsockethandshakerequest_p.h"
#include "private/qwebsockethandshakeresponse_p.h"
#include "private/qwebsocketprotocol_p.h"
#include "QtWebSockets/qwebsocketprotocol.h"

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QWebSocketProtocol::CloseCode)
Q_DECLARE_METATYPE(QWebSocketProtocol::OpCode)

class tst_HandshakeResponse : public QObject
{
    Q_OBJECT

public:
    tst_HandshakeResponse();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void tst_date_response();
};

tst_HandshakeResponse::tst_HandshakeResponse()
{}

void tst_HandshakeResponse::initTestCase()
{
}

void tst_HandshakeResponse::cleanupTestCase()
{}

void tst_HandshakeResponse::init()
{
    qRegisterMetaType<QWebSocketProtocol::OpCode>("QWebSocketProtocol::OpCode");
    qRegisterMetaType<QWebSocketProtocol::CloseCode>("QWebSocketProtocol::CloseCode");
}

void tst_HandshakeResponse::cleanup()
{
}

void tst_HandshakeResponse::tst_date_response()
{
    QWebSocketHandshakeRequest request(80, false);
    QByteArray bytes = "GET / HTTP/1.1\r\nHost: example.com\r\nSec-WebSocket-Version: 13\r\n"
                       "Sec-WebSocket-Key: AVDFBDDFF\r\n"
                       "Upgrade: websocket\r\n"
                       "Connection: Upgrade\r\n\r\n";
    request.readHandshake(bytes, 8 * 1024);

    QWebSocketHandshakeResponse response(request, "example.com", true,
                                         QList<QWebSocketProtocol::Version>() << QWebSocketProtocol::Version13,
                                         QList<QString>(),
                                         QList<QString>());
    QString data;
    QTextStream output(&data);
    output << response;

    QStringList list = data.split("\r\n");
    int index = list.indexOf(QRegularExpression("Date:.*"));
    QVERIFY(index > -1);
    QVERIFY(QLocale::c().toDateTime(list[index], "'Date:' ddd, dd MMM yyyy hh:mm:ss 'GMT'").isValid());
}

QTEST_MAIN(tst_HandshakeResponse)

#include "tst_handshakeresponse.moc"
