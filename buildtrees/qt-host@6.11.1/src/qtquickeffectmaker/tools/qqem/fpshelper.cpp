// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "fpshelper.h"

FpsHelper::FpsHelper()
{
    setFlag(QQuickItem::ItemHasContents);

    connect(this, &QQuickItem::enabledChanged, this, [this]() {
        if (isEnabled()) {
            m_frames = 0;
            m_timer.start();
            update();
        }
    });

    if (isEnabled())
        m_timer.start();
}

float FpsHelper::fps() const
{
    return m_fps;
}

QSGNode *FpsHelper::updatePaintNode(QSGNode *node, UpdatePaintNodeData *)
{
    m_frames++;
    qint64 nsElapsed = m_timer.nsecsElapsed();
    float ms = float(nsElapsed / 1000000.0);
    if (ms >= 1000.0f) {
        float fps = m_frames / (ms / 1000.0f);
        // Round to 0.2 accuracy
        fps = std::round(fps * 5.0f) / 5.0f;
        if (!qFuzzyCompare(fps, m_fps)) {
            m_fps = fps;
            Q_EMIT fpsChanged();
        }
        m_frames = 0;
        m_timer.restart();
    }

    // Call update in a loop if fps tracking is enabled
    if (isEnabled())
        update();

    return node;
}
