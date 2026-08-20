// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TESTS_ENGINE_H
#define TESTS_ENGINE_H

#include "rep_engine_source.h"

class Engine : public EngineSimpleSource
{
    Q_OBJECT
    Q_PROPERTY(bool purchasedPart READ purchasedPart WRITE setpurchasedPart)

public:
    Engine(int cylinders = 4, QObject *parent = nullptr);
    ~Engine() override;

    bool start() override;
    void increaseRpm(int deltaRpm) override;

    void unnormalizedSignature(int, int) override {}

    Temperature temperature() override;
    void setTemperature(const Temperature &value);

    void setSharedTemperature(const Temperature::Ptr &) override {}

    bool purchasedPart() {return _purchasedPart;}

public Q_SLOTS:
    void setpurchasedPart(bool value);

    QString myTestString() override { return _myTestString; }
    void setMyTestString(QString value) override { _myTestString = value; }

private:
    bool _purchasedPart;
    QString _myTestString;
    Temperature _temperature;
};

#endif
