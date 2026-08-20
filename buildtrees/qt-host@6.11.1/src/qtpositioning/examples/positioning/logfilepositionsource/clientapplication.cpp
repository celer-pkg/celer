// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "clientapplication.h"
#include "logfilepositionsource.h"

#include <QtWidgets/qtextedit.h>

ClientApplication::ClientApplication(QWidget *parent)
    : QMainWindow(parent)
{
    resize(700, 525);

    textEdit = new QTextEdit;
    setCentralWidget(textEdit);

    LogFilePositionSource *source = new LogFilePositionSource(this);
    connect(source, &LogFilePositionSource::positionUpdated,
            this, &ClientApplication::positionUpdated);

    source->startUpdates();
}

void ClientApplication::positionUpdated(const QGeoPositionInfo &info)
{
    textEdit->append(tr("Position updated: Date/time = %1, Coordinate = %2").
                     arg(info.timestamp().toString(), info.coordinate().toString()));
}
