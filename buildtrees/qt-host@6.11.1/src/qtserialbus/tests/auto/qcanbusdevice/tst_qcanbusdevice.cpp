// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusframe.h>

#include <QtCore/qtimer.h>
#include <QtCore/QElapsedTimer>
#include <QtCore/QtPlugin>
#include <QtTest/qsignalspy.h>
#include <QtTest/qtest.h>

#include <memory>

using namespace Qt::StringLiterals;

Q_DECLARE_METATYPE(QCanBusDevice::Filter)

class tst_Backend : public QCanBusDevice
{
    Q_OBJECT
public:
    tst_Backend()
    {
        referenceFrame.setFrameId(5);
        referenceFrame.setPayload(QByteArray("FOOBAR"));
        referenceFrame.setTimeStamp({ 22, 23 });
        referenceFrame.setExtendedFrameFormat(1);
    }

    bool triggerNewFrame()
    {
        if (state() != QCanBusDevice::ConnectedState)
            return false;

        // the line below triggers the framesReceived() signal
        enqueueReceivedFrames({referenceFrame});

        return true;
    }

    bool open() override
    {
        if (firstOpen) {
            firstOpen = false;
            return false;
        }
        setState(QCanBusDevice::ConnectedState);
        return true;
    }

    void close() override
    {
        setState(QCanBusDevice::UnconnectedState);
    }

    bool writeFrame(const QCanBusFrame &data) override
    {
        if (state() != QCanBusDevice::ConnectedState) {
            setError(QStringLiteral("Cannot write frame as device is not connected"),
                     QCanBusDevice::OperationError);
            return false;
        }

        if (writeBufferUsed) {
            enqueueOutgoingFrame(data);
            QTimer::singleShot(2000, this, [this](){ triggerDelayedWrites(); });
        } else {
            emit framesWritten(1);
        }
        return true;
    }

    void emulateError(const QString &text, QCanBusDevice::CanBusError e)
    {
        setError(text, e);
    }

    QString interpretErrorFrame(const QCanBusFrame &/*errorFrame*/) override
    {
        return QString();
    }

    bool isWriteBuffered() const { return writeBufferUsed; }
    void setWriteBuffered(bool isBuffered)
    {
        // allows switching between buffered and unbuffered write mode
        writeBufferUsed = isBuffered;
    }

    QCanBusDeviceInfo deviceInfo() const override
    {
        return createDeviceInfo(
            u"plugin"_s,
            u"name"_s,
            u"serial number"_s,
            u"description"_s,
            u"alias"_s,
            100,    //channel
            true,   // virtual
            true    // flexible data rate
        );
    }

public slots:
    void triggerDelayedWrites()
    {
        if (framesToWrite() == 0)
            return;

        dequeueOutgoingFrame();
        emit framesWritten(1);

        if (framesToWrite() > 0)
            QTimer::singleShot(2000, this, [this](){ triggerDelayedWrites(); });
    }

private:
    QCanBusFrame referenceFrame;
    bool firstOpen = true;
    bool writeBufferUsed = true;
};

class tst_QCanBusDevice : public QObject
{
    Q_OBJECT
public:
    explicit tst_QCanBusDevice();

private slots:
    void initTestCase();
    void conf();
    void write();
    void read();
    void readAll();
    void clearInputBuffer();
    void clearOutputBuffer();
    void error();
    void cleanupTestCase();
    void tst_filtering();
    void filterEqual_data();
    void filterEqual();
    void tst_bufferingAttribute();

    void tst_waitForFramesReceived();
    void tst_waitForFramesWritten();

    void tst_deviceInfo();
private:
    std::unique_ptr<tst_Backend> device;
};

tst_QCanBusDevice::tst_QCanBusDevice()
{
    qRegisterMetaType<QCanBusDevice::CanBusDeviceState>();
    qRegisterMetaType<QCanBusDevice::CanBusError>();
    qRegisterMetaType<QCanBusDevice::Filter>();
}

void tst_QCanBusDevice::initTestCase()
{
    device.reset(new tst_Backend());
    QVERIFY(device);

    QSignalSpy stateSpy(device.get(), &QCanBusDevice::stateChanged);

    QVERIFY(!device->connectDevice()); // first connect triggered to fail
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QVERIFY(device->connectDevice());
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState, 5000);
    QCOMPARE(stateSpy.size(), 4);
    QCOMPARE(stateSpy.at(0).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectingState);
    QCOMPARE(stateSpy.at(1).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::UnconnectedState);
    QCOMPARE(stateSpy.at(2).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectingState);
    QCOMPARE(stateSpy.at(3).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectedState);
}

void tst_QCanBusDevice::conf()
{
    QVERIFY(device->configurationKeys().isEmpty());

    // invalid QVariant ignored
    device->setConfigurationParameter(QCanBusDevice::RawFilterKey, QVariant());
    QVERIFY(device->configurationKeys().isEmpty());

    QCanBusFrame::FrameErrors error =
            (QCanBusFrame::LostArbitrationError | QCanBusFrame::BusError);

    device->setConfigurationParameter(
                QCanBusDevice::ErrorFilterKey, QVariant::fromValue(error));
    QVariant value = device->configurationParameter(QCanBusDevice::ErrorFilterKey);
    QVERIFY(value.isValid());

    QList<QCanBusDevice::ConfigurationKey> keys = device->configurationKeys();
    QCOMPARE(keys.size(), 1);
    QVERIFY(keys.at(0) == QCanBusDevice::ErrorFilterKey);

    QCOMPARE(value.value<QCanBusFrame::FrameErrors>(),
             QCanBusFrame::LostArbitrationError | QCanBusFrame::BusError);

    device->setConfigurationParameter(QCanBusDevice::ErrorFilterKey, QVariant());
    QVERIFY(device->configurationKeys().isEmpty());
}

void tst_QCanBusDevice::write()
{
    // we assume unbuffered writing in this function
    device->setWriteBuffered(false);
    QVERIFY(!device->isWriteBuffered());

    QSignalSpy spy(device.get(), &QCanBusDevice::framesWritten);
    QSignalSpy stateSpy(device.get(), &QCanBusDevice::stateChanged);

    QCanBusFrame frame;
    frame.setPayload(QByteArray("testData"));

    device->disconnectDevice();
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::UnconnectedState, 5000);
    QCOMPARE(stateSpy.size(), 2);
    QCOMPARE(stateSpy.at(0).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ClosingState);
    QCOMPARE(stateSpy.at(1).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::UnconnectedState);
    stateSpy.clear();
    QVERIFY(stateSpy.isEmpty());

    QVERIFY(!device->writeFrame(frame));
    QCOMPARE(device->error(), QCanBusDevice::OperationError);
    QCOMPARE(spy.size(), 0);

    device->connectDevice();
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState, 5000);
    QCOMPARE(stateSpy.size(), 2);
    QCOMPARE(stateSpy.at(0).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectingState);
    QCOMPARE(stateSpy.at(1).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectedState);

    QVERIFY(device->writeFrame(frame));
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QCOMPARE(spy.size(), 1);
}

void tst_QCanBusDevice::read()
{
    QSignalSpy stateSpy(device.get(), &QCanBusDevice::stateChanged);

    device->disconnectDevice();
    QCOMPARE(device->state(), QCanBusDevice::UnconnectedState);
    stateSpy.clear();

    const QCanBusFrame frame1 = device->readFrame();
    QCOMPARE(device->error(), QCanBusDevice::OperationError);

    QVERIFY(device->connectDevice());
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState, 5000);
    QCOMPARE(stateSpy.size(), 2);
    QCOMPARE(stateSpy.at(0).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectingState);
    QCOMPARE(stateSpy.at(1).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectedState);

    device->triggerNewFrame();
    const QCanBusFrame frame2 = device->readFrame();
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QVERIFY(!frame1.frameId());
    QVERIFY(!frame1.isValid());
    QVERIFY(frame2.frameId());
    QVERIFY(frame2.isValid());
}

void tst_QCanBusDevice::readAll()
{
    enum { FrameNumber = 10 };
    device->disconnectDevice();
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::UnconnectedState, 5000);

    const QList<QCanBusFrame> empty = device->readAllFrames();
    QCOMPARE(device->error(), QCanBusDevice::OperationError);
    QVERIFY(empty.isEmpty());

    QVERIFY(device->connectDevice());
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState, 5000);

    for (int i = 0; i < FrameNumber; ++i)
        device->triggerNewFrame();

    const QList<QCanBusFrame> frames = device->readAllFrames();
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QCOMPARE(FrameNumber, frames.size());
    QVERIFY(!device->framesAvailable());
}

void tst_QCanBusDevice::clearInputBuffer()
{
    device->disconnectDevice();
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::UnconnectedState, 5000);

    device->clear(QCanBusDevice::Input);
    QCOMPARE(device->error(), QCanBusDevice::OperationError);

    QVERIFY(device->connectDevice());
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState, 5000);

    device->clear(QCanBusDevice::Input);
    QCOMPARE(device->error(), QCanBusDevice::NoError);

    for (int i = 0; i < 10; ++i)
        device->triggerNewFrame();

    device->clear(QCanBusDevice::Input);
    QCOMPARE(device->error(), QCanBusDevice::NoError);

    QVERIFY(!device->framesAvailable());
}

void tst_QCanBusDevice::clearOutputBuffer()
{
    // this test requires buffered writing
    device->setWriteBuffered(true);
    device->disconnectDevice();
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::UnconnectedState, 5000);

    device->clear(QCanBusDevice::Output);
    QCOMPARE(device->error(), QCanBusDevice::OperationError);

    QVERIFY(device->connectDevice());
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState, 5000);

    device->clear(QCanBusDevice::Output);
    QCOMPARE(device->error(), QCanBusDevice::NoError);

    // first test buffered writing, frames will be written after some delay
    QSignalSpy spy(device.get(), &QCanBusDevice::framesWritten);
    for (int i = 0; i < 10; ++i)
        device->writeFrame(QCanBusFrame(0x123, "output"));
    QTRY_VERIFY_WITH_TIMEOUT(spy.size() == 10, 5000);

    // now test clearing the buffer before the frames are actually written
    spy.clear();
    for (int i = 0; i < 10; ++i)
        device->writeFrame(QCanBusFrame(0x123, "output"));

    device->clear(QCanBusDevice::Output);
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QTRY_VERIFY_WITH_TIMEOUT(spy.size() == 0, 5000);
}

void tst_QCanBusDevice::error()
{
    QSignalSpy spy(device.get(), &QCanBusDevice::errorOccurred);
    QString testString(QStringLiteral("testString"));

    auto backend = qobject_cast<tst_Backend *>(device.get());
    QVERIFY(backend);

    // NoError
    QVERIFY(device->errorString().isEmpty());

    // ReadError
    backend->emulateError(testString + QStringLiteral("a"), QCanBusDevice::ReadError);
    QCOMPARE(testString + QStringLiteral("a"), device->errorString());
    QCOMPARE(device->error(), 1);
    QCOMPARE(spy.size(), 1);

    // WriteError
    backend->emulateError(testString + QStringLiteral("b"), QCanBusDevice::WriteError);
    QCOMPARE(testString + QStringLiteral("b"), device->errorString());
    QCOMPARE(device->error(), 2);
    QCOMPARE(spy.size(), 2);

    // ConnectionError
    backend->emulateError(testString + QStringLiteral("c"), QCanBusDevice::ConnectionError);
    QCOMPARE(testString + QStringLiteral("c"), device->errorString());
    QCOMPARE(device->error(), 3);
    QCOMPARE(spy.size(), 3);

    // ConfigurationError
    backend->emulateError(testString + QStringLiteral("d"), QCanBusDevice::ConfigurationError);
    QCOMPARE(testString + QStringLiteral("d"), device->errorString());
    QCOMPARE(device->error(), 4);
    QCOMPARE(spy.size(), 4);

    // UnknownError
    backend->emulateError(testString + QStringLiteral("e"), QCanBusDevice::UnknownError);
    QCOMPARE(testString + QStringLiteral("e"), device->errorString());
    QCOMPARE(device->error(), 5);
    QCOMPARE(spy.size(), 5);
}

void tst_QCanBusDevice::cleanupTestCase()
{
    device->disconnectDevice();
    QCOMPARE(device->state(), QCanBusDevice::UnconnectedState);
    QCanBusFrame frame = device->readFrame();
    QVERIFY(!frame.frameId());
}

void tst_QCanBusDevice::tst_filtering()
{
    QCanBusDevice::Filter defaultFilter;
    QCOMPARE(defaultFilter.frameId, 0x0u);
    QCOMPARE(defaultFilter.frameIdMask, 0x0u);
    QCOMPARE(defaultFilter.type, QCanBusFrame::InvalidFrame);
    QCOMPARE(defaultFilter.format, QCanBusDevice::Filter::MatchBaseAndExtendedFormat);

    QList<QCanBusDevice::Filter> filters;
    QCanBusDevice::Filter f;
    f.frameId = 0x1;
    f.type = QCanBusFrame::DataFrame;
    f.frameIdMask = 0xFF;
    f.format = QCanBusDevice::Filter::MatchBaseAndExtendedFormat;
    filters.append(f);

    f.frameId = 0x2;
    f.type = QCanBusFrame::RemoteRequestFrame;
    f.frameIdMask = 0x0;
    f.format = QCanBusDevice::Filter::MatchBaseFormat;
    filters.append(f);

    const QVariant wrapper = QVariant::fromValue(filters);
    const auto newFilter = wrapper.value<QList<QCanBusDevice::Filter> >();
    QCOMPARE(newFilter.size(), 2);

    QCOMPARE(newFilter.at(0).type, QCanBusFrame::DataFrame);
    QCOMPARE(newFilter.at(0).frameId, 0x1u);
    QCOMPARE(newFilter.at(0).frameIdMask, 0xFFu);
    QVERIFY(newFilter.at(0).format & QCanBusDevice::Filter::MatchBaseAndExtendedFormat);
    QVERIFY(newFilter.at(0).format & QCanBusDevice::Filter::MatchBaseFormat);
    QVERIFY(newFilter.at(0).format & QCanBusDevice::Filter::MatchExtendedFormat);

    QCOMPARE(newFilter.at(1).type, QCanBusFrame::RemoteRequestFrame);
    QCOMPARE(newFilter.at(1).frameId, 0x2u);
    QCOMPARE(newFilter.at(1).frameIdMask, 0x0u);
    QVERIFY((newFilter.at(1).format & QCanBusDevice::Filter::MatchBaseAndExtendedFormat)
              != QCanBusDevice::Filter::MatchBaseAndExtendedFormat);
    QVERIFY(newFilter.at(1).format & QCanBusDevice::Filter::MatchBaseFormat);
    QVERIFY(!(newFilter.at(1).format & QCanBusDevice::Filter::MatchExtendedFormat));
}

void tst_QCanBusDevice::filterEqual_data()
{
    using Filter = QCanBusDevice::Filter;
    using Frame = QCanBusFrame;

    QTest::addColumn<QCanBusDevice::Filter>("first");
    QTest::addColumn<QCanBusDevice::Filter>("second");
    QTest::addColumn<bool>("isEqual");

    auto filter = [](quint32 frameId, quint32 frameIdMask,
                     Frame::FrameType type,
                     Filter::FormatFilter format) {
        Filter result;
        result.frameId = frameId;
        result.frameIdMask = frameIdMask;
        result.type = type;
        result.format = format;
        return result;
    };

    QTest::newRow("empty-equal")
            << Filter()
            << Filter()
            << true;
    QTest::newRow("empty-default-equal")
            << Filter()
            << filter(0, 0, Frame::InvalidFrame, Filter::MatchBaseAndExtendedFormat)
            << true;
    QTest::newRow("empty-non-default-different")
            << Filter()
            << filter(1, 2, Frame::ErrorFrame, Filter::MatchBaseFormat)
            << false;

    QTest::newRow("frame-id-equal")
            << filter(0x345, 0x800, Frame::RemoteRequestFrame, Filter::MatchBaseFormat)
            << filter(0x345, 0x800, Frame::RemoteRequestFrame, Filter::MatchBaseFormat)
            << true;
    QTest::newRow("frame-id-different")
            << filter(0x345, 0x000, Frame::RemoteRequestFrame, Filter::MatchBaseFormat)
            << filter(0x346, 0x000, Frame::RemoteRequestFrame, Filter::MatchBaseFormat)
            << false;

    QTest::newRow("frame-mask-equal")
            << filter(0x123, 0x7FF, Frame::InvalidFrame, Filter::MatchBaseAndExtendedFormat)
            << filter(0x123, 0x7FF, Frame::InvalidFrame, Filter::MatchBaseAndExtendedFormat)
            << true;
    QTest::newRow("frame-mask-different")
            << filter(0x123, 0x7FF, Frame::InvalidFrame, Filter::MatchBaseAndExtendedFormat)
            << filter(0x123, 0x7FE, Frame::InvalidFrame, Filter::MatchBaseAndExtendedFormat)
            << false;

    QTest::newRow("frame-type-equal")
            << filter(0xFFF, 0xBFF, Frame::DataFrame,    Filter::MatchBaseAndExtendedFormat)
            << filter(0xFFF, 0xBFF, Frame::DataFrame,    Filter::MatchBaseAndExtendedFormat)
            << true;
    QTest::newRow("frame-type-different")
            << filter(0xFFF, 0xBFF, Frame::DataFrame,    Filter::MatchBaseAndExtendedFormat)
            << filter(0xFFF, 0xBFF, Frame::InvalidFrame, Filter::MatchBaseAndExtendedFormat)
            << false;

    QTest::newRow("filter-equal")
            << filter(0xFFF, 0xBFF, Frame::ErrorFrame,    Filter::MatchExtendedFormat)
            << filter(0xFFF, 0xBFF, Frame::ErrorFrame,    Filter::MatchExtendedFormat)
            << true;
    QTest::newRow("filter-different")
            << filter(0xFFF, 0xBFF, Frame::ErrorFrame,    Filter::MatchExtendedFormat)
            << filter(0xFFF, 0xBFF, Frame::ErrorFrame,    Filter::MatchBaseAndExtendedFormat)
            << false;
}

void tst_QCanBusDevice::filterEqual()
{
    QFETCH(QCanBusDevice::Filter, first);
    QFETCH(QCanBusDevice::Filter, second);
    QFETCH(bool, isEqual);

    if (isEqual) {
        QCOMPARE(first, second);
    } else {
        QVERIFY(first != second);
    }
}

void tst_QCanBusDevice::tst_bufferingAttribute()
{
    std::unique_ptr<tst_Backend> canDevice(new tst_Backend);
    QVERIFY(canDevice != nullptr);
    // by default buffered set to true
    QVERIFY(canDevice->isWriteBuffered());

    canDevice->setWriteBuffered(false);
    QVERIFY(!canDevice->isWriteBuffered());
    canDevice->setWriteBuffered(true);
    QVERIFY(canDevice->isWriteBuffered());
}

void tst_QCanBusDevice::tst_waitForFramesReceived()
{
    device->disconnectDevice();
    QVERIFY(!device->waitForFramesReceived(100));
    QCOMPARE(device->error(), QCanBusDevice::OperationError);

    if (device->state() != QCanBusDevice::ConnectedState) {
        QVERIFY(device->connectDevice());
        QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState, 5000);
    }

    QVERIFY(!device->framesAvailable());
    QVERIFY(device->triggerNewFrame());
    QVERIFY(device->framesAvailable());

    // frame is already available, but no new frame comes in
    // while function blocks, therefore times out
    QVERIFY(!device->waitForFramesReceived(2000));
    QCOMPARE(device->error(), QCanBusDevice::TimeoutError);

    QCanBusFrame frame = device->readFrame();
    QVERIFY(frame.isValid());
    QCOMPARE(frame.payload(), QByteArray("FOOBAR"));
    QVERIFY(!device->framesAvailable());

    QElapsedTimer elapsed;
    elapsed.start();
    // no pending frame (should trigger active wait & timeout)
    QVERIFY(!device->waitForFramesReceived(5000));
    QVERIFY(elapsed.hasExpired(4000)); // should have caused time elapse
    QCOMPARE(device->error(), QCanBusDevice::TimeoutError);

    QTimer::singleShot(2000, [&]() { device->triggerNewFrame(); });
    elapsed.restart();
    // frame will be inserted after 2s
    QVERIFY(device->waitForFramesReceived(8000));
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QVERIFY(!elapsed.hasExpired(8000));

    frame = device->readFrame();
    QVERIFY(frame.isValid());
    QCOMPARE(frame.payload(), QByteArray("FOOBAR"));
    QVERIFY(!device->framesAvailable());

    QTimer::singleShot(2000, [&]() {
        device->emulateError(QStringLiteral("TriggerError"), QCanBusDevice::ReadError);
    });
    elapsed.restart();
    // error will be inserted after 2s
    QVERIFY(!device->waitForFramesReceived(8000));
    QVERIFY(!elapsed.hasExpired(8000));
    QCOMPARE(device->errorString(), QStringLiteral("TriggerError"));
    QCOMPARE(device->error(), QCanBusDevice::ReadError);

    // test recursive calling of waitForFramesReceived() behavior
    int handleCounter = 0;
    QTimer::singleShot(1000, [&]() {
        device->triggerNewFrame();
        device->triggerNewFrame();
    });
    QTimer::singleShot(2000, [&]() { device->triggerNewFrame(); });
    QObject::connect(device.get(), &QCanBusDevice::framesReceived, this, [this, &handleCounter]() {
        handleCounter++;
        // this should trigger a recursion which we want to catch
        QVERIFY(!device->waitForFramesReceived(5000));
        // Only the first two frames create a recursion, as the outer
        // waitForFramesReceived() will immediately exit once at least
        // one frame was received. Therefore the third frame here leads
        // to TimeoutError, as no further frame is received.
        if (handleCounter < 3) {
            QCOMPARE(device->error(), QCanBusDevice::OperationError);
        } else {
            QCOMPARE(device->error(), QCanBusDevice::TimeoutError);
        }
    });
    QVERIFY(device->waitForFramesReceived(8000));
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QTRY_COMPARE_WITH_TIMEOUT(handleCounter, 3, 5000);
}

void tst_QCanBusDevice::tst_waitForFramesWritten()
{
    device->disconnectDevice();
    QVERIFY(!device->waitForFramesWritten(100));
    QCOMPARE(device->error(), QCanBusDevice::OperationError);

    if (device->state() != QCanBusDevice::ConnectedState) {
        QVERIFY(!device->waitForFramesWritten(100));
        QCOMPARE(device->error(), QCanBusDevice::OperationError);

        QVERIFY(device->connectDevice());
        QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState, 5000);
    }

    device->setWriteBuffered(false);
    QVERIFY(!device->waitForFramesWritten(1000)); // no buffer, waiting not possible
    QCOMPARE(device->error(), QCanBusDevice::NoError);

    device->setWriteBuffered(true);

    QVERIFY(device->framesToWrite() == 0);
    QVERIFY(!device->waitForFramesWritten(1000)); // nothing in buffer, nothing to wait for
    QCOMPARE(device->error(), QCanBusDevice::NoError);

    QCanBusFrame frame;
    frame.setPayload(QByteArray("testData"));

    // test error case
    QTimer::singleShot(500, [&]() {
        device->emulateError(QStringLiteral("TriggerWriteError"), QCanBusDevice::WriteError);
    });
    device->writeFrame(frame);
    QElapsedTimer elapsed;
    elapsed.start();

    // error will be triggered
    QVERIFY(!device->waitForFramesWritten(8000));
    QVERIFY(!elapsed.hasExpired(8000));
    QCOMPARE(device->errorString(), QStringLiteral("TriggerWriteError"));
    QCOMPARE(device->error(), QCanBusDevice::WriteError);

    // flush remaining frames out to reset the test
    QTRY_VERIFY_WITH_TIMEOUT(device->framesToWrite() == 0, 10000);

    // test timeout
    device->writeFrame(frame);
    QVERIFY(!device->waitForFramesWritten(500));
    QCOMPARE(device->error(), QCanBusDevice::TimeoutError);
    QVERIFY(elapsed.hasExpired(500));

    // flush remaining frames out to reset the test
    QTRY_VERIFY_WITH_TIMEOUT(device->framesToWrite() == 0, 10000);

    device->writeFrame(frame);
    device->writeFrame(frame);
    elapsed.restart();
    QVERIFY(device->waitForFramesWritten(8000));
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QVERIFY(!elapsed.hasExpired(8000));

    // flush remaining frames out to reset the test
    QTRY_VERIFY_WITH_TIMEOUT(device->framesToWrite() == 0, 10000);

    // test recursive calling of waitForFramesWritten() behavior
    int handleCounter = 0;
    device->writeFrame(frame);
    QTimer::singleShot(1000, [&]() { device->writeFrame(frame); });
    QTimer::singleShot(2000, [&]() { device->writeFrame(frame); });
    QObject::connect(device.get(), &QCanBusDevice::framesWritten, this, [this, &handleCounter]() {
        handleCounter++;
        // this should trigger a recursion which we want to catch
        QVERIFY(!device->waitForFramesWritten(5000));
        QCOMPARE(device->error(), QCanBusDevice::OperationError);
    });
    QVERIFY(device->waitForFramesWritten(8000));
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QTRY_COMPARE_WITH_TIMEOUT(handleCounter, 3, 5000);

    device->setWriteBuffered(false);
}

void tst_QCanBusDevice::tst_deviceInfo()
{
    std::unique_ptr<tst_Backend> canDevice(new tst_Backend);
    QVERIFY(canDevice != nullptr);

    auto info = canDevice->deviceInfo();
    QCOMPARE(info.plugin(), u"plugin"_s);
    QCOMPARE(info.name(), u"name"_s);
    QCOMPARE(info.serialNumber(), u"serial number"_s);
    QCOMPARE(info.description(), u"description"_s);
    QCOMPARE(info.alias(), u"alias"_s);
    QCOMPARE(info.channel(), 100);
    QCOMPARE(info.hasFlexibleDataRate(), true);
    QCOMPARE(info.isVirtual(), true);
}

QTEST_MAIN(tst_QCanBusDevice)
Q_IMPORT_PLUGIN(TestCanBusPlugin)

#include "tst_qcanbusdevice.moc"
