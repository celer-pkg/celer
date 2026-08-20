// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QVALUE3DAXIS_H
#define QTGRAPHS_QVALUE3DAXIS_H

#include <QtGraphs/qabstract3daxis.h>
#include <QtGraphs/qvalue3daxisformatter.h>

QT_BEGIN_NAMESPACE

class QValue3DAxisPrivate;

class Q_GRAPHS_EXPORT QValue3DAxis : public QAbstract3DAxis
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QValue3DAxis)
    Q_PROPERTY(qsizetype segmentCount READ segmentCount WRITE setSegmentCount NOTIFY
                   segmentCountChanged FINAL)
    Q_PROPERTY(qsizetype subSegmentCount READ subSegmentCount WRITE setSubSegmentCount NOTIFY
                   subSegmentCountChanged FINAL)
    Q_PROPERTY(
        QString labelFormat READ labelFormat WRITE setLabelFormat NOTIFY labelFormatChanged FINAL)
    Q_PROPERTY(QValue3DAxisFormatter *formatter READ formatter WRITE setFormatter NOTIFY
                   formatterChanged FINAL)
    Q_PROPERTY(bool reversed READ reversed WRITE setReversed NOTIFY reversedChanged FINAL)
    QML_NAMED_ELEMENT(Value3DAxis)

public:
    explicit QValue3DAxis(QObject *parent = nullptr);
    ~QValue3DAxis() override;

    void setSegmentCount(qsizetype count);
    qsizetype segmentCount() const;

    void setSubSegmentCount(qsizetype count);
    qsizetype subSegmentCount() const;

    void setLabelFormat(const QString &format);
    QString labelFormat() const;

    void setFormatter(QValue3DAxisFormatter *formatter);
    QValue3DAxisFormatter *formatter() const;

    void setReversed(bool enable);
    bool reversed() const;

    void recalculate();
    qsizetype gridSize();
    qsizetype subGridSize();
    float gridPositionAt(qsizetype gridLine);
    float subGridPositionAt(qsizetype gridLine);
    float labelPositionAt(qsizetype index);
    float positionAt(float x);
    QString stringForValue(float x);

Q_SIGNALS:
    void segmentCountChanged(qsizetype count);
    void subSegmentCountChanged(qsizetype count);
    void labelFormatChanged(const QString &format);
    void formatterChanged(QValue3DAxisFormatter *formatter);
    void reversedChanged(bool enable);
    void formatterDirty();

private:
    Q_DISABLE_COPY(QValue3DAxis)
    friend class QQuickGraphsItem;
    friend class QQuickGraphsBars;
    friend class QQuickGraphsScatter;
    friend class QQuickGraphsSurface;
    friend class QValue3DAxisFormatterPrivate;
};

QT_END_NAMESPACE

#endif
