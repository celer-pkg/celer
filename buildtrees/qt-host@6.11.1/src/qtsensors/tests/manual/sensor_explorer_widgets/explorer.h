// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef EXPLORER_H
#define EXPLORER_H

#include <QMainWindow>
#include <ui_explorer.h>
#include <qsensor.h>


class Explorer : public QMainWindow, public QSensorFilter
{
    Q_OBJECT
public:
    Explorer(QWidget *parent = 0);
    ~Explorer();

    bool filter(QSensorReading *reading) override;

private slots:
    void loadSensors();
    void on_sensors_currentItemChanged();
    void on_sensorprops_itemChanged(QTableWidgetItem *item);
    void on_start_clicked();
    void on_stop_clicked();
    void sensor_changed();
    void adjustSizes();
    void loadSensorProperties();

private:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void clearReading();
    void loadReading();
    void clearSensorProperties();
    void adjustTableColumns(QTableWidget *table);
    void resizeSensors();

    Ui::Explorer ui;
    QSensor *m_sensor;
    bool ignoreItemChanged;
};

#endif

