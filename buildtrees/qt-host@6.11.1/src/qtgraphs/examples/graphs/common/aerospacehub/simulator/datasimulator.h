// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DATASIMULATOR_H
#define DATASIMULATOR_H

#include <QObject>
#include <QMutex>

class DataSimulator : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

    Q_PROPERTY(qreal max READ max WRITE setMax NOTIFY maxChanged)
    Q_PROPERTY(qreal min READ min WRITE setMin NOTIFY minChanged)
    Q_PROPERTY(int numberOfData READ numberOfData WRITE setNumberOfData NOTIFY numberOfDataChanged)
    Q_PROPERTY(bool live READ isLive WRITE setLive NOTIFY liveChanged)
    Q_PROPERTY(DataSimulator::SortingOrder order READ order WRITE setOrder NOTIFY orderChanged)
    Q_PROPERTY(int deviation READ deviation WRITE setDeviation NOTIFY deviationChanged)
    Q_PROPERTY(int updatePeriod READ updatePeriod WRITE setUpdatePeriod NOTIFY updatePeriodChanged)
    Q_PROPERTY(QVariantList data READ data)

public:
    enum class SortingOrder {
        Ascending,
        Descending,
        NotSorted,
    };
    Q_ENUM(SortingOrder)

    explicit DataSimulator(QObject *parent = 0);
    ~DataSimulator() override = default;

    qreal max() const;
    void setMax(qreal max);
    qreal min() const;
    void setMin(qreal min);
    int numberOfData() const;
    void setNumberOfData(int numberOfData);
    bool isLive() const;
    void setLive(bool live);
    DataSimulator::SortingOrder order() const;
    int deviation() const;
    void setDeviation(int deviation);
    int updatePeriod() const;
    void setUpdatePeriod(int updatePeriod);
    void setOrder(DataSimulator::SortingOrder order);

    Q_INVOKABLE virtual void generateData();

    QVariantList data();

Q_SIGNALS:
    void maxChanged(qreal max);
    void minChanged(qreal min);
    void numberOfDataChanged(int numberOfData);
    void liveChanged(bool live);
    void orderChanged(DataSimulator::SortingOrder order);
    void simulateDataCompleted(QVariantList data, int numberOfColumns, int numberOfRows, int numberOfData);
    void deviationChanged(int deviation);
    void updatePeriodChanged(int updatePeriod);

protected:
    void generateData(int numberOfSet, int numberOfColumns, int numberOfRows = 1);
    bool hasData() const;

private:
    void updateData();
    void createThread();

    qreal m_max = 0;
    qreal m_min = -1;
    int m_numberOfData = 1;
    int m_numberOfColumns = 1;
    int m_numberOfRows = 1;
    QVariantList m_data;
    bool m_live = false;
    DataSimulator::SortingOrder m_order = SortingOrder::NotSorted;
    int m_deviation = 1;
    bool m_hasData = false;
    std::atomic<bool> m_isRunning{false};
    int m_updatePeriod = 6000;
    QMutex m_mutex;
    QThread *m_updateThread = nullptr;
};

#endif // DATASIMULATOR_H
