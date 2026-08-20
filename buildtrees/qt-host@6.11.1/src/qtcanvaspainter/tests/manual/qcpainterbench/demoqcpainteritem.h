// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef DEMOQCPAINTERITEM_H
#define DEMOQCPAINTERITEM_H

#include "qcanvaspainteritem.h"

class QCanvasPainterItemRenderer;

class DemoQCPainterItem : public QCanvasPainterItem
{
    Q_OBJECT
    Q_PROPERTY(float animationTime READ animationTime WRITE setAnimationTime NOTIFY animationTimeChanged)
    Q_PROPERTY(int enabledTests READ enabledTests WRITE setEnabledTests NOTIFY enabledTestsChanged)
    Q_PROPERTY(int testCount READ testCount WRITE setTestCount NOTIFY testCountChanged)
    Q_PROPERTY(bool highQualityRendering READ highQualityRendering WRITE setHighQualityRendering NOTIFY highQualityRenderingChanged)
    QML_ELEMENT

public:
    DemoQCPainterItem(QQuickItem *parent = nullptr);

    // Reimplement
    QCanvasPainterItemRenderer *createItemRenderer() const;

    float animationTime() const { return m_animationTime; }
    int enabledTests() const { return m_enabledTests; }
    int testCount() const { return m_testCount; }
    bool highQualityRendering() const { return m_highQualityRendering; }

    void setAnimationTime(float value) {
        if (qFuzzyCompare(value, m_animationTime))
            return;
        m_animationTime = value;
        Q_EMIT animationTimeChanged();
        update();
    }

    void setEnabledTests(int value) {
        if (value == m_enabledTests)
            return;
        m_enabledTests = value;
        Q_EMIT enabledTestsChanged();
        update();
    }

    void setTestCount(int value) {
        if (value == m_testCount)
            return;
        m_testCount = value;
        Q_EMIT testCountChanged();
        update();
    }

    void setHighQualityRendering(bool value) {
        if (value == m_highQualityRendering)
            return;
        m_highQualityRendering = value;
        Q_EMIT highQualityRenderingChanged();
        update();
    }

Q_SIGNALS:
    void animationTimeChanged();
    void enabledTestsChanged();
    void testCountChanged();
    void highQualityRenderingChanged();

private:
    float m_animationTime = 0.0f;
    int m_enabledTests = 0;
    int m_testCount = 1;
    bool m_highQualityRendering = false;
};

#endif // DEMOQCPAINTERITEM_H
