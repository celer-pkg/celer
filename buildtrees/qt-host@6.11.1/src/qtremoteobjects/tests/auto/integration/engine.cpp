// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "engine.h"

Engine::Engine(int cylinders, QObject *parent) :
  EngineSimpleSource(cylinders, parent)
{
    setRpm(0);
    setpurchasedPart(false);
}

Engine::~Engine()
{
}

bool Engine::start()
{
    if (started())
        return false; // already started

    setStarted(true);
    return true;
}

void Engine::increaseRpm(int deltaRpm)
{
    setRpm(rpm() + deltaRpm);
}

Temperature Engine::temperature()
{
    return _temperature;
}

void Engine::setTemperature(const Temperature &value)
{
    _temperature = value;
}

void Engine::setpurchasedPart(bool value)
{
    _purchasedPart = value;
}
