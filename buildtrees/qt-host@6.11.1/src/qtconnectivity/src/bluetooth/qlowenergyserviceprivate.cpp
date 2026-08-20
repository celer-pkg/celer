// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qlowenergyserviceprivate_p.h"

#include "qlowenergycontrollerbase_p.h"

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN_TAGGED(QSharedPointer<QLowEnergyServicePrivate>,
                               QSharedPointer_QLowEnergyServicePrivate)

QLowEnergyServicePrivate::QLowEnergyServicePrivate(QObject *parent) : QObject(parent) { }

QLowEnergyServicePrivate::~QLowEnergyServicePrivate()
{
}

void QLowEnergyServicePrivate::setController(QLowEnergyControllerPrivate *control)
{
    controller = control;

    if (control)
        setState(QLowEnergyService::RemoteService);
    else
        setState(QLowEnergyService::InvalidService);
}

void QLowEnergyServicePrivate::setError(QLowEnergyService::ServiceError newError)
{
    lastError = newError;
    emit errorOccurred(newError);
}

void QLowEnergyServicePrivate::setState(QLowEnergyService::ServiceState newState)
{
    if (state == newState)
        return;

    state = newState;
    emit stateChanged(newState);
}

QT_END_NAMESPACE

#include "moc_qlowenergyserviceprivate_p.cpp"
