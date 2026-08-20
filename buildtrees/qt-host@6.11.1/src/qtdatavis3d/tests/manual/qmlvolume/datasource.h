// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QtDataVisualization/QCustom3DVolume>
#include <QtCore/QObject>

class DataSource : public QObject
{
    Q_OBJECT
public:
    explicit DataSource(QObject *parent = 0);
    virtual ~DataSource();

public:
    Q_INVOKABLE void fillVolume(QCustom3DVolume *volumeItem);
};

#endif
