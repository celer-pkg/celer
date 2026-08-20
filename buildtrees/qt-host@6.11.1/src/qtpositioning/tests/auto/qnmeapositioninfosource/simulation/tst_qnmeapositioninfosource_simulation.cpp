// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//TESTED_COMPONENT=src/location

#include "tst_qnmeapositioninfosource.h"

class tst_QNmeaPositionInfoSource_Simulation : public tst_QNmeaPositionInfoSource
{
    Q_OBJECT
public:
    tst_QNmeaPositionInfoSource_Simulation()
        : tst_QNmeaPositionInfoSource(QNmeaPositionInfoSource::SimulationMode) {}
};

#include "tst_qnmeapositioninfosource_simulation.moc"

QTEST_GUILESS_MAIN(tst_QNmeaPositionInfoSource_Simulation);
