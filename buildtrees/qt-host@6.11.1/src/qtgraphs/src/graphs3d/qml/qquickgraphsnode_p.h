// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICKGRAPHSNODE_P_H
#define QQUICKGRAPHSNODE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#include "qgraphs3dnamespace.h"
#include "qquickgraphsitem_p.h"
#include <QtQuick3D/private/qquick3dnode_p.h>

QT_BEGIN_NAMESPACE

class Q_GRAPHS_EXPORT QQuickGraphsNode : public QQuick3DNode
{
    Q_OBJECT
    Q_PROPERTY(QtGraphs3D::SelectionFlags selectionMode READ selectionMode WRITE setSelectionMode
                   NOTIFY selectionModeChanged)
    Q_PROPERTY(QGraphsTheme *theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(QQmlListProperty<QCustom3DItem> customItemList READ customItemList CONSTANT)
    Q_PROPERTY(
        QtGraphs3D::ElementType selectedElement READ selectedElement NOTIFY selectedElementChanged)
    Q_PROPERTY(qreal aspectRatio READ aspectRatio WRITE setAspectRatio NOTIFY aspectRatioChanged)
    Q_PROPERTY(QtGraphs3D::OptimizationHint optimizationHint READ optimizationHint WRITE
                   setOptimizationHint NOTIFY optimizationHintChanged)
    Q_PROPERTY(bool polar READ isPolar WRITE setPolar NOTIFY polarChanged)
    Q_PROPERTY(float labelMargin READ labelMargin WRITE setLabelMargin NOTIFY labelMarginChanged)
    Q_PROPERTY(float radialLabelOffset READ radialLabelOffset WRITE setRadialLabelOffset NOTIFY
                   radialLabelOffsetChanged)
    Q_PROPERTY(qreal horizontalAspectRatio READ horizontalAspectRatio WRITE setHorizontalAspectRatio
                   NOTIFY horizontalAspectRatioChanged)
    Q_PROPERTY(QLocale locale READ locale WRITE setLocale NOTIFY localeChanged)
    Q_PROPERTY(
        QVector3D queriedGraphPosition READ queriedGraphPosition NOTIFY queriedGraphPositionChanged)
    Q_PROPERTY(qreal margin READ margin WRITE setMargin NOTIFY marginChanged)
    Q_PROPERTY(QtGraphs3D::GridLineType gridLineType READ gridLineType WRITE setGridLineType NOTIFY
                   gridLineTypeChanged FINAL)

    QML_NAMED_ELEMENT(GraphsNode)
    QML_UNCREATABLE("")

public:
    explicit QQuickGraphsNode(QQuick3DNode *parent = nullptr);
    ~QQuickGraphsNode();

    void componentComplete() override;

    QtGraphs3D::ElementType selectedElement() const;

    virtual void setSelectionMode(QtGraphs3D::SelectionFlags selectionMode);
    QtGraphs3D::SelectionFlags selectionMode() const;

    void setTheme(QGraphsTheme *theme);
    QGraphsTheme *theme() const;

    QQmlListProperty<QCustom3DItem> customItemList() const;

    void setAspectRatio(qreal aspectRatio);
    qreal aspectRatio() const;

    void setOptimizationHint(QtGraphs3D::OptimizationHint optimizationHint);
    QtGraphs3D::OptimizationHint optimizationHint() const;

    void setPolar(bool enabled);
    bool isPolar() const;

    void setLabelMargin(float labelMargin);
    float labelMargin() const;

    void setRadialLabelOffset(float radialLabelOffset);
    float radialLabelOffset() const;

    void setHorizontalAspectRatio(qreal horizontalAspectRatio);
    qreal horizontalAspectRatio() const;

    void setLocale(QLocale locale);
    QLocale locale() const;

    QVector3D queriedGraphPosition() const;

    void setMargin(qreal margin);
    qreal margin() const;

    QtGraphs3D::GridLineType gridLineType() const;
    void setGridLineType(const QtGraphs3D::GridLineType &gridLineType);

    Q_INVOKABLE virtual bool hasSeries(QAbstract3DSeries *series);
    Q_INVOKABLE virtual void clearSelection() = 0;

    virtual void addSeriesInternal(QAbstract3DSeries *series);
    void insertSeries(qsizetype index, QAbstract3DSeries *series);
    virtual void removeSeriesInternal(QAbstract3DSeries *series);
    QList<QAbstract3DSeries *> seriesList();

    Q_INVOKABLE virtual qsizetype addCustomItem(QCustom3DItem *item);
    Q_INVOKABLE virtual void removeCustomItems();
    Q_INVOKABLE virtual void removeCustomItem(QCustom3DItem *item);
    Q_INVOKABLE virtual void removeCustomItemAt(QVector3D position);
    Q_INVOKABLE virtual void releaseCustomItem(QCustom3DItem *item);

    Q_INVOKABLE virtual int selectedLabelIndex() const;
    Q_INVOKABLE virtual QAbstract3DAxis *selectedAxis() const;

    Q_INVOKABLE virtual qsizetype selectedCustomItemIndex() const;
    Q_INVOKABLE virtual QCustom3DItem *selectedCustomItem() const;

    Q_INVOKABLE virtual bool doPicking(QPointF point) = 0;
    Q_INVOKABLE virtual bool doRayPicking(const QVector3D &origin, const QVector3D &direction) = 0;

protected:
    void setGraphParent();
    std::unique_ptr<QQuickGraphsItem> m_graph;
    QList<QAbstract3DSeries *> m_seriesList;
    QtGraphs3D::SelectionFlags m_selectionMode;

    QAbstract3DAxis *m_axisX = nullptr;
    QAbstract3DAxis *m_axisY = nullptr;
    QAbstract3DAxis *m_axisZ = nullptr;

Q_SIGNALS:
    void queriedGraphPositionChanged();
    void selectedElementChanged();
    void optimizationHintChanged();
    void selectionModeChanged();
    void themeChanged();
    void aspectRatioChanged();
    void polarChanged();
    void labelMarginChanged();
    void radialLabelOffsetChanged();
    void horizontalAspectRatioChanged();
    void localeChanged();
    void marginChanged(qreal margin);
    void gridLineTypeChanged();

private:
    QGraphsTheme *m_theme = nullptr;
    qreal m_aspectRatio;
    QtGraphs3D::OptimizationHint m_optimizationHint;
    bool m_polar;
    float m_labelMargin;
    float m_radialLabelOffset;
    qreal m_horizontalAspectRatio;
    QLocale m_locale;
    qreal m_margin;
    QtGraphs3D::GridLineType m_gridLineType;
    QList<QCustom3DItem *> m_customItemList = {};
};

QT_END_NAMESPACE

#endif // QQUICKGRAPHSNODE_P_H
