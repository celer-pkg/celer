// Copyright (C) 2024 Stan Morris.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SPIRALITEM_H
#define SPIRALITEM_H

#include <QQuickItem>
#include <QElapsedTimer>

class QSGGeometry;
class QSGGeometryNode;

class SpiralItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(int vertexCount READ vertexCount WRITE setVertexCount NOTIFY vertexCountChanged FINAL)
    Q_PROPERTY(int maxVertices READ maxVertices CONSTANT FINAL)
    Q_PROPERTY(int fps READ fps WRITE setFps NOTIFY fpsChanged FINAL)
    Q_PROPERTY(QString mode READ mode WRITE setMode NOTIFY modeChanged FINAL)
    Q_PROPERTY(bool resizeByVertexCount READ resizeByVertexCount NOTIFY resizeByVertexCountChanged FINAL)
    QML_ELEMENT

public:
    SpiralItem();

signals:
    void vertexCountChanged();
    void fpsChanged();
    void cpuChanged();
    void modeChanged();
    void resizeByVertexCountChanged();

public slots:
    void toggleMode();
    void handleResizeByVertexCountChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);

private:
    void updateGeometry(QSGGeometry *&geometry);
    void setupGeometryNode(QSGGeometryNode *);
    QSGGeometry *createGeometry();
    void populateVertices(QSGGeometry *, const int count);
    void populateColors(QSGGeometry *, const int count);
    void reallocateVertices(QSGGeometry* geometry, const int count);
    void logStatistics();
    int vertexCount() const;
    void setVertexCount(int newVertexCount);
    int maxVertices() const;
    int fps() const;
    void setFps(int newFps);
    QString mode() const;
    void setMode(const QString &newMode);
    bool resizeByVertexCount() const;
    void setResizeByVertexCount(bool newResizeByVertexCount);

    QElapsedTimer m_statisticsTimer;
    QString m_mode;
    const int m_maxVertices {60000};
    int m_vertexCount {0};
    int m_fps = {};
    bool m_resizeByVertexCount = {true};
    bool m_animating = {false};
    bool m_resetVertices = {false};
    int m_maxCount {-1};
    int m_minCount {m_maxVertices};
};

#endif // SPIRALITEM_H
