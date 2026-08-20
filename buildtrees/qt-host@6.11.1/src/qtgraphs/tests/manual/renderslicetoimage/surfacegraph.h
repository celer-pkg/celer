// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SURFACEGRAPH_H
#define SURFACEGRAPH_H

#include <QtCore/qobject.h>
#include <QtQuick/qquickitemgrabresult.h>

class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class SurfaceGraphModifier;
class SurfaceGraphWidget;

class SurfaceGraph : public QObject
{
    Q_OBJECT
public:
    SurfaceGraph(QWidget *parent = nullptr);
    ~SurfaceGraph();

    void initialize();
    QWidget *surfaceWidget() { return m_surfaceWidget; }

private:
    void renderSliceToImage();
    void applySliceImage(QImage image);
    void changeSelectionMode(bool checked);

    SurfaceGraphModifier *m_modifier = nullptr;
    SurfaceGraphWidget *m_surfaceGraphWidget = nullptr;
    QWidget *m_surfaceWidget = nullptr;

    QCheckBox *m_pickCheckBox = nullptr;
    QCheckBox *m_invertCheckBox = nullptr;
    QRadioButton *m_rowRadioButton = nullptr;
    QLineEdit *m_lineSelectText = nullptr;
    QLabel *m_sliceResultLabel = nullptr;
    QSharedPointer<QQuickItemGrabResult> m_grab;
};

#endif
