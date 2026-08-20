// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore>
#include "rep_timemodel_source.h"

class MinuteTimer : public MinuteTimerSimpleSource
{
    Q_OBJECT
public:
    MinuteTimer(QObject *parent = nullptr);
    ~MinuteTimer() override;

public slots:
    void SetTimeZone(const int &zn) override;

protected:
    void timerEvent(QTimerEvent *) override;

private:
    QTime time;
    QBasicTimer timer;
    int zone;
};
