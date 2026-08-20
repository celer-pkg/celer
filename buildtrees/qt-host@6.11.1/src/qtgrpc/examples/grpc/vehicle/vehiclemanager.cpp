// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "vehiclemanager.h"
#include "navigationthread.h"
#include "vehiclethread.h"

using namespace qtgrpc::examples;

VehicleManager::VehicleManager(QObject *parent) : QObject(parent)
{
    startVehicleClient();
    startNavigationClient();
}

VehicleManager::~VehicleManager()
{
    if (m_vehicleThread && m_vehicleThread->isRunning()) {
        m_vehicleThread->quit();
        m_vehicleThread->wait();
    }

    if (m_navigationThread && m_navigationThread->isRunning()) {
        m_navigationThread->quit();
        m_navigationThread->wait();
    }
}

void VehicleManager::startNavigationClient()
{
    m_navigationThread = std::make_unique<NavigationThread>();

    connect(m_navigationThread.get(), &NavigationThread::totalDistanceChanged, this,
            &VehicleManager::totalDistanceChanged, Qt::QueuedConnection);
    connect(m_navigationThread.get(), &NavigationThread::traveledDistanceChanged, this,
            &VehicleManager::traveledDistanceChanged, Qt::QueuedConnection);

    connect(
        m_navigationThread.get(), &NavigationThread::directionChanged, this,
        [this](qtgrpc::examples::DirectionEnumGadget::DirectionEnum direction) {
            VehicleManager::directionChanged(qToUnderlying(direction));
        },
        Qt::QueuedConnection);

    connect(m_navigationThread.get(), &NavigationThread::streetChanged, this,
            &VehicleManager::streetChanged, Qt::QueuedConnection);

    connect(m_navigationThread.get(), &NavigationThread::connectionError, this,
            &VehicleManager::addNavigationError, Qt::QueuedConnection);

    m_navigationThread->start();
}

void VehicleManager::startVehicleClient()
{
    m_vehicleThread = std::make_unique<VehicleThread>();

    connect(m_vehicleThread.get(), &VehicleThread::speedChanged, this,
            &VehicleManager::speedChanged, Qt::QueuedConnection);
    connect(m_vehicleThread.get(), &VehicleThread::autonomyChanged, this,
            &VehicleManager::autonomyChanged, Qt::QueuedConnection);
    connect(m_vehicleThread.get(), &VehicleThread::rpmChanged, this, &VehicleManager::rpmChanged,
            Qt::QueuedConnection);

    connect(m_vehicleThread.get(), &VehicleThread::connectionError, this,
            &VehicleManager::addVehicleError, Qt::QueuedConnection);

    m_vehicleThread->start();
}

void VehicleManager::addVehicleError(QString error)
{
    m_vehicleErrors = m_vehicleErrors.isEmpty() ? error : m_vehicleErrors + "\n" + error;
    emit vehicleErrorsChanged(m_vehicleErrors);
}

void VehicleManager::addNavigationError(QString error)
{
    m_navigationErrors = m_navigationErrors.isEmpty() ? error : m_navigationErrors + "\n" + error;
    emit navigationErrorsChanged(m_navigationErrors);
}

void VehicleManager::restart()
{
    m_vehicleErrors = QString();
    m_navigationErrors = QString();

    if (m_vehicleThread->isRunning()) {
        m_vehicleThread->quit();
        m_vehicleThread->wait();
        m_vehicleThread->start();
    }

    if (m_navigationThread->isRunning()) {
        m_navigationThread->quit();
        m_navigationThread->wait();
        m_navigationThread->start();
    }
}
