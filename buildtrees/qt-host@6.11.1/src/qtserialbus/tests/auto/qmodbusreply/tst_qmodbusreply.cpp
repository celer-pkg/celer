// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtSerialBus/QModbusReply>

class tst_QModbusReply : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void tst_ctor();
    void tst_setFinished();
    void tst_setError_data();
    void tst_setError();
    void tst_setResult();
};

void tst_QModbusReply::initTestCase()
{
    qRegisterMetaType<QModbusDevice::Error>();
}

void tst_QModbusReply::tst_ctor()
{
    QModbusReply r(QModbusReply::Common, 1, this);
    QCOMPARE(r.type(), QModbusReply::Common);
    QCOMPARE(r.serverAddress(), 1);
    QCOMPARE(r.isFinished(), false);
    QCOMPARE(r.result().isValid(), false);
    QCOMPARE(r.rawResult().isValid(), false);
    QCOMPARE(r.errorString(), QString());
    QCOMPARE(r.error(), QModbusDevice::NoError);

    QModbusReply r2(QModbusReply::Raw, 2, this);
    QCOMPARE(r2.type(), QModbusReply::Raw);
    QCOMPARE(r2.serverAddress(), 2);
    QCOMPARE(r2.isFinished(), false);
    QCOMPARE(r2.result().isValid(), false);
    QCOMPARE(r2.rawResult().isValid(), false);
    QCOMPARE(r2.errorString(), QString());
    QCOMPARE(r2.error(), QModbusDevice::NoError);
}

void tst_QModbusReply::tst_setFinished()
{
    QModbusReply replyTest(QModbusReply::Common, 1);
    QCOMPARE(replyTest.serverAddress(), 1);
    QSignalSpy finishedSpy(&replyTest, &QModbusReply::finished);
    QSignalSpy errorSpy(&replyTest, &QModbusReply::errorOccurred);

    QCOMPARE(replyTest.serverAddress(), 1);
    QCOMPARE(replyTest.isFinished(), false);
    QCOMPARE(replyTest.result().isValid(), false);
    QCOMPARE(replyTest.rawResult().isValid(), false);
    QCOMPARE(replyTest.errorString(), QString());
    QCOMPARE(replyTest.error(), QModbusDevice::NoError);

    QVERIFY(finishedSpy.isEmpty());
    QVERIFY(errorSpy.isEmpty());

    replyTest.setFinished(true);
    QVERIFY(finishedSpy.size() == 1);
    QVERIFY(errorSpy.isEmpty());
    QCOMPARE(replyTest.serverAddress(), 1);
    QCOMPARE(replyTest.isFinished(), true);
    QCOMPARE(replyTest.result().isValid(), false);
    QCOMPARE(replyTest.rawResult().isValid(), false);
    QCOMPARE(replyTest.errorString(), QString());
    QCOMPARE(replyTest.error(), QModbusDevice::NoError);

    replyTest.setFinished(false);
    QVERIFY(finishedSpy.size() == 1); // no further signal
    QVERIFY(errorSpy.isEmpty());
    QCOMPARE(replyTest.serverAddress(), 1);
    QCOMPARE(replyTest.isFinished(), false);
    QCOMPARE(replyTest.result().isValid(), false);
    QCOMPARE(replyTest.rawResult().isValid(), false);
    QCOMPARE(replyTest.errorString(), QString());
    QCOMPARE(replyTest.error(), QModbusDevice::NoError);
}

void tst_QModbusReply::tst_setError_data()
{
    QTest::addColumn<QModbusDevice::Error>("error");
    QTest::addColumn<QString>("errorString");

    QTest::newRow("ProtocolError") << QModbusDevice::ProtocolError << QString("ProtocolError");
    QTest::newRow("NoError") << QModbusDevice::NoError << QString("NoError");
    QTest::newRow("NoError-empty") << QModbusDevice::NoError << QString();
    QTest::newRow("TimeoutError") << QModbusDevice::TimeoutError << QString("TimeoutError");
    QTest::newRow("ReplyAbortedError") << QModbusDevice::ReplyAbortedError << QString("AbortedError");
}

void tst_QModbusReply::tst_setError()
{
    QFETCH(QModbusDevice::Error, error);
    QFETCH(QString, errorString);

    QModbusReply replyTest(QModbusReply::Common, 1);
    QCOMPARE(replyTest.serverAddress(), 1);
    QSignalSpy finishedSpy(&replyTest, &QModbusReply::finished);
    QSignalSpy errorSpy(&replyTest, &QModbusReply::errorOccurred);

    QVERIFY(finishedSpy.isEmpty());
    QVERIFY(errorSpy.isEmpty());

    replyTest.setError(error, errorString);
    QCOMPARE(finishedSpy.size(), 1);
    QCOMPARE(errorSpy.size(), 1);
    QCOMPARE(replyTest.rawResult().isValid(), false);
    QCOMPARE(replyTest.error(), error);
    QCOMPARE(replyTest.errorString(), errorString);
    QCOMPARE(errorSpy.at(0).at(0).value<QModbusDevice::Error>(), error);

    replyTest.setError(error, errorString);
    replyTest.setFinished(true);
    QCOMPARE(finishedSpy.size(), 3); //setError() implies call to setFinished()
    QCOMPARE(errorSpy.size(), 2);
}

void tst_QModbusReply::tst_setResult()
{
    QModbusDataUnit unit(QModbusDataUnit::Coils, 5, {4,5,6});
    QCOMPARE(unit.startAddress(), 5);
    QCOMPARE(unit.valueCount(), 3);
    QCOMPARE(unit.registerType(), QModbusDataUnit::Coils);
    QCOMPARE(unit.isValid(), true);
    QList<quint16> reference = { 4, 5, 6 };
    QCOMPARE(unit.values(), reference);

    QModbusReply replyTest(QModbusReply::Common, 1);
    QCOMPARE(replyTest.serverAddress(), 1);
    QSignalSpy finishedSpy(&replyTest, &QModbusReply::finished);
    QSignalSpy errorSpy(&replyTest, &QModbusReply::errorOccurred);

    QVERIFY(finishedSpy.isEmpty());
    QVERIFY(errorSpy.isEmpty());

    QCOMPARE(replyTest.result().startAddress(), -1);
    QCOMPARE(replyTest.result().valueCount(), 0);
    QCOMPARE(replyTest.result().registerType(), QModbusDataUnit::Invalid);
    QCOMPARE(replyTest.result().isValid(), false);
    QCOMPARE(replyTest.rawResult().isValid(), false);
    QCOMPARE(replyTest.result().values(), QList<quint16>());

    QModbusResponse response(QModbusResponse::ReadExceptionStatus, quint16(0x0000));
    replyTest.setResult(unit);
    replyTest.setRawResult(response);
    QCOMPARE(finishedSpy.size(), 0);
    QCOMPARE(errorSpy.size(), 0);
    QCOMPARE(replyTest.result().startAddress(), 5);
    QCOMPARE(replyTest.result().valueCount(), 3);
    QCOMPARE(replyTest.result().registerType(), QModbusDataUnit::Coils);
    QCOMPARE(replyTest.result().isValid(), true);
    QCOMPARE(replyTest.result().values(), reference);
    QCOMPARE(replyTest.rawResult().isValid(), true);

    auto tmp = replyTest.rawResult();
    QCOMPARE(tmp.functionCode(), QModbusResponse::ReadExceptionStatus);
    QCOMPARE(tmp.data(), QByteArray::fromHex("0000"));

    QModbusReply replyRawTest(QModbusReply::Raw, 1);
    QCOMPARE(replyRawTest.serverAddress(), 1);
    QSignalSpy finishedSpyRaw(&replyRawTest, &QModbusReply::finished);
    QSignalSpy errorSpyRaw(&replyRawTest, &QModbusReply::errorOccurred);

    QVERIFY(finishedSpyRaw.isEmpty());
    QVERIFY(errorSpyRaw.isEmpty());

    QCOMPARE(replyRawTest.result().startAddress(), -1);
    QCOMPARE(replyRawTest.result().valueCount(), 0);
    QCOMPARE(replyRawTest.result().registerType(), QModbusDataUnit::Invalid);
    QCOMPARE(replyRawTest.result().isValid(), false);
    QCOMPARE(replyRawTest.rawResult().isValid(), false);
    QCOMPARE(replyRawTest.result().values(), QList<quint16>());

    replyRawTest.setResult(unit);
    replyRawTest.setRawResult(response);
    QCOMPARE(finishedSpy.size(), 0);
    QCOMPARE(errorSpyRaw.size(), 0);
    QCOMPARE(replyRawTest.result().isValid(), true);
    QCOMPARE(replyRawTest.rawResult().isValid(), true);

    tmp = replyRawTest.rawResult();
    QCOMPARE(tmp.functionCode(), QModbusResponse::ReadExceptionStatus);
    QCOMPARE(tmp.data(), QByteArray::fromHex("0000"));
}

QTEST_MAIN(tst_QModbusReply)

#include "tst_qmodbusreply.moc"
