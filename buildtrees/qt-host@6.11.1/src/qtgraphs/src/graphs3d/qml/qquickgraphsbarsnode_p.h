// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
#ifndef QQUICKGRAPHSBARSNODE_P_H
#define QQUICKGRAPHSBARSNODE_P_H

#include "qbar3dseries.h"
#include "qquickgraphsbars_p.h"
#include "qquickgraphsnode_p.h"

#include <private/qgraphsglobal_p.h>

QT_BEGIN_NAMESPACE

class QQuickGraphsBarsNode;

class Q_GRAPHS_EXPORT QQuickGraphsBarsNode : public QQuickGraphsNode
{
    Q_OBJECT
    Q_PROPERTY(QCategory3DAxis *rowAxis READ rowAxis WRITE setRowAxis NOTIFY rowAxisChanged)
    Q_PROPERTY(QValue3DAxis *valueAxis READ valueAxis WRITE setValueAxis NOTIFY valueAxisChanged)
    Q_PROPERTY(
        QCategory3DAxis *columnAxis READ columnAxis WRITE setColumnAxis NOTIFY columnAxisChanged)
    Q_PROPERTY(bool multiSeriesUniform READ isMultiSeriesUniform WRITE setMultiSeriesUniform NOTIFY
                   multiSeriesUniformChanged)
    Q_PROPERTY(float barThickness READ barThickness WRITE setBarThickness NOTIFY barThicknessChanged)
    Q_PROPERTY(QSizeF barSpacing READ barSpacing WRITE setBarSpacing NOTIFY barSpacingChanged)
    Q_PROPERTY(bool barSpacingRelative READ isBarSpacingRelative WRITE setBarSpacingRelative NOTIFY
                   barSpacingRelativeChanged)
    Q_PROPERTY(QSizeF barSeriesMargin READ barSeriesMargin WRITE setBarSeriesMargin NOTIFY
                   barSeriesMarginChanged)
    Q_PROPERTY(QQmlListProperty<QBar3DSeries> seriesList READ seriesList CONSTANT)
    Q_PROPERTY(QBar3DSeries *selectedSeries READ selectedSeries NOTIFY selectedSeriesChanged)
    Q_PROPERTY(QBar3DSeries *primarySeries READ primarySeries WRITE setPrimarySeries NOTIFY
                   primarySeriesChanged)
    Q_PROPERTY(float floorLevel READ floorLevel WRITE setFloorLevel NOTIFY floorLevelChanged)
    Q_CLASSINFO("DefaultProperty", "seriesList")

    QML_ADDED_IN_VERSION(6, 10)
    QML_NAMED_ELEMENT(Bars3DNode)

public:
    explicit QQuickGraphsBarsNode(QQuick3DNode *parent = nullptr);
    ~QQuickGraphsBarsNode() override;

    void setRowAxis(QCategory3DAxis *axis);
    QCategory3DAxis *rowAxis() const;
    void setValueAxis(QValue3DAxis *axis);
    QValue3DAxis *valueAxis() const;
    void setColumnAxis(QCategory3DAxis *axis);
    QCategory3DAxis *columnAxis() const;

    void setMultiSeriesUniform(bool uniform);
    bool isMultiSeriesUniform() const;

    void setBarThickness(float thickness);
    float barThickness() const;

    void setBarSpacing(QSizeF spacing);
    QSizeF barSpacing() const;

    void setBarSpacingRelative(bool relative);
    bool isBarSpacingRelative() const;

    void setBarSeriesMargin(QSizeF margin);
    QSizeF barSeriesMargin() const;

    QBar3DSeries *selectedSeries() const;

    void setPrimarySeries(QBar3DSeries *series);
    QBar3DSeries *primarySeries() const;

    void setFloorLevel(float floorLevel);
    float floorLevel() const;

    QList<QBar3DSeries *> barSeriesList();
    QQmlListProperty<QBar3DSeries> seriesList();
    static void appendSeriesFunc(QQmlListProperty<QBar3DSeries> *list, QBar3DSeries *series);
    static qsizetype countSeriesFunc(QQmlListProperty<QBar3DSeries> *list);
    static QBar3DSeries *atSeriesFunc(QQmlListProperty<QBar3DSeries> *list, qsizetype index);
    static void clearSeriesFunc(QQmlListProperty<QBar3DSeries> *list);

    Q_INVOKABLE void addSeries(QBar3DSeries *series);
    Q_INVOKABLE void removeSeries(QBar3DSeries *series);
    Q_INVOKABLE void insertSeries(qsizetype index, QBar3DSeries *series);
    Q_INVOKABLE void clearSelection() override;


    Q_INVOKABLE bool doPicking(QPointF point) override;
    Q_INVOKABLE bool doRayPicking(const QVector3D &origin, const QVector3D &direction) override;

protected:
    void componentComplete() override;

Q_SIGNALS:
    void rowAxisChanged(QCategory3DAxis *axis);
    void valueAxisChanged(QValue3DAxis *axis);
    void columnAxisChanged(QCategory3DAxis *axis);
    void multiSeriesUniformChanged(bool uniform);
    void barThicknessChanged(float thicknessRatio);
    void barSpacingChanged(QSizeF spacing);
    void barSpacingRelativeChanged(bool relative);
    void barSeriesMarginChanged(QSizeF margin);
    void primarySeriesChanged(QBar3DSeries *series);
    void selectedSeriesChanged(QBar3DSeries *series);
    void floorLevelChanged(float level);

private:
    QQuickGraphsBars *graphBars();
    const QQuickGraphsBars *graphBars() const;

    bool m_multiSeriesUniform;
    QSizeF m_barSpacing;
    float m_barThickness;
    bool m_barSpacingRelative;
    QSizeF m_barSeriesMargin;
    QBar3DSeries *m_primarySeries = nullptr;
    QBar3DSeries *m_selectedSeries = nullptr;
    float m_floorLevel;
};

QT_END_NAMESPACE

#endif // QQUICKGRAPHSBARSNODE_P_H
