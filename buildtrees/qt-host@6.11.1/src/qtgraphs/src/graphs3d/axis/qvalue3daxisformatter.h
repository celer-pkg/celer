// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QVALUE3DAXISFORMATTER_H
#define QTGRAPHS_QVALUE3DAXISFORMATTER_H

#include <QtCore/qlist.h>
#include <QtCore/qlocale.h>
#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qstringlist.h>
#include <QtGraphs/qgraphsglobal.h>
#include <QtQmlIntegration/qqmlintegration.h>

QT_BEGIN_NAMESPACE

class QValue3DAxisFormatterPrivate;
class QValue3DAxis;

class Q_GRAPHS_EXPORT QValue3DAxisFormatter : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QValue3DAxisFormatter)
    QML_NAMED_ELEMENT(Value3DAxisFormatter)

protected:
    explicit QValue3DAxisFormatter(QValue3DAxisFormatterPrivate &d, QObject *parent = nullptr);

public:
    explicit QValue3DAxisFormatter(QObject *parent = nullptr);
    ~QValue3DAxisFormatter() override;

private Q_SLOTS:
    void markDirtyNoLabelChange();

protected:
    void setAllowNegatives(bool allow);
    bool allowNegatives() const;
    void setAllowZero(bool allow);
    bool allowZero() const;
    void setAxis(QValue3DAxis *axis);

    virtual QValue3DAxisFormatter *createNewInstance() const;
    virtual void recalculate();
    virtual QString stringForValue(qreal value, const QString &format);
    virtual float positionAt(float value) const;
    virtual float valueAt(float position) const;
    virtual void populateCopy(QValue3DAxisFormatter &copy);

    void markDirty(bool labelsChange = false);
    QValue3DAxis *axis() const;

    const QList<float> &gridPositions() const &;
    QList<float> gridPositions() &&;
    void setGridPoitions(QList<float> gridPositions);
    const QList<float> &subGridPositions() const &;
    QList<float> subGridPositions() &&;
    void setSubGridPositions(QList<float> subGridPositions);
    const QList<float> &labelPositions() const &;
    QList<float> labelPositions() &&;
    void setlabelPositions(QList<float> labelPositions);
    const QStringList &labelStrings() const &;
    QStringList labelStrings() &&;
    void setLabelStrings(QStringList labelStrings);

    void setLocale(const QLocale &locale);
    QLocale locale() const;

private:
    Q_DISABLE_COPY(QValue3DAxisFormatter)

    friend class QValue3DAxis;
    friend class QValue3DAxisPrivate;
    friend class QBar3DSeriesPrivate;
    friend class QScatter3DSeriesPrivate;
    friend class QSurface3DSeriesPrivate;
    friend class QQuickGraphsItem;
    friend class QQuickGraphsScatter;
    friend class QQuickGraphsBars;
};

QT_END_NAMESPACE

#endif
