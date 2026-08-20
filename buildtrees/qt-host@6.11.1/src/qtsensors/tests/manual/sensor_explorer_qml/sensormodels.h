// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QSEONSOREXPLORER_H
#define QSEONSOREXPLORER_H

#include <QtSensors/qsensor.h>

#include <QtQml/qqml.h>
#include <QtCore/QAbstractListModel>
#include <QtCore/QAbstractTableModel>

QT_BEGIN_NAMESPACE

class AvailableSensorsModel: public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit AvailableSensorsModel(QObject* parent = nullptr);
    int rowCount(const QModelIndex & = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Q_INVOKABLE QSensor* get(int index) const;

private:
    void loadSensors();
    QList<QSensor*> m_availableSensors;
};

class SensorPropertyModel: public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(QSensor* sensor READ sensor WRITE setSensor NOTIFY sensorChanged)
    QML_ELEMENT

public:
    explicit SensorPropertyModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex & = QModelIndex()) const override;
    int columnCount(const QModelIndex & = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void setSensor(QSensor* sensor);
    QSensor* sensor() const;

signals:
    void sensorChanged();

private slots:
    void onReadingChanged();

private:
    QSensor* m_sensor = nullptr;
    // m_values is used to cache sensor property values to avoid
    // full metaobject iteration on every sensor reading change
    QList<std::tuple<QByteArray, QByteArray>> m_values;
};

QT_END_NAMESPACE

#endif // QSEONSOREXPLORER_H
