// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QPIESLICE_H
#define QTGRAPHS_QPIESLICE_H

#include <QtCore/qobject.h>
#include <QtGraphs/qgraphsglobal.h>
#include <QtQml/qqmlengine.h>

QT_BEGIN_NAMESPACE

class PieRenderer;
class QPieSlicePrivate;
class QPieSeries;

class Q_GRAPHS_EXPORT QPieSlice : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged FINAL)
    Q_PROPERTY(bool labelVisible READ isLabelVisible WRITE setLabelVisible NOTIFY
                   labelVisibleChanged FINAL)
    Q_PROPERTY(LabelPosition labelPosition READ labelPosition WRITE setLabelPosition NOTIFY
                   labelPositionChanged FINAL)
    Q_PROPERTY(QColor labelColor READ labelColor WRITE setLabelColor NOTIFY labelColorChanged FINAL)
    Q_PROPERTY(QFont labelFont READ labelFont WRITE setLabelFont NOTIFY labelFontChanged FINAL)
    Q_PROPERTY(qreal labelArmLengthFactor READ labelArmLengthFactor WRITE setLabelArmLengthFactor
                   NOTIFY labelArmLengthFactorChanged FINAL)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(
        QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged FINAL)
    Q_PROPERTY(
        qreal borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged FINAL)
    Q_PROPERTY(qreal value READ value WRITE setValue NOTIFY valueChanged FINAL)
    Q_PROPERTY(bool exploded READ isExploded WRITE setExploded NOTIFY explodedChanged FINAL)
    Q_PROPERTY(qreal explodeDistanceFactor READ explodeDistanceFactor WRITE setExplodeDistanceFactor
                   NOTIFY explodeDistanceFactorChanged FINAL)
    Q_PROPERTY(qreal percentage READ percentage NOTIFY percentageChanged FINAL)
    Q_PROPERTY(qreal startAngle READ startAngle NOTIFY startAngleChanged FINAL)
    Q_PROPERTY(qreal angleSpan READ angleSpan NOTIFY angleSpanChanged FINAL)
    Q_PROPERTY(qsizetype subSlicesCount READ subSlicesCount NOTIFY subSlicesCountChanged
                    REVISION(6, 11))
    Q_PROPERTY(qreal subSlicesSum READ subSlicesSum NOTIFY subSlicesSumChanged REVISION(6, 11))
    Q_PROPERTY(qreal subSlicesRatio READ subSlicesRatio WRITE setSubSlicesRatio
                    NOTIFY subSlicesRatioChanged REVISION(6, 11))
    Q_PROPERTY(QQmlListProperty<QPieSlice> sliceChildren READ sliceChildren REVISION(6, 11))
    Q_CLASSINFO("DefaultProperty", "sliceChildren")
    QML_NAMED_ELEMENT(PieSlice)

public:
    enum class LabelPosition {
        Outside,
        InsideHorizontal,
        InsideTangential,
        InsideNormal,
    };
    Q_ENUM(LabelPosition)

    explicit QPieSlice(QObject *parent = nullptr);
    QPieSlice(const QString &label, qreal value, QObject *parent = nullptr);
    ~QPieSlice() override;

    QQmlListProperty<QPieSlice> sliceChildren();

    void setLabel(const QString &label);
    QString label() const;
    void setLabelVisible(bool visible = true);
    bool isLabelVisible() const;
    void setLabelPosition(LabelPosition position);
    LabelPosition labelPosition();
    void setLabelColor(QColor color);
    QColor labelColor() const;
    void setLabelFont(const QFont &font);
    QFont labelFont() const;
    void setLabelArmLengthFactor(qreal factor);
    qreal labelArmLengthFactor() const;

    void setColor(QColor color);
    QColor color() const;

    void setBorderColor(QColor color);
    QColor borderColor() const;
    void setBorderWidth(qreal borderWidth);
    qreal borderWidth() const;

    void setValue(qreal value);
    qreal value() const;

    void setExploded(bool exploded);
    bool isExploded() const;
    void setExplodeDistanceFactor(qreal factor);
    qreal explodeDistanceFactor() const;

    qreal percentage() const;
    qreal startAngle() const;
    qreal angleSpan() const;

    QPieSeries *series() const;

    Q_REVISION(6, 11) Q_INVOKABLE bool append(QPieSlice *slice);
    Q_REVISION(6, 11) Q_INVOKABLE bool append(const QList<QPieSlice *> &slices);
    Q_REVISION(6, 11) Q_INVOKABLE bool insert(qsizetype index, QPieSlice *slice);
    Q_REVISION(6, 11) Q_INVOKABLE bool remove(QPieSlice *slice);
    Q_REVISION(6, 11) Q_SLOT void clear();
    Q_REVISION(6, 11) Q_INVOKABLE QPieSlice *append(const QString &label, qreal value);
    Q_REVISION(6, 11) Q_INVOKABLE QPieSlice *at(qsizetype index) const;
    Q_REVISION(6, 11) Q_INVOKABLE QPieSlice *find(const QString &label) const;
    Q_REVISION(6, 11) Q_INVOKABLE bool replace(qsizetype index, QPieSlice *slice);
    Q_REVISION(6, 11) Q_INVOKABLE void removeMultiple(qsizetype index, int count);
    Q_REVISION(6, 11) Q_INVOKABLE bool remove(qsizetype index);
    Q_REVISION(6, 11) Q_INVOKABLE bool replace(QPieSlice *oldSlice, QPieSlice *newSlice);
    Q_REVISION(6, 11) Q_INVOKABLE bool replaceAll(const QList<QPieSlice *> &slices);
    Q_REVISION(6, 11) Q_INVOKABLE bool take(QPieSlice *slice);

    QList<QPieSlice *> subSlices() const;

    qsizetype subSlicesCount() const;
    bool isEmpty() const;

    qreal subSlicesSum() const;

    void setSubSlicesRatio(qreal subSlicesRatio);
    qreal subSlicesRatio() const;

Q_SIGNALS:
    void labelChanged();
    void labelVisibleChanged();
    void labelFontChanged();
    void labelColorChanged();
    void valueChanged();
    void explodedChanged();
    void explodeDistanceFactorChanged();
    void percentageChanged();
    void startAngleChanged();
    void angleSpanChanged();
    void sliceChanged();
    void labelPositionChanged();
    void labelArmLengthFactorChanged();
    void colorChanged();
    void borderColorChanged();
    void borderWidthChanged();
    Q_REVISION(6, 11) void subSlicesAdded(const QList<QPieSlice *> &slices);
    Q_REVISION(6, 11) void subSlicesRemoved(const QList<QPieSlice *> &slices);
    Q_REVISION(6, 11) void subSlicesReplaced(const QList<QPieSlice *> &slices);
    Q_REVISION(6, 11) void subSlicesCountChanged(qsizetype count);
    Q_REVISION(6, 11) void subSlicesSumChanged(qreal sum);
    Q_REVISION(6, 11) void subSlicesRatioChanged(qreal newSubSlicesRatio);

private:
    friend class PieRenderer;
    friend class QPieSeries;
    friend class QPieSeriesPrivate;

    Q_DECLARE_PRIVATE(QPieSlice)
    Q_DISABLE_COPY(QPieSlice)
};

QT_END_NAMESPACE

#endif // QTGRAPHS_QPIESLICE_H
