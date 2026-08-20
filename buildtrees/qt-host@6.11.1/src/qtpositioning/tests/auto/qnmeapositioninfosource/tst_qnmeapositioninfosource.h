// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../qgeopositioninfosource/testqgeopositioninfosource_p.h"
#include "../utils/qnmeaproxyfactory.h"
#include "../utils/qlocationtestutils_p.h"

#include <QtPositioning/qnmeapositioninfosource.h>

#include <QTest>
#include <QDir>
#include <QDebug>
#include <QBuffer>
#include <QSignalSpy>
#include <QMetaType>
#include <QFile>
#include <QTemporaryFile>
#include <QHash>
#include <QTimer>

QT_USE_NAMESPACE
Q_DECLARE_METATYPE(QNmeaPositionInfoSource::UpdateMode)
Q_DECLARE_METATYPE(QList<QDateTime>)

class tst_QNmeaPositionInfoSource : public QObject
{
    Q_OBJECT

public:
    enum UpdateTriggerMethod
    {
        StartUpdatesMethod,
        RequestUpdatesMethod
    };

    tst_QNmeaPositionInfoSource(QNmeaPositionInfoSource::UpdateMode mode, QObject *parent = 0);

private:
    QList<QDateTime> createDateTimes(const QDateTime &dt, int count) const
    {
        QList<QDateTime> dateTimes;
        int interval = 100;
        for (int i=0; i<count; i++) {
            dateTimes << dt.addMSecs(interval);
            interval += 100;
        }
        return dateTimes;
    }

    QList<QDateTime> createDateTimes(int count) const
    {
        return createDateTimes(QDateTime::currentDateTime().toUTC(), count);
    }

private slots:
    void initTestCase();

    void constructor();

    void supportedPositioningMethods();

    void minimumUpdateInterval();

    void userEquivalentRangeError();

    void setUpdateInterval_delayedUpdate();

    void lastKnownPosition();

    void beginWithBufferedData();
    void beginWithBufferedData_data();

    void startUpdates();
    void startUpdates_data();

    void startUpdates_withTimeout();

    void startUpdates_expectLatestUpdateOnly();

    void startUpdates_waitForValidDateTime();
    void startUpdates_waitForValidDateTime_data();

    void requestUpdate_waitForValidDateTime();
    void requestUpdate_waitForValidDateTime_data();

    void requestUpdate();
    void requestUpdate_after_start();

    void testWithBadNmea();
    void testWithBadNmea_data();

private:
    QNmeaPositionInfoSource::UpdateMode m_mode;
};

Q_DECLARE_METATYPE(tst_QNmeaPositionInfoSource::UpdateTriggerMethod)

//---------------------------------------------------

class Feeder : public QObject
{
    Q_OBJECT

public:
    Feeder(QObject *parent)
        : QObject(parent)
    {
    }

    void start(QNmeaPositionInfoSourceProxy *proxy)
    {
        m_proxy = proxy;
        QTimer *timer = new QTimer(this);
        QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
        timer->setInterval(proxy->source()->minimumUpdateInterval()*2);
        timer->start();
    }

public slots:
    void timeout()
    {
        m_proxy->feedBytes(QLocationTestUtils::createRmcSentence(QDateTime::currentDateTime()).toLatin1());
    }

private:
    QNmeaPositionInfoSourceProxy *m_proxy;
};

//---------------------------------------------------


class UnlimitedNmeaStream : public QIODevice
{
    Q_OBJECT

public:
    UnlimitedNmeaStream(QObject *parent) : QIODevice(parent) {}

protected:
    qint64 readData(char *data, qint64 maxSize) override
    {
        QByteArray bytes = QLocationTestUtils::createRmcSentence(QDateTime::currentDateTime()).toLatin1();
        qint64 sz = qMin(qint64(bytes.size()), maxSize);
        memcpy(data, bytes.constData(), sz);
        return sz;
    }

    qint64 writeData(const char *, qint64) override
    {
        return -1;
    }

    qint64 bytesAvailable() const override
    {
        return 1024 + QIODevice::bytesAvailable();
    }
};
