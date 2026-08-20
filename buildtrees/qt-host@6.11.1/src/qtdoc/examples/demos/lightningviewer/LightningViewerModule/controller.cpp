// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "controller.h"
#include "laststrikeinfo.h"
#include "lightningitemmodel.h"
#include "lightningprovider.h"

namespace {
constexpr int NOTIFICATION_RADIUS = 100000;  // meters
}

Controller::Controller(QObject *parent)
    : QObject{parent}
{
    connect(&m_provider, &LightningProvider::dataReady, this, &Controller::onDataReceived);
}

Controller::~Controller()
{
}

QAbstractItemModel *Controller::getModel()
{
    return &m_model;
}

int Controller::getLastStrikeTime()
{
    return m_lastStrikeInfo.isValid() ? m_lastStrikeInfo.timestamp.count() : -1;
}

double Controller::getLastStrikeDistance()
{
    return m_lastStrikeInfo.isValid() ? m_lastStrikeInfo.distance : -1;
}

double Controller::getLastStrikeDirection()
{
    return m_lastStrikeInfo.isValid() ? m_lastStrikeInfo.direction : -1;
}

bool Controller::isDistanceTimeLayerEnabled() const
{
    return m_distanceTimeLayerEnabled;
}

void Controller::updateDistanceTime()
{
    if (!isDistanceTimeLayerEnabled())
        return;

    m_lastStrikeInfo.invalidate();
    m_model.getLatestStrikeInfo(m_userLocation, NOTIFICATION_RADIUS, &m_lastStrikeInfo);

    emit lastStrikeInfoUpdated();
}

void Controller::updateDistanceTime(const LightningItemData &data)
{
    if (!isDistanceTimeLayerEnabled())
        return;

    LastStrikeInfo strikeInfo(data.getDistanceTo(m_userLocation),
                              data.timestamp,
                              data.getDirectionFrom(m_userLocation));
    if (strikeInfo.distance > NOTIFICATION_RADIUS)
        return;

    m_lastStrikeInfo = strikeInfo;

    emit lastStrikeInfoUpdated();
}

void Controller::onDataReceived(const LightningItemData &data)
{
    m_model.insertData(data);

    updateDistanceTime(data);
}

void Controller::setUserLocation(const QGeoCoordinate &coordinate)
{
    if (m_userLocation == coordinate)
        return;
    m_userLocation = coordinate;

    updateDistanceTime();
}

void Controller::setDistanceTimeLayerEnabled(bool enabled)
{
    if (m_distanceTimeLayerEnabled == enabled)
        return;
    m_distanceTimeLayerEnabled = enabled;
    emit distanceTimeLayerEnabledChanged();

    updateDistanceTime();
}
