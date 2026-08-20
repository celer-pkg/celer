// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef FPSHELPER_H
#define FPSHELPER_H

#include <QQuickItem>
#include <QElapsedTimer>

class FpsHelper : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(float fps READ fps NOTIFY fpsChanged)

public:
    FpsHelper();

    float fps() const;

protected:
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *) override;

signals:
    void fpsChanged();

private:
    float m_fps = 0.0f;
    int m_frames = 0;
    QElapsedTimer m_timer;
};

#endif // FPSHELPER_H
