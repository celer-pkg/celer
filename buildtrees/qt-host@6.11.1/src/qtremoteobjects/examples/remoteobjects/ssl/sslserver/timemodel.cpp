// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "timemodel.h"

MinuteTimer::MinuteTimer(QObject *parent) : MinuteTimerSimpleSource(parent), zone(0)
{
    time = QTime::currentTime();
    setHour(time.hour());
    setMinute(time.minute());
    timer.start(60000-time.second()*1000, this);
}
MinuteTimer::~MinuteTimer()
{
    timer.stop();
}
void MinuteTimer::timerEvent(QTimerEvent *)
{
    QTime now = QTime::currentTime();
    if (now.second() == 59 && now.minute() == time.minute() && now.hour() == time.hour()) {
        // just missed time tick over, force it, wait extra 0.5 seconds
        time = time.addSecs(60);
        timer.start(60500, this);
    } else {
        time = now;
        timer.start(60000-time.second()*1000, this);
    }
    qDebug()<<"Time"<<time;
    setHour(time.hour());
    setMinute(time.minute());
    emit timeChanged();
    emit timeChanged2(time);
    static PresetInfo bla(3, 93.9f, "Best Station");
    emit sendCustom(bla);
}
void MinuteTimer::SetTimeZone(const int &zn)
{
    qDebug()<<"SetTimeZone"<<zn;
    if (zn != zone)
    {
        zone = zn;
    }
}
