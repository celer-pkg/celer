// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtSerialBus/qmodbusdevice.h>

class DummyDevice : public QModbusDevice
{
    Q_OBJECT
    friend class tst_QModbusDevice;

public:
    DummyDevice()
    {
        qRegisterMetaType<QModbusDevice::State>("QModbusDevice::State");
    }

protected:
    bool open() override { return openState; }
    void close() override {}

    bool openState = false;
};

class tst_QModbusDevice : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void connectDevice();
    void disconnectDevice();
    void state();
    void error();

private:
    DummyDevice *device;
};

void tst_QModbusDevice::initTestCase()
{
    device = new DummyDevice();
}

void tst_QModbusDevice::cleanupTestCase()
{
    delete device;
}

void tst_QModbusDevice::connectDevice()
{
    device->openState = false;
    QVERIFY(!device->connectDevice());
    QCOMPARE(device->state(), QModbusDevice::UnconnectedState);

    device->openState = true;
    QVERIFY(device->connectDevice());
    QCOMPARE(device->state(), QModbusDevice::ConnectingState);

    device->disconnectDevice();
    QCOMPARE(device->state(), QModbusDevice::ClosingState);

    device->setState(QModbusDevice::ClosingState);
    QVERIFY(!device->connectDevice());
}

void tst_QModbusDevice::disconnectDevice()
{
    //see QTBUG-66648
    DummyDevice dev;
    QCOMPARE(dev.state(), QModbusDevice::UnconnectedState);
    dev.disconnectDevice();
    QCOMPARE(dev.state(), QModbusDevice::UnconnectedState);
}

void tst_QModbusDevice::state()
{
    device->setState(QModbusDevice::ConnectedState);
    QCOMPARE(device->state(), QModbusDevice::ConnectedState);
    QSignalSpy spy(device, &DummyDevice::stateChanged);
    device->setState(QModbusDevice::UnconnectedState);
    QCOMPARE(device->state(), QModbusDevice::UnconnectedState);
    device->setState(QModbusDevice::UnconnectedState);
    QCOMPARE(device->state(), QModbusDevice::UnconnectedState);

    QCOMPARE(spy.size(), 1);
}

void tst_QModbusDevice::error()
{
    QCOMPARE(device->error(), QModbusDevice::NoError);
    QVERIFY(device->errorString().isEmpty());

    QString errorString("error string");
    device->setError(errorString, QModbusDevice::ConnectionError);

    QCOMPARE(device->error(), QModbusDevice::ConnectionError);
    QCOMPARE(device->errorString(), errorString);
}

QTEST_MAIN(tst_QModbusDevice)

#include "tst_qmodbusdevice.moc"
