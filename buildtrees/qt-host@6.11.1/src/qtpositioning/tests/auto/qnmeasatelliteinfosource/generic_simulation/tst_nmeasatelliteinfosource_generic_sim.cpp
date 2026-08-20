// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QtPositioning/QNmeaSatelliteInfoSource>
#include "../../qgeosatelliteinfosource/testqgeosatelliteinfosource_p.h"
#include "../../utils/qlocationtestutils_p.h"

class UnlimitedNmeaStream : public QIODevice
{
    Q_OBJECT

public:
    UnlimitedNmeaStream(QObject *parent) : QIODevice(parent) {}

protected:
    qint64 readData(char *data, qint64 maxSize) override
    {
        if (maxSize == 0)
            return 0;
        QByteArray bytes;
        if (genSatInView) {
            increaseSatId();
            bytes = QLocationTestUtils::createGsvVariableSentence(satId).toLatin1();
        } else {
            bytes = QLocationTestUtils::createGsaVariableSentence(satId).toLatin1();
        }
        genSatInView = !genSatInView;
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

private:
    void increaseSatId()
    {
        if (++satId == 0)
            satId = 1;
    }

    quint8 satId = 0;
    bool genSatInView = true;
};

class tst_QNmeaSatelliteInfoSource_Generic_Simulation : public TestQGeoSatelliteInfoSource
{
    Q_OBJECT
protected:
    QGeoSatelliteInfoSource *createTestSource() override
    {
        QNmeaSatelliteInfoSource *source =
                new QNmeaSatelliteInfoSource(QNmeaSatelliteInfoSource::UpdateMode::SimulationMode);
        source->setDevice(new UnlimitedNmeaStream(source));
        return source;
    }
};

#include "tst_nmeasatelliteinfosource_generic_sim.moc"

QTEST_GUILESS_MAIN(tst_QNmeaSatelliteInfoSource_Generic_Simulation)
