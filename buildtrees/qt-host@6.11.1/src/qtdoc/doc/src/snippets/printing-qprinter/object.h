// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QObject>

class Object : public QObject
{
    Q_OBJECT

public:
    Object(QObject *parent = 0);

public slots:
    void print();
};
