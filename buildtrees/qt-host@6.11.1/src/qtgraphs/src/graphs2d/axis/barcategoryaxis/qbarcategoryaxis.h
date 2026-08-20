// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QBARCATEGORYAXIS_H
#define QTGRAPHS_QBARCATEGORYAXIS_H

#include <QtGraphs/qabstractaxis.h>
#include <QtGraphs/qgraphsglobal.h>
#include <QtQml/qqmlengine.h>

QT_BEGIN_NAMESPACE

class QBarCategoryAxisPrivate;

class Q_GRAPHS_EXPORT QBarCategoryAxis : public QAbstractAxis
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    Q_PROPERTY(
        QStringList categories READ categories WRITE setCategories NOTIFY categoriesChanged FINAL)
    Q_PROPERTY(QString min READ min WRITE setMin NOTIFY minChanged FINAL)
    Q_PROPERTY(QString max READ max WRITE setMax NOTIFY maxChanged FINAL)
    Q_PROPERTY(qsizetype count READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(LabelPosition labelPosition READ labelPosition WRITE setLabelPosition NOTIFY
                   labelPositionChanged REVISION(6, 11))
    QML_NAMED_ELEMENT(BarCategoryAxis)

public:
    explicit QBarCategoryAxis(QObject *parent = nullptr);
    ~QBarCategoryAxis() override;

protected:
    QBarCategoryAxis(QBarCategoryAxisPrivate &dd, QObject *parent = nullptr);

public:
    enum class LabelPosition {
        Center,
        OnValue,
    };
    Q_ENUM(LabelPosition)

    AxisType type() const override;
    Q_INVOKABLE void append(const QStringList &categories);
    Q_INVOKABLE void append(const QString &category);
    Q_INVOKABLE void remove(const QString &category);
    Q_INVOKABLE void remove(qsizetype index);
    Q_INVOKABLE void insert(qsizetype index, const QString &category);
    Q_INVOKABLE void replace(const QString &oldCategory, const QString &newCategory);
    Q_INVOKABLE void clear();
    Q_INVOKABLE QString at(qsizetype index) const;
    void setCategories(const QStringList &categories);
    QStringList categories();
    qsizetype count() const;
    void setLabelPosition(LabelPosition position);
    LabelPosition labelPosition() const;

    //range handling
    void setMin(const QString &minCategory);
    QString min() const;
    void setMax(const QString &maxCategory);
    QString max() const;
    void setRange(const QString &minCategory, const QString &maxCategory);

Q_SIGNALS:
    void categoriesChanged();
    void minChanged(const QString &min);
    void maxChanged(const QString &max);
    void categoryRangeChanged(const QString &min, const QString &max);
    void countChanged();
    Q_REVISION(6, 11) void labelPositionChanged(LabelPosition position);

private:
    Q_DECLARE_PRIVATE(QBarCategoryAxis)
    Q_DISABLE_COPY(QBarCategoryAxis)
};

QT_END_NAMESPACE

#endif // QTGRAPHS_QBARCATEGORYAXIS_H
