// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//TESTED_COMPONENT=src/location

#include "tst_qnmeapositioninfosource.h"

class tst_QNmeaPositionInfoSource_Simulation_Generic : public TestQGeoPositionInfoSource
{
    Q_OBJECT
public:
    tst_QNmeaPositionInfoSource_Simulation_Generic()
    {
#if QT_CONFIG(library)
        /*
         * Set custom path since CI doesn't install test plugins
         */
#ifdef Q_OS_WIN
        QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() +
                                     QStringLiteral("/../../../../plugins"));
#else
        QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath()
                                         + QStringLiteral("/../../../../plugins"));
#endif
#endif
    }

protected:
    QGeoPositionInfoSource *createTestSource() override
    {
        QNmeaPositionInfoSource *source = new QNmeaPositionInfoSource(QNmeaPositionInfoSource::SimulationMode);
        source->setDevice(new UnlimitedNmeaStream(source));
        return source;
    }
};

#include "tst_qnmeapositioninfosource_simulation_generic.moc"

QTEST_GUILESS_MAIN(tst_QNmeaPositionInfoSource_Simulation_Generic);
