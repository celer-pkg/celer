// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//TESTED_COMPONENT=src/location

#include "tst_qnmeapositioninfosource.h"

class tst_QNmeaPositionInfoSource_RealTime : public tst_QNmeaPositionInfoSource
{
    Q_OBJECT

public:
    tst_QNmeaPositionInfoSource_RealTime()
        : tst_QNmeaPositionInfoSource(QNmeaPositionInfoSource::RealTimeMode) {}
};

#include "tst_qnmeapositioninfosource_realtime.moc"

QTEST_GUILESS_MAIN(tst_QNmeaPositionInfoSource_RealTime);
