// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#ifndef WIDGET_H
#define WIDGET_H

#include "logwidget.h"

#include <QWidget>
#include <QGeoPositionInfoSource>

namespace Ui {
    class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(LogWidget *log, QWidget *parent = nullptr);
    ~Widget();

public slots:
    void positionUpdated(QGeoPositionInfo gpsPos);
    void setInterval(int msec);
    void positionTimedOut();
    void errorChanged(QGeoPositionInfoSource::Error err);
private slots:
    void on_buttonRetrieve_clicked();
    void on_buttonStart_clicked();
    void on_radioButton_2_clicked();
    void on_radioButton_clicked();
    void on_radioButton_3_clicked();
    void on_radioButton_4_clicked();

    void on_buttonUpdateSupported_clicked();
    void on_buttonResetError_clicked();

private:
    LogWidget *log = nullptr;
    Ui::Widget *ui;
    QGeoPositionInfoSource *m_posSource;
};

#endif // WIDGET_H
