// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CUSTOMLINESERIES_H
#define CUSTOMLINESERIES_H

#include <QList>
#include <QQmlEngine>
#include <QTimer>
#include <QtGraphs/qlineseries.h>

class CustomLineSeries : public QLineSeries
{
    Q_OBJECT
    Q_PROPERTY(qreal amplitude READ amplitude WRITE setAmplitude NOTIFY amplitudeChanged)
    Q_PROPERTY(qreal frequency READ frequency WRITE setFrequency NOTIFY frequencyChanged)
    Q_PROPERTY(qreal phase READ phase WRITE setPhase NOTIFY phaseChanged)
    QML_ELEMENT
public:
    CustomLineSeries(QLineSeries *parent = nullptr);

    qreal frequency() const;
    void setFrequency(qreal newFrequency);

    qreal amplitude() const;
    void setAmplitude(qreal newAmplitude);

    qreal phase() const;
    void setPhase(qreal newPhase);

public Q_SLOTS:
    void updateData();

signals:
    void frequencyChanged();

    void amplitudeChanged();

    void phaseChanged();

private:
    QTimer m_updateTimer;
    qreal m_frequency = 10.0;
    qreal m_amplitude = 1.0;
    qreal m_phase = 0.0;
    int m_scanIndex = 0;
    int m_scanCount = 5;
};

#endif // CUSTOMLINESERIES_H
