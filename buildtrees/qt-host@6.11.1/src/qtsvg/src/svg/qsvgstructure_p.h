// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSVGSTRUCTURE_P_H
#define QSVGSTRUCTURE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qsvgnode_p.h"
#include <QtSvg/private/qsvghelper_p.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class QSvgDocument;
class QSvgNode;
class QPainter;
class QSvgDefs;

class Q_SVG_EXPORT QSvgStructureNode : public QSvgNode
{
public:
    QSvgStructureNode(QSvgNode *parent);
    ~QSvgStructureNode();
    QSvgNode *scopeNode(const QString &id) const;
    void addChild(std::unique_ptr<QSvgNode> child, const QString &id);
    QRectF internalBounds(QPainter *p, QSvgExtraStates &states) const override;
    QRectF decoratedInternalBounds(QPainter *p, QSvgExtraStates &states) const override;
    QSvgNode *previousSiblingNode(QSvgNode *n) const;
    const std::vector<std::unique_ptr<QSvgNode>> &renderers() const { return m_renderers; }
protected:
    std::vector<std::unique_ptr<QSvgNode>>          m_renderers;
    mutable bool              m_recursing = false;

private:
    Q_DISABLE_COPY_X(QSvgStructureNode, "Class has a vector of unique"
                                        "pointers as member variable")
};

class Q_SVG_EXPORT QSvgG : public QSvgStructureNode
{
public:
    QSvgG(QSvgNode *parent);
    void drawCommand(QPainter *, QSvgExtraStates &) override;
    bool shouldDrawNode(QPainter *p, QSvgExtraStates &states) const override;
    Type type() const override;
    bool requiresGroupRendering() const override;
};

class Q_SVG_EXPORT QSvgDefs : public QSvgStructureNode
{
public:
    QSvgDefs(QSvgNode *parent);
    void drawCommand(QPainter *, QSvgExtraStates &) override {};
    bool shouldDrawNode(QPainter *p, QSvgExtraStates &states) const override;
    Type type() const override;
};

class Q_SVG_EXPORT QSvgSymbolLike : public QSvgStructureNode
{
    // Marker, Symbol and potentially other elements share a lot of common
    // attributes and functionality. By making a common base class we can
    // avoid repetition.
public:
    enum class Overflow : quint8 {
        Visible,
        Hidden,
        Scroll = Visible, //Will not support scrolling
        Auto = Visible
    };

    enum class PreserveAspectRatio : quint8 {
        None =  0b000000,
        xMin =  0b000001,
        xMid =  0b000010,
        xMax =  0b000011,
        yMin =  0b000100,
        yMid =  0b001000,
        yMax =  0b001100,
        meet =  0b010000,
        slice = 0b100000,
        xMask = xMin | xMid | xMax,
        yMask = yMin | yMid | yMax,
        xyMask = xMask | yMask,
        meetSliceMask = meet | slice
    };
    Q_DECLARE_FLAGS(PreserveAspectRatios, PreserveAspectRatio)

    QSvgSymbolLike(QSvgNode *parent, QRectF bounds, QRectF viewBox, QPointF refP,
                   QSvgSymbolLike::PreserveAspectRatios pAspectRatios, QSvgSymbolLike::Overflow overflow);
    void drawCommand(QPainter *, QSvgExtraStates &) override {};
    QRectF decoratedInternalBounds(QPainter *p, QSvgExtraStates &states) const override;
    bool requiresGroupRendering() const override;

    QRectF viewBox() const
    {
        return m_viewBox;
    }

    QRectF rect() const
    {
        return m_rect;
    }

    QPointF refP() const
    {
        return m_refP;
    }

    QTransform aspectRatioTransform() const;
    QRectF clipRect() const;

    Overflow overflow() const
    {
        return m_overflow;
    }

    PreserveAspectRatios preserveAspectRatios() const
    {
        return m_pAspectRatios;
    }

protected:
    void setPainterToRectAndAdjustment(QPainter *p) const;
protected:
    QRectF m_rect;
    QRectF m_viewBox;
    QPointF m_refP;
    PreserveAspectRatios m_pAspectRatios;
    Overflow m_overflow;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSvgSymbolLike::PreserveAspectRatios)

class Q_SVG_EXPORT QSvgSymbol : public QSvgSymbolLike
{
public:
    QSvgSymbol(QSvgNode *parent, QRectF bounds, QRectF viewBox, QPointF refP,
               QSvgSymbolLike::PreserveAspectRatios pAspectRatios, QSvgSymbolLike::Overflow overflow);
    void drawCommand(QPainter *p, QSvgExtraStates &states) override;
    Type type() const override;
};

class Q_SVG_EXPORT QSvgMarker : public QSvgSymbolLike
{
public:
    enum class Orientation : quint8 {
        Auto,
        AutoStartReverse,
        Value
    };
    enum class MarkerUnits : quint8 {
        StrokeWidth,
        UserSpaceOnUse
    };

    QSvgMarker(QSvgNode *parent, QRectF bounds, QRectF viewBox, QPointF refP,
               QSvgSymbolLike::PreserveAspectRatios pAspectRatios, QSvgSymbolLike::Overflow overflow,
               Orientation orientation, qreal orientationAngle, MarkerUnits markerUnits);
    void drawCommand(QPainter *p, QSvgExtraStates &states) override;
    static void drawMarkersForNode(QSvgNode *node, QPainter *p, QSvgExtraStates &states);
    static QRectF markersBoundsForNode(const QSvgNode *node, QPainter *p, QSvgExtraStates &states);

    Orientation orientation() const {
        return m_orientation;
    }
    qreal orientationAngle() const {
        return m_orientationAngle;
    }
    MarkerUnits markerUnits() const {
        return m_markerUnits;
    }
    Type type() const override;

private:
    static void drawHelper(const QSvgNode *node, QPainter *p,
                           QSvgExtraStates &states, QRectF *boundingRect = nullptr);

    Orientation m_orientation;
    qreal m_orientationAngle;
    MarkerUnits m_markerUnits;
};

class Q_SVG_EXPORT QSvgFilterContainer : public QSvgStructureNode
{
public:

    QSvgFilterContainer(QSvgNode *parent, const QSvgRectF &bounds, QtSvg::UnitTypes filterUnits, QtSvg::UnitTypes primitiveUnits);
    void drawCommand(QPainter *, QSvgExtraStates &) override {};
    bool shouldDrawNode(QPainter *, QSvgExtraStates &) const override;
    Type type() const override;
    QImage applyFilter(const QImage &buffer, QPainter *p, const QRectF &bounds) const;
    void setSupported(bool supported);
    bool supported() const;
    QRectF filterRegion(const QRectF &itemBounds) const;

    QSvgRectF rect() const { return m_rect; }
    QtSvg::UnitTypes filterUnits() const { return m_filterUnits; }
    QtSvg::UnitTypes primitiveUnits() const { return m_primitiveUnits; }

private:
    QSvgRectF m_rect;
    QtSvg::UnitTypes m_filterUnits;
    QtSvg::UnitTypes m_primitiveUnits;
    bool m_supported;
};


class Q_SVG_EXPORT QSvgSwitch : public QSvgStructureNode
{
public:
    QSvgSwitch(QSvgNode *parent);
    void drawCommand(QPainter *p, QSvgExtraStates &states) override;
    Type type() const override;

    QSvgNode *childToRender() const;
private:
    void init();
private:
    QString m_systemLanguage;
    QString m_systemLanguagePrefix;
};

class Q_SVG_EXPORT QSvgMask : public QSvgStructureNode
{
public:
    QSvgMask(QSvgNode *parent, QSvgRectF bounds,
             QtSvg::UnitTypes contentsUnits);
    void drawCommand(QPainter *, QSvgExtraStates &) override {};
    bool shouldDrawNode(QPainter *, QSvgExtraStates &) const override;
    Type type() const override;
    QImage createMask(QPainter *p, QSvgExtraStates &states, QSvgNode *targetNode, QRectF *globalRect) const;
    QImage createMask(QPainter *p, QSvgExtraStates &states, const QRectF &localRect, QRectF *globalRect) const;

    QSvgRectF rect() const
    {
        return m_rect;
    }

    QtSvg::UnitTypes contentUnits() const
    {
        return m_contentUnits;
    }

private:
    QSvgRectF m_rect;
    QtSvg::UnitTypes m_contentUnits;
};

class Q_SVG_EXPORT QSvgPattern : public QSvgStructureNode
{
public:
    QSvgPattern(QSvgNode *parent, QSvgRectF bounds, QRectF viewBox,
                QtSvg::UnitTypes contentUnits, QTransform transform);
    void drawCommand(QPainter *, QSvgExtraStates &) override {};
    bool shouldDrawNode(QPainter *, QSvgExtraStates &) const override;
    QImage patternImage(QPainter *p, QSvgExtraStates &states, const QSvgNode *patternElement);
    Type type() const override;
    const QTransform& appliedTransform() const { return m_appliedTransform; }

private:
    QImage renderPattern(QSize size, qreal contentScaleX, qreal contentScaleY);
    void calculateAppliedTransform(QTransform& worldTransform, QRectF peLocalBB, QSize imageSize);

private:
    QTransform m_appliedTransform;
    QSvgRectF m_rect;
    QRectF m_viewBox;
    QtSvg::UnitTypes m_contentUnits;
    mutable bool m_isRendering;
    QTransform m_transform;
};

QT_END_NAMESPACE

#endif // QSVGSTRUCTURE_P_H
