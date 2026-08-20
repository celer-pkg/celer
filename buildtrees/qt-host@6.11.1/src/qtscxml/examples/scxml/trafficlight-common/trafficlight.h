// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qabstractbutton.h>
#include <QtScxml/qscxmlstatemachine.h>

class TrafficLight : public QWidget
{
    Q_OBJECT

public:
    TrafficLight(QScxmlStateMachine *machine, QWidget *parent = nullptr);

private slots:
    void toggleWorking(bool pause);

private:
    QScxmlStateMachine *m_machine;
};

class LightWidget: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool on READ isOn WRITE setOn)

public:
    LightWidget(const QString &image, QWidget *parent = nullptr);

    bool isOn() const;
    void setOn(bool on);

public slots:
    void switchLight(bool onoff);

protected:
    void paintEvent(QPaintEvent *) override;
    QSize sizeHint() const override;

private:
    QImage m_image;
    bool m_on = false;
};

class ButtonWidget : public QAbstractButton
{
    Q_OBJECT
public:
    ButtonWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
    QSize sizeHint() const override;

private:
    QImage m_playIcon;
    QImage m_pauseIcon;
};

#endif // TRAFFICLIGHT_H
