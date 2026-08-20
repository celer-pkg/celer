// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#ifndef SLIDERWRAPPER_H
#define SLIDERWRAPPER_H
#include <QObject>
#include <QSlider>

class SliderWrapper : public QObject
{
    Q_OBJECT

public:
    explicit SliderWrapper(QSlider *slider);

public Q_SLOTS:
    void setEnabled(int enabled);

private:
    QSlider *m_slider = nullptr;

};

#endif // SLIDERWRAPPER_H
