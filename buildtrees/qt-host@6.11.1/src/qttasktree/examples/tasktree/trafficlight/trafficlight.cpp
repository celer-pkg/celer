// Copyright (C) 2025 Jarek Kobus
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "trafficlight.h"

#include "glueinterface.h"

#include <QAbstractButton>
#include <QBoxLayout>
#include <QPainter>

class LightWidget final : public QWidget
{
public:
    LightWidget(const QString &image, QWidget *parent = nullptr)
        : QWidget(parent)
        , m_image(image)
    {}

    void setOn(bool on)
    {
        if (on == m_on)
            return;
        m_on = on;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        if (!m_on)
            return;
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawImage(QPointF{}, m_image);
    }
    QSize sizeHint() const override { return m_image.size(); }

private:
    QImage m_image;
    bool m_on = false;
};

class ButtonWidget final : public QAbstractButton
{
public:
    ButtonWidget(QWidget *parent = nullptr)
        : QAbstractButton(parent), m_playIcon(":/play.png")
        , m_pauseIcon(":/pause.png")
    {
        setCheckable(true);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawImage(QPointF{}, currentIcon());
    }
    QSize sizeHint() const override { return isChecked() ? m_playIcon.size() : m_pauseIcon.size(); }

private:
    const QImage &currentIcon() const { return isChecked() ? m_playIcon : m_pauseIcon; }

    QImage m_playIcon;
    QImage m_pauseIcon;
};

static constexpr int s_marginUnit = 20;

class TrafficLightWidget final : public QWidget
{
public:
    TrafficLightWidget(QWidget *parent = nullptr)
        : QWidget(parent)
        , m_background(":/background.png")
    {
        QVBoxLayout *vbox = new QVBoxLayout(this);
        vbox->setContentsMargins(0, 2 * s_marginUnit, 0, 4 * s_marginUnit);
        m_red = new LightWidget(":/red.png");
        vbox->addWidget(m_red, 0, Qt::AlignHCenter);
        m_yellow = new LightWidget(":/yellow.png");
        vbox->addWidget(m_yellow, 0, Qt::AlignHCenter);
        m_green = new LightWidget(":/green.png");
        vbox->addWidget(m_green, 0, Qt::AlignHCenter);
        setLayout(vbox);
    }

    LightWidget *redLight() const { return m_red; }
    LightWidget *yellowLight() const { return m_yellow; }
    LightWidget *greenLight() const { return m_green; }

    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawImage(QPointF{}, m_background);
    }

    QSize sizeHint() const override { return m_background.size(); }

private:
    QImage m_background;
    LightWidget *m_red;
    LightWidget *m_yellow;
    LightWidget *m_green;
};

TrafficLight::TrafficLight(GlueInterface &iface)
{
    setWindowTitle(tr("Traffic Light (QtTaskTree)"));
    TrafficLightWidget *widget = new TrafficLightWidget(this);
    setFixedSize(widget->sizeHint());

    QAbstractButton *button = new ButtonWidget(this);
    auto setButtonGeometry = [this, button] {
        const QSize buttonSize = button->sizeHint();
        button->setGeometry(width() - buttonSize.width() - s_marginUnit,
                            height() - buttonSize.height() - s_marginUnit,
                            buttonSize.width(), buttonSize.height());
    };
    connect(button, &QAbstractButton::toggled, this, setButtonGeometry);
    setButtonGeometry();

    connect(&iface, &GlueInterface::lightsChanged, this, [widget](Lights lights) {
        widget->redLight()->setOn(lights & Light::Red);
        widget->yellowLight()->setOn(lights & Light::Yellow);
        widget->greenLight()->setOn(lights & Light::Green);
    });

    connect(button, &QAbstractButton::toggled, this, [iface = &iface](bool pause) {
        pause ? iface->smash() : iface->repair();
    });
}
