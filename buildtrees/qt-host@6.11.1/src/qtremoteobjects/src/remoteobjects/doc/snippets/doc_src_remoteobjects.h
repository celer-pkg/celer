// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [qtremoteobject_include]
#include <QtRemoteObjects>
//! [qtremoteobject_include]

//! [implementing_source]
#include "rep_TimeModel_source.h"

class MinuteTimer : public MinuteTimerSource
{
Q_OBJECT
public:
    MinuteTimer(QObject *parent = nullptr);
    virtual ~MinuteTimer();

public slots:
    virtual void SetTimeZone(int zn) {  //this is a pure virtual method in MinuteTimerSource
        qDebug()<<"SetTimeZone"<<zn;
        if (zn != zone) {
            zone = zn;
        }
    };

protected:
    void timerEvent(QTimerEvent *);

private:
    QTime time;
    QBasicTimer timer;
    int zone;
};
//! [implementing_source]
