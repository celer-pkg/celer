// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QBARSET_H
#define QTGRAPHS_QBARSET_H

#include <QtCore/qobject.h>
#include <QtGraphs/qgraphsglobal.h>
#include <QtGui/qbrush.h>
#include <QtGui/qfont.h>
#include <QtGui/qpen.h>
#include <QtQml/qqmlengine.h>

QT_BEGIN_NAMESPACE

class QBarSetPrivate;

class Q_GRAPHS_EXPORT QBarSet : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged FINAL)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(QColor selectedColor READ selectedColor WRITE setSelectedColor NOTIFY
                   selectedColorChanged FINAL)
    Q_PROPERTY(
        QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged FINAL)
    Q_PROPERTY(QColor labelColor READ labelColor WRITE setLabelColor NOTIFY labelColorChanged FINAL)
    Q_PROPERTY(QVariantList values READ values WRITE setValues NOTIFY valuesChanged FINAL)
    Q_PROPERTY(
        qreal borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged FINAL)
    Q_PROPERTY(qsizetype count READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(QList<qsizetype> selectedBars READ selectedBars NOTIFY selectedBarsChanged FINAL)
    QML_NAMED_ELEMENT(BarSet)
    Q_DECLARE_PRIVATE(QBarSet)

public:
    explicit QBarSet(QObject *parent = nullptr);
    explicit QBarSet(const QString &label, QObject *parent = nullptr);
    ~QBarSet() override;

    void setLabel(const QString &label);
    QString label() const;

    Q_INVOKABLE void append(qreal value);
    Q_INVOKABLE void append(const QList<qreal> &values);
    Q_INVOKABLE void insert(qsizetype index, qreal value);
    Q_INVOKABLE void remove(qsizetype index, qsizetype count = 1);
    Q_INVOKABLE void replace(qsizetype index, qreal value);
    Q_INVOKABLE qreal at(qsizetype index) const;
    Q_INVOKABLE qsizetype count() const;
    Q_INVOKABLE qreal sum() const;
    Q_INVOKABLE void clear();

    qreal operator [](qsizetype index) const;
    QBarSet &operator << (qreal value);

    QColor color() const;
    void setColor(QColor color);

    QColor borderColor() const;
    void setBorderColor(QColor color);

    QColor labelColor() const;
    void setLabelColor(QColor color);

    QColor selectedColor() const;
    void setSelectedColor(QColor color);

    QVariantList values() const;
    void setValues(const QVariantList &values);
    qreal borderWidth() const;
    void setBorderWidth(qreal borderWidth);

    Q_INVOKABLE bool isBarSelected(qsizetype index) const;
    Q_INVOKABLE void selectBar(qsizetype index);
    Q_INVOKABLE void deselectBar(qsizetype index);
    Q_INVOKABLE void setBarSelected(qsizetype index, bool selected);
    Q_INVOKABLE void selectAllBars();
    Q_INVOKABLE void deselectAllBars();
    Q_INVOKABLE void selectBars(const QList<qsizetype> &indexes);
    Q_INVOKABLE void deselectBars(const QList<qsizetype> &indexes);
    Q_INVOKABLE void toggleSelection(const QList<qsizetype> &indexes);
    QList<qsizetype> selectedBars() const;

Q_SIGNALS:
    void update();
    void labelChanged();
    void colorChanged(QColor color);
    void borderColorChanged(QColor color);
    void labelColorChanged(QColor color);
    void valuesChanged();
    void selectedColorChanged(QColor color);
    void countChanged();

    void borderWidthChanged(qreal width);

    void valuesAdded(qsizetype index, qsizetype count);
    void valuesRemoved(qsizetype index, qsizetype count);
    void valueChanged(qsizetype index);

    void updatedBars();
    void valueAdded(qsizetype index, qsizetype count);
    void valueRemoved(qsizetype index, qsizetype count);

    void selectedBarsChanged(const QList<qsizetype> &indexes);

private:
    Q_DISABLE_COPY(QBarSet)
    friend class QBarSeries;
    friend class QBarSeriesPrivate;
};

QT_END_NAMESPACE

#endif // QTGRAPHS_QBARSET_H
