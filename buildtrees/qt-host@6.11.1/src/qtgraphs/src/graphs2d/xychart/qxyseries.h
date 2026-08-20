// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QXYSERIES_H
#define QTGRAPHS_QXYSERIES_H

#include <QtGraphs/qabstractseries.h>
#include <QtGraphs/qgraphsglobal.h>

QT_BEGIN_NAMESPACE
class QModelIndex;
class QXYSeriesPrivate;
class QXYModelMapper;

class Q_GRAPHS_EXPORT QXYSeries : public QAbstractSeries
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(QColor selectedColor READ selectedColor WRITE setSelectedColor NOTIFY
                   selectedColorChanged FINAL)
    Q_PROPERTY(QQmlComponent *pointDelegate READ pointDelegate WRITE setPointDelegate NOTIFY pointDelegateChanged FINAL)
    Q_PROPERTY(bool draggable READ isDraggable WRITE setDraggable NOTIFY draggableChanged FINAL)
    Q_PROPERTY(QList<qsizetype> selectedPoints READ selectedPoints NOTIFY selectedPointsChanged FINAL)
    Q_PROPERTY(qsizetype count READ count NOTIFY countChanged FINAL)

protected:
    explicit QXYSeries(QXYSeriesPrivate &dd, QObject *parent = nullptr);

public:
    Q_INVOKABLE void append(qreal x, qreal y);
    Q_INVOKABLE void append(QPointF point);
    Q_INVOKABLE void append(const QList<QPointF> &points);
    Q_INVOKABLE void replace(qreal oldX, qreal oldY, qreal newX, qreal newY);
    Q_INVOKABLE void replace(QPointF oldPoint, QPointF newPoint);
    Q_INVOKABLE void replace(qsizetype index, qreal newX, qreal newY);
    Q_INVOKABLE void replace(qsizetype index, QPointF newPoint);
    Q_INVOKABLE void replace(const QList<QPointF> &points);
    Q_INVOKABLE void remove(qreal x, qreal y);
    Q_INVOKABLE void remove(QPointF point);
    Q_INVOKABLE void remove(qsizetype index);
    Q_INVOKABLE void insert(qsizetype index, QPointF point);
    Q_INVOKABLE void clear();
    Q_INVOKABLE QPointF at(qsizetype index) const;
    Q_INVOKABLE qsizetype find(QPointF point) const;
    Q_INVOKABLE void removeMultiple(qsizetype index, qsizetype count);
    Q_INVOKABLE bool take(QPointF point);

    ~QXYSeries() override;

    QList<QPointF> points() const;

    QXYSeries &operator<<(QPointF point);
    QXYSeries &operator<<(const QList<QPointF> &points);

    void setColor(QColor newColor);
    QColor color() const;

    void setSelectedColor(QColor color);
    QColor selectedColor() const;

    qsizetype count() const;

    Q_INVOKABLE bool isPointSelected(qsizetype index) const;
    Q_INVOKABLE void selectPoint(qsizetype index);
    Q_INVOKABLE void deselectPoint(qsizetype index);
    Q_INVOKABLE void setPointSelected(qsizetype index, bool selected);
    Q_INVOKABLE void selectAllPoints();
    Q_INVOKABLE void deselectAllPoints();
    Q_INVOKABLE void selectPoints(const QList<qsizetype> &indexes);
    Q_INVOKABLE void deselectPoints(const QList<qsizetype> &indexes);
    Q_INVOKABLE void toggleSelection(const QList<qsizetype> &indexes);
    QList<qsizetype> selectedPoints() const;

    QQmlComponent *pointDelegate() const;
    void setPointDelegate(QQmlComponent *newPointDelegate);

    QML_ELEMENT
    QML_UNCREATABLE("XYSeries is an abstract base class.")

    bool isDraggable() const;
    void setDraggable(bool newDraggable);

Q_SIGNALS:
    void pointReplaced(qsizetype index);
    void pointRemoved(qsizetype index);
    void pointAdded(qsizetype index);
    Q_REVISION(6, 9) void pointsAdded(qsizetype start, qsizetype end);
    void colorChanged(QColor color);
    void selectedColorChanged(QColor color);
    void pointsReplaced();
    void pointsRemoved(qsizetype index, qsizetype count);
    void selectedPointsChanged();
    void pointDelegateChanged();
    void draggableChanged();
    void seriesUpdated();
    void countChanged();

    Q_REVISION(6, 9) void clicked(QPoint point);
    Q_REVISION(6, 9) void doubleClicked(QPoint point);
    Q_REVISION(6, 9) void pressed(QPoint point);
    Q_REVISION(6, 9) void released(QPoint point);

private:
    friend class PointRenderer;
    friend class QGraphPointAnimation;
    friend class QGraphTransition;
    Q_DECLARE_PRIVATE(QXYSeries)
    Q_DISABLE_COPY(QXYSeries)
};

QT_END_NAMESPACE

#endif // QTGRAPHS_QXYSERIES_H
