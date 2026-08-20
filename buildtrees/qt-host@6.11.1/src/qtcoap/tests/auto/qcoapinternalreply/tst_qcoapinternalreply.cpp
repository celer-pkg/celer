// Copyright (C) 2018 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>
#include <QCoreApplication>

#include <private/qcoapinternalreply_p.h>
#include <private/qcoapreply_p.h>

class tst_QCoapInternalReply : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void parseReplyPdu_data();
    void parseReplyPdu();
    void updateReply_data();
    void updateReply();
};

void tst_QCoapInternalReply::parseReplyPdu_data()
{
    QTest::addColumn<QtCoap::ResponseCode>("responseCode");
    QTest::addColumn<QCoapMessage::Type>("type");
    QTest::addColumn<quint16>("messageId");
    QTest::addColumn<QByteArray>("token");
    QTest::addColumn<quint8>("tokenLength");
    QTest::addColumn<QList<QCoapOption::OptionName>>("optionsNames");
    QTest::addColumn<QList<quint8>>("optionsLengths");
    QTest::addColumn<QList<QByteArray>>("optionsValues");
    QTest::addColumn<QString>("payload");
    QTest::addColumn<QString>("pduHexa");

    QList<QCoapOption::OptionName> optionsNamesReply({QCoapOption::ContentFormat,
                                                      QCoapOption::MaxAge});
    QList<quint8> optionsLengthsReply({0, 1});
    QList<QByteArray> optionsValuesReply({"", QByteArray::fromHex("1e")});

    QList<QCoapOption::OptionName> bigOptionNameReply({QCoapOption::Size1});
    QList<quint8> bigOptionLengthReply({26});
    QList<QByteArray> bigOptionValueReply({QByteArray("abcdefghijklmnopqrstuvwxyz")});

    QTest::newRow("reply_with_options_and_payload")
            << QtCoap::ResponseCode::Content
            << QCoapMessage::Type::NonConfirmable
            << quint16(64463)
            << QByteArray("4647f09b")
            << quint8(4)
            << optionsNamesReply
            << optionsLengthsReply
            << optionsValuesReply
            << "Type: 1 (NON)\nCode: 1 (GET)\nMID: 56400\nToken: 4647f09b"
            << "5445fbcf4647f09bc0211eff547970653a203120284e4f4e290a436f64653a20"
               "312028474554290a4d49443a2035363430300a546f6b656e3a20343634376630"
               "3962";

    QTest::newRow("reply_with_payload")
            << QtCoap::ResponseCode::Content
            << QCoapMessage::Type::NonConfirmable
            << quint16(64463)
            << QByteArray("4647f09b")
            << quint8(4)
            << QList<QCoapOption::OptionName>()
            << QList<quint8>()
            << QList<QByteArray>()
            << "Type: 1 (NON)\nCode: 1 (GET)\nMID: 56400\nToken: 4647f09b"
            << "5445fbcf4647f09bff547970653a203120284e4f4e290a436f64653a20312028"
               "474554290a4d49443a2035363430300a546f6b656e3a203436343766303962";

    QTest::newRow("reply_with_options")
            << QtCoap::ResponseCode::Content
            << QCoapMessage::Type::NonConfirmable
            << quint16(64463)
            << QByteArray("4647f09b")
            << quint8(4)
            << optionsNamesReply
            << optionsLengthsReply
            << optionsValuesReply
            << ""
            << "5445fbcf4647f09bc0211e";

    QTest::newRow("reply_only")
            << QtCoap::ResponseCode::Content
            << QCoapMessage::Type::NonConfirmable
            << quint16(64463)
            << QByteArray("4647f09b")
            << quint8(4)
            << QList<QCoapOption::OptionName>()
            << QList<quint8>()
            << QList<QByteArray>()
            << ""
            << "5445fbcf4647f09b";

    QTest::newRow("reply_with_big_option")
            << QtCoap::ResponseCode::Content
            << QCoapMessage::Type::NonConfirmable
            << quint16(64463)
            << QByteArray("4647f09b")
            << quint8(4)
            << bigOptionNameReply
            << bigOptionLengthReply
            << bigOptionValueReply
            << ""
            << "5445fbcf4647f09bdd2f0d6162636465666768696a6b6c6d6e6f707172737475"
               "767778797a";
}

void tst_QCoapInternalReply::parseReplyPdu()
{
    QFETCH(QtCoap::ResponseCode, responseCode);
    QFETCH(QCoapMessage::Type, type);
    QFETCH(quint16, messageId);
    QFETCH(QByteArray, token);
    QFETCH(quint8, tokenLength);
    QFETCH(QList<QCoapOption::OptionName>, optionsNames);
    QFETCH(QList<quint8>, optionsLengths);
    QFETCH(QList<QByteArray>, optionsValues);
    QFETCH(QString, payload);
    QFETCH(QString, pduHexa);

    QScopedPointer<QCoapInternalReply>
            reply(QCoapInternalReply::createFromFrame(QByteArray::fromHex(pduHexa.toUtf8())));

    QCOMPARE(reply->message()->type(), type);
    QCOMPARE(reply->message()->tokenLength(), tokenLength);
    QCOMPARE(reply->responseCode(), responseCode);
    QCOMPARE(reply->message()->messageId(), messageId);
    QCOMPARE(reply->message()->token().toHex(), token);
    QCOMPARE(reply->message()->optionCount(), optionsNames.size());
    for (int i = 0; i < reply->message()->optionCount(); ++i) {
        QCoapOption option = reply->message()->optionAt(i);
        QCOMPARE(option.name(), optionsNames.at(i));
        QCOMPARE(option.length(), optionsLengths.at(i));
        QCOMPARE(option.opaqueValue(), optionsValues.at(i));
    }
    QCOMPARE(reply->message()->payload(), payload.toUtf8());
}

void tst_QCoapInternalReply::updateReply_data()
{
    QTest::addColumn<QByteArray>("data");

    QTest::newRow("success") << QByteArray("Data for the updating test");
}

void tst_QCoapInternalReply::updateReply()
{
    QFETCH(QByteArray, data);

    QScopedPointer<QCoapReply> reply(QCoapReplyPrivate::createCoapReply(QCoapRequest()));
    QCoapInternalReply internalReply;
    internalReply.message()->setPayload(data);
    QSignalSpy spyReplyFinished(reply.data(), &QCoapReply::finished);

    QMetaObject::invokeMethod(reply.data(), "_q_setContent",
                              Q_ARG(QHostAddress, internalReply.senderAddress()),
                              Q_ARG(QCoapMessage, *internalReply.message()),
                              Q_ARG(QtCoap::ResponseCode, internalReply.responseCode()));
    QMetaObject::invokeMethod(reply.data(), "_q_setFinished", Q_ARG(QtCoap::Error, QtCoap::Error::Ok));

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.size(), 1, 1000);
    QCOMPARE(reply->readAll(), data);
}

QTEST_MAIN(tst_QCoapInternalReply)

#include "tst_qcoapinternalreply.moc"
