// Copyright (C) 2024 Stan Morris.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


#ifndef INDEXEDSPIRALITEM_H
#define INDEXEDSPIRALITEM_H

#include <QQuickItem>

class QSGGeometry;
class QSGGeometryNode;

class IndexedSpiralItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(int indexCount READ indexCount WRITE setIndexCount NOTIFY indexCountChanged FINAL)
    Q_PROPERTY(int maxIndices READ maxIndices CONSTANT FINAL)
    QML_ELEMENT
public:
    IndexedSpiralItem();

signals:
    void indexCountChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);

private:
    bool applyIndexChange(QSGGeometryNode *);
    void configureGeometryNode(QSGGeometryNode *);
    QSGGeometry *createGeometry() const;
    void configureGeometry(QSGGeometry *) const;
    void configureColors(QSGGeometry *) const;
    int indexCount() const;
    void setIndexCount(int newIndexCount);
    int maxIndices() const;

    const int m_maxVertices {16000};
    const int m_maxIndices {m_maxVertices};
    int m_indexCount {m_maxVertices};
    bool m_growFromCenter {true};
};

#endif // INDEXEDSPIRALITEM_H
