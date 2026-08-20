// Copyright (C) 2018 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>
#include <QCoreApplication>

#include <QtCoap/qcoaprequest.h>
#include <private/qcoapinternalrequest_p.h>
#include <private/qcoaprequest_p.h>

class tst_QCoapInternalRequest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void requestToFrame_data();
    void requestToFrame();
    void parseUri_data();
    void parseUri();
    void urlOptions_data();
    void urlOptions();
    void invalidUrls_data();
    void invalidUrls();
    void isMulticast_data();
    void isMulticast();
    void parseBlockOption_data();
    void parseBlockOption();
    void createBlockOption_data();
    void createBlockOption();
    void initEmptyMessage_data();
    void initEmptyMessage();
};

void tst_QCoapInternalRequest::requestToFrame_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QtCoap::Method>("method");
    QTest::addColumn<QCoapMessage::Type>("type");
    QTest::addColumn<quint16>("messageId");
    QTest::addColumn<QByteArray>("token");
    QTest::addColumn<QString>("pduHeader");
    QTest::addColumn<QString>("pduPayload");

    QTest::newRow("request_with_option_and_payload")
        << QUrl("coap://10.20.30.40:5683/test")
        << QtCoap::Method::Get
        << QCoapRequest::Type::NonConfirmable
        << quint16(56400)
        << QByteArray::fromHex("4647f09b")
        << "5401dc504647f09bb474657374ff"
        << "Some payload";

    QTest::newRow("request_domain")
        << QUrl("coap://domain.com:5683/test")
        << QtCoap::Method::Get
        << QCoapRequest::Type::NonConfirmable
        << quint16(56400)
        << QByteArray::fromHex("4647f09b")
        << "5401dc504647f09b3a646f6d61696e2e636f6d8474657374ff"
        << "Some payload";

    QTest::newRow("request_ipv6")
        << QUrl("coap://[::ffff:ac11:3]:5683/test")
        << QtCoap::Method::Get
        << QCoapRequest::Type::NonConfirmable
        << quint16(56400)
        << QByteArray::fromHex("4647f09b")
        << "5401dc504647f09bb474657374ff"
        << "Some payload";

    QTest::newRow("request_without_payload")
        << QUrl("coap://10.20.30.40:5683/test")
        << QtCoap::Method::Get
        << QCoapRequest::Type::NonConfirmable
        << quint16(56400)
        << QByteArray::fromHex("4647f09b")
        << "5401dc504647f09bb474657374"
        << "";

    QTest::newRow("request_without_option")
        << QUrl("coap://10.20.30.40:5683/")
        << QtCoap::Method::Put
        << QCoapRequest::Type::Confirmable
        << quint16(56400)
        << QByteArray::fromHex("4647f09b")
        << "4403dc504647f09bff"
        << "Some payload";

    QTest::newRow("request_only")
        << QUrl("coap://10.20.30.40:5683/")
        << QtCoap::Method::Get
        << QCoapRequest::Type::NonConfirmable
        << quint16(56400)
        << QByteArray::fromHex("4647f09b")
        << "5401dc504647f09b"
        << "";

    QTest::newRow("request_with_multiple_options")
        << QUrl("coap://10.20.30.40:5683/test/oui")
        << QtCoap::Method::Get
        << QCoapRequest::Type::NonConfirmable
        << quint16(56400)
        << QByteArray::fromHex("4647f09b")
        << "5401dc504647f09bb474657374036f7569"
        << "";

    QTest::newRow("request_with_big_option_number")
        << QUrl("coap://10.20.30.40:5683/test")
        << QtCoap::Method::Get
        << QCoapRequest::Type::NonConfirmable
        << quint16(56400)
        << QByteArray::fromHex("4647f09b")
        << "5401dc504647f09bb474657374dd240d6162636465666768696a6b6c6d6e6f70"
           "7172737475767778797aff"
        << "Some payload";
}

void tst_QCoapInternalRequest::requestToFrame()
{
    QFETCH(QUrl, url);
    QFETCH(QtCoap::Method, method);
    QFETCH(QCoapMessage::Type, type);
    QFETCH(quint16, messageId);
    QFETCH(QByteArray, token);
    QFETCH(QString, pduHeader);
    QFETCH(QString, pduPayload);

    QCoapRequest request = QCoapRequestPrivate::createRequest(QCoapRequest(url), method);
    request.setType(type);
    request.setPayload(pduPayload.toUtf8());
    request.setMessageId(messageId);
    request.setToken(token);
    if (qstrcmp(QTest::currentDataTag(), "request_with_big_option_number") == 0)
        request.addOption(QCoapOption::Size1, QByteArray("abcdefghijklmnopqrstuvwxyz"));

    QByteArray pdu;
    pdu.append(pduHeader.toUtf8());
    if (!pduPayload.isEmpty())
        pdu.append(pduPayload.toUtf8().toHex());

    QCoapInternalRequest internalRequest(request);
    QCOMPARE(internalRequest.toQByteArray().toHex(), pdu);
}

void tst_QCoapInternalRequest::parseUri_data()
{
    qRegisterMetaType<QList<QCoapOption>>();
    QTest::addColumn<QUrl>("uri");
    QTest::addColumn<QUrl>("proxyUri");
    QTest::addColumn<QList<QCoapOption>>("options");

    QTest::newRow("port_path")
                        << QUrl("coap://10.20.30.40:1234/test/path1")
                        << QUrl()
                        << QList<QCoapOption>({
                            QCoapOption(QCoapOption::UriPort, 1234),
                            QCoapOption(QCoapOption::UriPath, QByteArray("test")),
                            QCoapOption(QCoapOption::UriPath, QByteArray("path1")) });

    QTest::newRow("path_query")
                        << QUrl("coap://10.20.30.40/test/path1/?rd=25&nd=4")
                        << QUrl()
                        << QList<QCoapOption>({
                            QCoapOption(QCoapOption::UriPath, QByteArray("test")),
                            QCoapOption(QCoapOption::UriPath, QByteArray("path1")),
                            QCoapOption(QCoapOption::UriQuery, QByteArray("rd=25")),
                            QCoapOption(QCoapOption::UriQuery, QByteArray("nd=4")) });

    QTest::newRow("host_path_query")
                        << QUrl("coap://aa.bb.cc.com:5683/test/path1/?rd=25&nd=4")
                        << QUrl()
                        << QList<QCoapOption>({
                            QCoapOption(QCoapOption::UriHost, QByteArray("aa.bb.cc.com")),
                            QCoapOption(QCoapOption::UriPath, QByteArray("test")),
                            QCoapOption(QCoapOption::UriPath, QByteArray("path1")),
                            QCoapOption(QCoapOption::UriQuery, QByteArray("rd=25")),
                            QCoapOption(QCoapOption::UriQuery, QByteArray("nd=4")) });

    QTest::newRow("proxy_url")
                        << QUrl("coap://aa.bb.cc.com:5683/test/path1/?rd=25&nd=4")
                        << QUrl("coap://10.20.30.40/test:5684/othertest/path")
                        << QList<QCoapOption>({
                            QCoapOption(QCoapOption::ProxyUri,
                            QByteArray("coap://10.20.30.40/test:5684/othertest/path")) });
}

void tst_QCoapInternalRequest::parseUri()
{
    QFETCH(QUrl, uri);
    QFETCH(QUrl, proxyUri);
    QFETCH(QList<QCoapOption>, options);

    QCoapRequest request(uri, QCoapMessage::Type::NonConfirmable, proxyUri);
    QCoapInternalRequest internalRequest(request);

    for (QCoapOption opt : options)
        QVERIFY2(internalRequest.message()->options().contains(opt), "Missing option");

    QCOMPARE(options.size(), internalRequest.message()->optionCount());
}

void tst_QCoapInternalRequest::urlOptions_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QList<QCoapOption>>("options");

    QList<QCoapOption> options = {
        { QCoapOption::UriHost, QByteArray("example.com") },
        { QCoapOption::UriPath, QByteArray("~sensors") },
        { QCoapOption::UriPath, QByteArray("temp.xml") }
    };

    QTest::newRow("url_with_default_port")
            << "coap://example.com:5683/~sensors/temp.xml"
            << options;

    QTest::newRow("url_percent_encoding_uppercase")
            << "coap://EXAMPLE.com/%7Esensors/temp.xml"
            << options;

    QTest::newRow("url_with_no_port_uppercase")
            << "coap://EXAMPLE.com:/%7esensors/temp.xml"
            << options;

    QTest::newRow("url_with_dot_segments")
            << "coap://exaMPLE.com/%7esensors/../%7esensors//./temp.xml"
            << options;

    //! TODO Add more test URLs
}

void tst_QCoapInternalRequest::urlOptions()
{
    QFETCH(QString, url);
    QFETCH(QList<QCoapOption>, options);

    const QCoapRequest request(QUrl{url});
    const QCoapInternalRequest internalRequest(request);

    auto requestOptions = internalRequest.message()->options();
    for (const auto& option : options)
        QVERIFY2(requestOptions.removeAll(option) > 0, "Missing option");

    QVERIFY2(requestOptions.isEmpty(), "Fewer options were expected");
}

void tst_QCoapInternalRequest::invalidUrls_data()
{
    QTest::addColumn<QString>("url");

    QTest::newRow("url_with_non_ascii")
            << QString("coap://example.com:5683/~sensors/%1temp.xml").arg(QChar(0x00A3));

    QTest::newRow("url_no_scheme")
            << "example.com:5683/~sensors/temp.xml";

    QTest::newRow("url_wrong_scheme")
            << "http://example.com:5683/~sensors/temp.xml";

    //! TODO Add more test URLs
}

void tst_QCoapInternalRequest::invalidUrls()
{
    QFETCH(QString, url);
    const QCoapRequest request(QUrl{url});
    const QCoapInternalRequest internalRequest(request);

    QVERIFY(!internalRequest.isValid());
    QVERIFY(internalRequest.message()->options().empty());
}

void tst_QCoapInternalRequest::isMulticast_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<bool>("result");

    QTest::newRow("ipv4_multicast") << QString("coap://224.0.1.187") << true;
    QTest::newRow("ipv4_multicast_resource") << QString("coap://224.0.1.187/path") << true;
    QTest::newRow("ipv6_multicast_link_local") << "coap://[ff02::fd]" << true;
    QTest::newRow("ipv6_multicast_site_local") << "coap://[ff05::fd]" << true;
    QTest::newRow("not_multicast") << QString("coap://127.0.0.1") << false;
}

void tst_QCoapInternalRequest::isMulticast()
{
    QFETCH(QString, url);
    QFETCH(bool, result);

    const QCoapRequest request(QUrl{url});
    const QCoapInternalRequest internalRequest(request);
    QCOMPARE(internalRequest.isMulticast(), result);
}

void tst_QCoapInternalRequest::parseBlockOption_data()
{
    QTest::addColumn<QByteArray>("value");
    QTest::addColumn<uint>("blockNumber");
    QTest::addColumn<bool>("hasNext");
    QTest::addColumn<uint>("blockSize");

    QTest::newRow("block_option_1byte_more_blocks") << QByteArray::fromHex("3B")
                                                    << 3u
                                                    << true
                                                    << 128u;
    QTest::newRow("block_option_1byte_no_more_blocks") << QByteArray::fromHex("93")
                                                       << 9u
                                                       << false
                                                       << 128u;
    QTest::newRow("block_option_2bytes_more_blocks") << QByteArray::fromHex("12A")
                                                     << 18u
                                                     << true
                                                     << 64u;
    QTest::newRow("block_option_2bytes_no_more_blocks") << QByteArray::fromHex("132")
                                                        << 19u
                                                        << false
                                                        << 64u;
    QTest::newRow("block_option_3bytes_more_blocks") << QByteArray::fromHex("3AB2A")
                                                     << 15026u
                                                     << true
                                                     << 64u;
    QTest::newRow("block_option_3bytes_no_more_blocks") << QByteArray::fromHex("3AB22")
                                                        << 15026u
                                                        << false
                                                        << 64u;
}

void tst_QCoapInternalRequest::parseBlockOption()
{
    QFETCH(QByteArray, value);
    QFETCH(uint, blockNumber);
    QFETCH(bool, hasNext);
    QFETCH(uint, blockSize);

    QCoapInternalRequest request;
    request.addOption(QCoapOption::Block1, value);

    QCOMPARE(request.currentBlockNumber(), blockNumber);
    QCOMPARE(request.hasMoreBlocksToReceive(), hasNext);
    QCOMPARE(request.blockSize(), blockSize);
}

void tst_QCoapInternalRequest::createBlockOption_data()
{
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<uint>("blockNumber");
    QTest::addColumn<uint>("blockSize");
    QTest::addColumn<QCoapOption>("expectedOption");

    QByteArray data;
    for (uint i = 0; i < 1024; ++i)
        data.append(0xF);

    QByteArray largeData;
    for (uint i = 0; i < 32; ++i)
        largeData.append(data);

    QTest::newRow("block1_option_1byte_more_blocks")
            << data
            << 3u
            << 64u
            << QCoapOption(QCoapOption::Block1, QByteArray::fromHex("3A"));
    QTest::newRow("block1_option_1byte_no_more_blocks")
            << data
            << 8u
            << 128u
            << QCoapOption(QCoapOption::Block1, QByteArray::fromHex("83"));
    QTest::newRow("block2_option_1byte")
            << data
            << 3u
            << 64u
            << QCoapOption(QCoapOption::Block2, QByteArray::fromHex("32"));
    QTest::newRow("block1_option_2bytes_more_blocks")
            << data
            << 29u
            << 32u
            << QCoapOption(QCoapOption::Block1, QByteArray::fromHex("1D9"));
    QTest::newRow("block1_option_2bytes_no_more_blocks")
            << data
            << 32u
            << 32u
            << QCoapOption(QCoapOption::Block1, QByteArray::fromHex("201"));
    QTest::newRow("block2_option_2bytes")
            << data
            << 29u
            << 32u
            << QCoapOption(QCoapOption::Block2, QByteArray::fromHex("1D1"));
    QTest::newRow("block1_option_3bytes_more_blocks")
            << largeData
            << 4096u
            << 4u
            << QCoapOption(QCoapOption::Block1, QByteArray::fromHex("10008"));
    QTest::newRow("block1_option_3bytes_no_more_blocks")
            << largeData
            << 8192u
            << 4u
            << QCoapOption(QCoapOption::Block1, QByteArray::fromHex("20000"));
    QTest::newRow("block2_option_3bytes")
            << largeData
            << 4096u
            << 4u
            << QCoapOption(QCoapOption::Block2, QByteArray::fromHex("10000"));
}

void tst_QCoapInternalRequest::createBlockOption()
{
    QFETCH(QByteArray, payload);
    QFETCH(uint, blockNumber);
    QFETCH(uint, blockSize);
    QFETCH(QCoapOption, expectedOption);

    QCoapRequest request;
    request.setPayload(payload);
    QCoapInternalRequest internalRequest(request);
    if (expectedOption.name() == QCoapOption::Block1)
        internalRequest.setToSendBlock(blockNumber, blockSize);
    else if (expectedOption.name() == QCoapOption::Block2)
        internalRequest.setToRequestBlock(blockNumber, blockSize);
    else
        QFAIL("Incorrect option, the test expects Block1 or Block2 options.");

    QCOMPARE(internalRequest.message()->options().size(), 1);

    const auto option = internalRequest.message()->options().back();
    QCOMPARE(option.name(), expectedOption.name());
    QCOMPARE(option.opaqueValue(), expectedOption.opaqueValue());
}

void tst_QCoapInternalRequest::initEmptyMessage_data()
{
    QTest::addColumn<QCoapMessage::Type>("type");
    QTest::addColumn<QByteArray>("messageHeader");
    QTest::newRow("acknowledge") << QCoapMessage::Type::Acknowledgment << QByteArray("6000002a");
    QTest::newRow("reset") << QCoapMessage::Type::Reset << QByteArray("7000002a");

}

void tst_QCoapInternalRequest::initEmptyMessage()
{
    QFETCH(QCoapMessage::Type, type);
    QFETCH(QByteArray, messageHeader);

    // Populate the request with random data
    QCoapRequest request = QCoapRequestPrivate::createRequest(QCoapRequest("coap://test"),
                                                              QtCoap::Method::Get);
    request.setVersion(1);
    request.setType(QCoapMessage::Type::Confirmable);
    request.setMessageId(111);
    request.setToken("token");
    request.addOption(QCoapOption::ProxyUri);
    request.setPayload("payload");

    QCoapInternalRequest emptyMessageRequest(request);
    emptyMessageRequest.initEmptyMessage(42, type);
    QCOMPARE(emptyMessageRequest.toQByteArray().toHex(), messageHeader);
}

QTEST_APPLESS_MAIN(tst_QCoapInternalRequest)

#include "tst_qcoapinternalrequest.moc"
