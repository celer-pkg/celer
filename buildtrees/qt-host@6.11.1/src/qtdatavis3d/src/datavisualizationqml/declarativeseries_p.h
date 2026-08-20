// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef DECLARATIVESERIES_P_H
#define DECLARATIVESERIES_P_H

#include "qbar3dseries.h"
#include "qscatter3dseries.h"
#include "qsurface3dseries.h"
#include "colorgradient_p.h"
#include "declarativecolor_p.h"

#include <private/datavisualizationglobal_p.h>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

enum GradientType {
    GradientTypeBase,
    GradientTypeSingle,
    GradientTypeMulti
};

class DeclarativeBar3DSeries : public QBar3DSeries
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> seriesChildren READ seriesChildren)
    // This property is overloaded to use QPointF instead of QPoint to work around qml bug
    // where Qt.point(0, 0) can't be assigned due to error "Cannot assign QPointF to QPoint".
    Q_PROPERTY(QPointF selectedBar READ selectedBar WRITE setSelectedBar NOTIFY selectedBarChanged)
    // This is static method in parent class, overload as constant property for qml.
    Q_PROPERTY(QPointF invalidSelectionPosition READ invalidSelectionPosition CONSTANT)
    Q_PROPERTY(ColorGradient *baseGradient READ baseGradient WRITE setBaseGradient NOTIFY baseGradientChanged)
    Q_PROPERTY(ColorGradient *singleHighlightGradient READ singleHighlightGradient WRITE setSingleHighlightGradient NOTIFY singleHighlightGradientChanged)
    Q_PROPERTY(ColorGradient *multiHighlightGradient READ multiHighlightGradient WRITE setMultiHighlightGradient NOTIFY multiHighlightGradientChanged)
    Q_PROPERTY(QQmlListProperty<DeclarativeColor> rowColors READ rowColors REVISION(6, 3))
    Q_CLASSINFO("DefaultProperty", "seriesChildren")

    QML_NAMED_ELEMENT(Bar3DSeries)
    QML_ADDED_IN_VERSION(1, 0)

public:
    DeclarativeBar3DSeries(QObject *parent = 0);
    virtual ~DeclarativeBar3DSeries();

    QQmlListProperty<QObject> seriesChildren();
    static void appendSeriesChildren(QQmlListProperty<QObject> *list, QObject *element);

    void setSelectedBar(const QPointF &position);
    QPointF selectedBar() const;
    QPointF invalidSelectionPosition() const;

    void setBaseGradient(ColorGradient *gradient);
    ColorGradient *baseGradient() const;
    void setSingleHighlightGradient(ColorGradient *gradient);
    ColorGradient *singleHighlightGradient() const;
    void setMultiHighlightGradient(ColorGradient *gradient);
    ColorGradient *multiHighlightGradient() const;

    QQmlListProperty<DeclarativeColor> rowColors();
    static void appendRowColorsFunc(QQmlListProperty<DeclarativeColor> *list,
                                    DeclarativeColor *color);
    static qsizetype countRowColorsFunc(QQmlListProperty<DeclarativeColor> *list);
    static DeclarativeColor *atRowColorsFunc(QQmlListProperty<DeclarativeColor> *list,
                                             qsizetype index);
    static void clearRowColorsFunc(QQmlListProperty<DeclarativeColor> *list);

public Q_SLOTS:
    void handleBaseGradientUpdate();
    void handleSingleHighlightGradientUpdate();
    void handleMultiHighlightGradientUpdate();
    void handleRowColorUpdate();

Q_SIGNALS:
    void selectedBarChanged(const QPointF &position);
    void baseGradientChanged(ColorGradient *gradient);
    void singleHighlightGradientChanged(ColorGradient *gradient);
    void multiHighlightGradientChanged(ColorGradient *gradient);

private:
    ColorGradient *m_baseGradient; // Not owned
    ColorGradient *m_singleHighlightGradient; // Not owned
    ColorGradient *m_multiHighlightGradient; // Not owned

    QList<DeclarativeColor *> m_rowColors;
    bool m_dummyColors;

    void addColor(DeclarativeColor *color);
    QList<DeclarativeColor *> colorList();
    void clearColors();
    void clearDummyColors();
};

class DeclarativeScatter3DSeries : public QScatter3DSeries
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> seriesChildren READ seriesChildren)
    Q_PROPERTY(ColorGradient *baseGradient READ baseGradient WRITE setBaseGradient NOTIFY baseGradientChanged)
    Q_PROPERTY(ColorGradient *singleHighlightGradient READ singleHighlightGradient WRITE setSingleHighlightGradient NOTIFY singleHighlightGradientChanged)
    Q_PROPERTY(ColorGradient *multiHighlightGradient READ multiHighlightGradient WRITE setMultiHighlightGradient NOTIFY multiHighlightGradientChanged)
    // This is static method in parent class, overload as constant property for qml.
    Q_PROPERTY(int invalidSelectionIndex READ invalidSelectionIndex CONSTANT)
    Q_CLASSINFO("DefaultProperty", "seriesChildren")

    QML_NAMED_ELEMENT(Scatter3DSeries)
    QML_ADDED_IN_VERSION(1, 0)

public:
    DeclarativeScatter3DSeries(QObject *parent = 0);
    virtual ~DeclarativeScatter3DSeries();

    QQmlListProperty<QObject> seriesChildren();
    static void appendSeriesChildren(QQmlListProperty<QObject> *list, QObject *element);

    void setBaseGradient(ColorGradient *gradient);
    ColorGradient *baseGradient() const;
    void setSingleHighlightGradient(ColorGradient *gradient);
    ColorGradient *singleHighlightGradient() const;
    void setMultiHighlightGradient(ColorGradient *gradient);
    ColorGradient *multiHighlightGradient() const;

    int invalidSelectionIndex() const;

public Q_SLOTS:
    void handleBaseGradientUpdate();
    void handleSingleHighlightGradientUpdate();
    void handleMultiHighlightGradientUpdate();

Q_SIGNALS:
    void baseGradientChanged(ColorGradient *gradient);
    void singleHighlightGradientChanged(ColorGradient *gradient);
    void multiHighlightGradientChanged(ColorGradient *gradient);

private:
    ColorGradient *m_baseGradient; // Not owned
    ColorGradient *m_singleHighlightGradient; // Not owned
    ColorGradient *m_multiHighlightGradient; // Not owned
};

class DeclarativeSurface3DSeries : public QSurface3DSeries
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> seriesChildren READ seriesChildren)
    // This property is overloaded to use QPointF instead of QPoint to work around qml bug
    // where Qt.point(0, 0) can't be assigned due to error "Cannot assign QPointF to QPoint".
    Q_PROPERTY(QPointF selectedPoint READ selectedPoint WRITE setSelectedPoint NOTIFY selectedPointChanged)
    // This is static method in parent class, overload as constant property for qml.
    Q_PROPERTY(QPointF invalidSelectionPosition READ invalidSelectionPosition CONSTANT)
    Q_PROPERTY(ColorGradient *baseGradient READ baseGradient WRITE setBaseGradient NOTIFY baseGradientChanged)
    Q_PROPERTY(ColorGradient *singleHighlightGradient READ singleHighlightGradient WRITE setSingleHighlightGradient NOTIFY singleHighlightGradientChanged)
    Q_PROPERTY(ColorGradient *multiHighlightGradient READ multiHighlightGradient WRITE setMultiHighlightGradient NOTIFY multiHighlightGradientChanged)
    Q_CLASSINFO("DefaultProperty", "seriesChildren")

    QML_NAMED_ELEMENT(Surface3DSeries)
    QML_ADDED_IN_VERSION(1, 0)

public:
    DeclarativeSurface3DSeries(QObject *parent = 0);
    virtual ~DeclarativeSurface3DSeries();

    void setSelectedPoint(const QPointF &position);
    QPointF selectedPoint() const;
    QPointF invalidSelectionPosition() const;

    QQmlListProperty<QObject> seriesChildren();
    static void appendSeriesChildren(QQmlListProperty<QObject> *list, QObject *element);

    void setBaseGradient(ColorGradient *gradient);
    ColorGradient *baseGradient() const;
    void setSingleHighlightGradient(ColorGradient *gradient);
    ColorGradient *singleHighlightGradient() const;
    void setMultiHighlightGradient(ColorGradient *gradient);
    ColorGradient *multiHighlightGradient() const;

public Q_SLOTS:
    void handleBaseGradientUpdate();
    void handleSingleHighlightGradientUpdate();
    void handleMultiHighlightGradientUpdate();

Q_SIGNALS:
    void selectedPointChanged(const QPointF &position);
    void baseGradientChanged(ColorGradient *gradient);
    void singleHighlightGradientChanged(ColorGradient *gradient);
    void multiHighlightGradientChanged(ColorGradient *gradient);

private:
    ColorGradient *m_baseGradient; // Not owned
    ColorGradient *m_singleHighlightGradient; // Not owned
    ColorGradient *m_multiHighlightGradient; // Not owned
};

QT_END_NAMESPACE

#endif
