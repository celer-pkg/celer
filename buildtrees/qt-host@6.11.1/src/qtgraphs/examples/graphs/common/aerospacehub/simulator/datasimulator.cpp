// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include "datasimulator.h"

#include <QRandomGenerator>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QVariant>

DataSimulator::DataSimulator(QObject *parent) : QObject(parent)
{
}

void DataSimulator::createThread()
{
    m_updateThread = QThread::create([this]() {
        QTimer timer;
        QEventLoop eventLoop;
        connect(&timer, &QTimer::timeout, this, [this]() {
            if (m_isRunning.load())
                return;

            m_isRunning = true;

            updateData();

            m_isRunning = false;
        });

        timer.start(m_updatePeriod);

        connect(m_updateThread, &QThread::finished, &eventLoop, &QEventLoop::quit);
        eventLoop.exec();
    });
}

qreal DataSimulator::max() const
{
    return m_max;
}

void DataSimulator::setMax(qreal max)
{
    if (qFuzzyCompare(m_max, max))
        return;

    m_max = max;
    emit maxChanged(max);
}

qreal DataSimulator::min() const
{
    return m_min;
}

void DataSimulator::setMin(qreal min)
{
    if (qFuzzyCompare(m_min, min))
        return;

    m_min = min;
    emit minChanged(min);
}

int DataSimulator::numberOfData() const
{
    return m_numberOfData;
}

void DataSimulator::setNumberOfData(int numberOfData)
{
    if (m_numberOfData == numberOfData)
        return;

    m_numberOfData = numberOfData;
    emit numberOfDataChanged(numberOfData);
}

bool DataSimulator::isLive() const
{
    return m_live;
}

void DataSimulator::setLive(bool live)
{
    if (m_live == live)
        return;

    m_live = live;
    if (m_live && m_hasData) {
        createThread();
        m_updateThread->start();
    } else {
        m_updateThread->quit();
        delete m_updateThread;
    }
    emit liveChanged(live);
}

DataSimulator::SortingOrder DataSimulator::order() const
{
    return m_order;
}

void DataSimulator::setOrder(DataSimulator::SortingOrder order)
{
    if (m_order == order)
        return;

    m_order = order;
    emit orderChanged(order);
}

int DataSimulator::deviation() const
{
    return m_deviation;
}

void DataSimulator::setDeviation(int deviation)
{
    if (m_deviation == deviation)
        return;

    m_deviation = deviation;
    emit deviationChanged(deviation);
}

int DataSimulator::updatePeriod() const
{
    return m_updatePeriod;
}

void DataSimulator::setUpdatePeriod(int updatePeriod)
{
    if (m_updatePeriod == updatePeriod)
        return;

    const bool wasLive = isLive();

    setLive(false);

    m_updatePeriod = updatePeriod;
    emit updatePeriodChanged(updatePeriod);

    setLive(wasLive);
}

QVariantList DataSimulator::data()
{
    return m_data;
}

void DataSimulator::generateData()
{
    generateData(1, 1);
}

void DataSimulator::generateData(int numberOfSet, int numberOfColumns, int numberOfRows)
{
    m_numberOfColumns = numberOfColumns;
    m_numberOfRows = numberOfRows;
    int numberOfCategory = numberOfColumns * numberOfRows;
    int size = m_numberOfData * numberOfCategory * numberOfSet;
    qreal range = m_max - m_min;
    qreal prev = m_order == SortingOrder::Ascending ? m_min : m_max;
    m_data.clear();

    float highValue = -1000;
    float lowValue = 1000;

    for (int i = 0; i < size; i++) {
        qreal randData;
        qreal diff;
        if (m_order != SortingOrder::NotSorted) {
            diff = QRandomGenerator::global()->bounded(range / numberOfSet);
            if (m_deviation > 0)
                diff *= QRandomGenerator::global()->bounded(qreal(m_deviation));
            diff *= QRandomGenerator::global()->bounded(-1, 2);
            if (m_order == SortingOrder::Ascending)
                randData = prev + diff;
            else
                randData = prev - diff;
        } else {
            diff = QRandomGenerator::global()->bounded(range);
            randData = diff;
        }

        m_data.append(QVariant(randData));
        if (i + 1 < numberOfSet)
            continue;
        prev = m_data.at(m_data.size() - numberOfSet).toReal();
        highValue = qMax(highValue, prev);
        lowValue = qMin(lowValue, prev);
    }

    //remap data to min-max
    qreal currentRange = highValue - lowValue;
    for (auto &data : m_data) {
        qreal normalizedData = (data.toReal() - lowValue) / currentRange;
        data = QVariant(normalizedData * range + m_min);
    }

    m_hasData = true;
    if (m_live) {
        if (m_updateThread->isRunning()) {
            connect(m_updateThread, &QThread::finished, this, [this]() {
                m_updateThread->start();
            }, Qt::SingleShotConnection);
            m_updateThread->quit();
        } else {
            m_updateThread->start();
        }
    }
    emit simulateDataCompleted(m_data, numberOfColumns, numberOfRows, m_numberOfData);
}

bool DataSimulator::hasData() const
{
    return m_hasData;
}

void DataSimulator::updateData()
{
    QMutexLocker locker(&m_mutex);

    for (int i = 0; i < m_data.size(); i++) {
        QVariant data = m_data.at(i);
        qreal diff = QRandomGenerator::global()->bounded(qreal(m_deviation));
        diff *= QRandomGenerator::global()->bounded(-1, 2);
        QVariant rand = data.toReal() + diff;
        while (rand.toReal() < m_min || rand.toReal() > m_max) {
            diff = QRandomGenerator::global()->bounded(qreal(m_deviation));
            diff *= QRandomGenerator::global()->bounded(-1, 2);
            rand = data.toReal() + diff;
            if (data.toReal() > m_max || data.toReal() < m_min)
                break;
        }
        m_data.replace(i, rand);
    }
    emit simulateDataCompleted(m_data, m_numberOfColumns, m_numberOfRows, m_numberOfData);
}
