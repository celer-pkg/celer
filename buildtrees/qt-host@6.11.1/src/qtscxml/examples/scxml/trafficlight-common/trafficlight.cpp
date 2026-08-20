// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "trafficlight.h"

#include <QtWidgets/qboxlayout.h>
#include <QtGui/qpainter.h>

using namespace Qt::Literals::StringLiterals;

class TrafficLightWidget : public QWidget
{
public:
    TrafficLightWidget(QWidget *parent = nullptr)
        : QWidget(parent), m_background(":/background.png"_L1)
    {
        QVBoxLayout *vbox = new QVBoxLayout(this);
        vbox->setContentsMargins(0, 40, 0, 80);
        m_red = new LightWidget(":/red.png"_L1);
        vbox->addWidget(m_red, 0, Qt::AlignHCenter);
        m_yellow = new LightWidget(":/yellow.png"_L1);
        vbox->addWidget(m_yellow, 0, Qt::AlignHCenter);
        m_green = new LightWidget(":/green.png"_L1);
        vbox->addWidget(m_green, 0, Qt::AlignHCenter);
        setLayout(vbox);
    }

    LightWidget *redLight() const
    { return m_red; }
    LightWidget *yellowLight() const
    { return m_yellow; }
    LightWidget *greenLight() const
    { return m_green; }

    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawImage(0, 0, m_background);
    }

    QSize sizeHint() const override
    {
        return m_background.size();
    }

private:
    QImage m_background;
    LightWidget *m_red;
    LightWidget *m_yellow;
    LightWidget *m_green;
};

TrafficLight::TrafficLight(QScxmlStateMachine *machine, QWidget *parent)
    : QWidget(parent)
    , m_machine(machine)
{
    TrafficLightWidget *widget = new TrafficLightWidget(this);
    setFixedSize(widget->sizeHint());

    machine->connectToState(u"red"_s, widget->redLight(), &LightWidget::switchLight);
    machine->connectToState(u"redGoingGreen"_s, widget->redLight(), &LightWidget::switchLight);
    machine->connectToState(u"yellow"_s, widget->yellowLight(), &LightWidget::switchLight);
    machine->connectToState(u"blinking"_s, widget->yellowLight(), &LightWidget::switchLight);
    machine->connectToState(u"green"_s, widget->greenLight(), &LightWidget::switchLight);

    QAbstractButton *button = new ButtonWidget(this);
    auto setButtonGeometry = [this, button](){
        QSize buttonSize = button->sizeHint();
        button->setGeometry(width() - buttonSize.width() - 20,
                            height() - buttonSize.height() - 20,
                            buttonSize.width(), buttonSize.height());
    };
    connect(button, &QAbstractButton::toggled, this, setButtonGeometry);
    setButtonGeometry();

    connect(button, &QAbstractButton::toggled, this, &TrafficLight::toggleWorking);
}

void TrafficLight::toggleWorking(bool pause)
{
    m_machine->submitEvent(pause ? "smash" : "repair");
}

LightWidget::LightWidget(const QString &image, QWidget *parent)
    : QWidget(parent)
    , m_image(image)
{}

bool LightWidget::isOn() const
{ return m_on; }

void LightWidget::setOn(bool on)
{
    if (on == m_on)
        return;
    m_on = on;
    update();
}

void LightWidget::switchLight(bool onoff)
{
    setOn(onoff);
}

void LightWidget::paintEvent(QPaintEvent *)
{
    if (!m_on)
        return;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawImage(0, 0, m_image);
}

QSize LightWidget::sizeHint() const
{
    return m_image.size();
}

ButtonWidget::ButtonWidget(QWidget *parent) :
    QAbstractButton(parent), m_playIcon(":/play.png"_L1),
    m_pauseIcon(":/pause.png"_L1)
{
    setCheckable(true);
}

void ButtonWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawImage(0, 0, isChecked() ? m_playIcon : m_pauseIcon);
}

QSize ButtonWidget::sizeHint() const
{
    return isChecked() ? m_playIcon.size() : m_pauseIcon.size();
}
