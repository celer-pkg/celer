// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef BARGRAPH_H
#define BARGRAPH_H

#include <QtCore/qobject.h>
#include <QtQuick/qquickitemgrabresult.h>

class BarGraphModifier;
class BarGraphWidget;
class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;

class BarGraph : public QObject
{
    Q_OBJECT
public:
    BarGraph(QWidget *parent = nullptr);
    ~BarGraph();

    void initialize();
    QWidget *barWidget() { return m_barWidget; }

private:
    void renderSliceToImage();
    void changeSelectionMode(bool checked);

    BarGraphModifier *m_modifier = nullptr;
    BarGraphWidget *m_barGraphWidget = nullptr;
    QWidget *m_barWidget = nullptr;

    QCheckBox *m_pickCheckBox = nullptr;
    QCheckBox *m_invertCheckBox = nullptr;
    QRadioButton *m_rowRadioButton = nullptr;
    QLineEdit *m_lineSelectText = nullptr;
    QLabel *m_sliceResultLabel = nullptr;
    QSharedPointer<QQuickItemGrabResult> m_grab;
};

#endif
