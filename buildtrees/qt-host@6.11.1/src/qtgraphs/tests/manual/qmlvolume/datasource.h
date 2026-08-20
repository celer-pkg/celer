// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QtGraphs/QCustom3DVolume>
#include <QtCore/QObject>

class DataSource : public QObject
{
    Q_OBJECT
public:
    explicit DataSource(QObject *parent = 0);
    ~DataSource() override;

public:
    Q_INVOKABLE void fillVolume(QCustom3DVolume *volumeItem);
};

#endif
