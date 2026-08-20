// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef DATA_H
#define DATA_H

#include <QtGraphsWidgets/Q3DScatterWidgetItem>
#include <QtGraphsWidgets/Q3DBarsWidgetItem>
#include <QtGraphsWidgets/Q3DSurfaceWidgetItem>
#include <QtGraphs/QScatterDataProxy>
#include <QtGraphs/QBarDataProxy>
#include <QtGraphs/QHeightMapSurfaceDataProxy>
#include <QTextEdit>

class Data : public QObject
{
    Q_OBJECT

public:
    explicit Data(Q3DSurfaceWidgetItem *surface, Q3DScatterWidgetItem *scatter, Q3DBarsWidgetItem *bars,
                  QTextEdit *statusLabel, QWidget *widget);
    ~Data();

    void start();
    void stop();

    void updateData();
    void clearData();

    void scrollDown();
    void setData(const QImage &image);
    void useGradientOne();
    void useGradientTwo();

public:
    enum GraphsMode {
        Surface = 0,
        Scatter,
        Bars
    };

public Q_SLOTS:
    void setResolution(int selection);
    void changeMode(int mode);

private:
    Q3DSurfaceWidgetItem *m_surface;
    Q3DScatterWidgetItem *m_scatter;
    Q3DBarsWidgetItem *m_bars;
    QTextEdit *m_statusArea;
    QWidget *m_widget;
    bool m_resize;
    QSize m_resolution;
    int m_resolutionLevel;
    GraphsMode m_mode;
    QScatterDataArray m_scatterDataArray;
    QBarDataArray m_barDataArray;
    bool m_started;
};

class ContainerChanger : public QObject
{
    Q_OBJECT

public:
    explicit ContainerChanger(QWidget *surface, QWidget *scatter, QWidget *bars,
                              QWidget *buttonOne, QWidget *buttonTwo);
    ~ContainerChanger();

public Q_SLOTS:
    void changeContainer(int container);

private:
    QWidget *m_surface;
    QWidget *m_scatter;
    QWidget *m_bars;
    QWidget *m_button1;
    QWidget *m_button2;
};

#endif
