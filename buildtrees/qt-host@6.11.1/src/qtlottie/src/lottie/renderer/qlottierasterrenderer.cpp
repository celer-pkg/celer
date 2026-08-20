// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtLottie/private/qlottierasterrenderer_p.h>

#include <QPainter>
#include <QRectF>
#include <QBrush>
#include <QTransform>
#include <QGradient>
#include <QPointer>

#include <QtLottie/private/qlottieshape_p.h>
#include <QtLottie/private/qlottiefill_p.h>
#include <QtLottie/private/qlottiegfill_p.h>
#include <QtLottie/private/qlottieimage_p.h>
#include <QtLottie/private/qlottiebasictransform_p.h>
#include <QtLottie/private/qlottieshapetransform_p.h>
#include <QtLottie/private/qlottierect_p.h>
#include <QtLottie/private/qlottieellipse_p.h>
#include <QtLottie/private/qlottiepolystar_p.h>
#include <QtLottie/private/qlottieround_p.h>
#include <QtLottie/private/qlottiefreeformshape_p.h>
#include <QtLottie/private/qlottietrimpath_p.h>
#include <QtLottie/private/qlottiefilleffect_p.h>
#include <QtLottie/private/qlottierepeater_p.h>
#include <QtLottie/private/qlottieflatlayers_p.h>

QT_BEGIN_NAMESPACE

QLottieRasterRenderer::QLottieRasterRenderer(QPainter *painter)
    : m_painter(painter)
{
    m_painter->setPen(QPen(Qt::NoPen));
    m_unitedPath.setCachingEnabled(true);
}

void QLottieRasterRenderer::saveState()
{
    qCDebug(lcLottieQtLottieRender) << "Save painter state";
    m_painter->save();
    saveTrimmingState();
    m_pathStack.push_back(m_unitedPath);
    m_fillEffectStack.push_back(m_fillEffect);
    m_unitedPath.clear();
}

void QLottieRasterRenderer::restoreState()
{
    qCDebug(lcLottieQtLottieRender) << "Restore painter state";
    m_painter->restore();
    restoreTrimmingState();
    m_unitedPath = m_pathStack.pop();
    m_renderPath.setFillRule(m_unitedPath.fillRule());
    m_clipPath.setFillRule(m_unitedPath.fillRule());
    m_fillEffect = m_fillEffectStack.pop();
}

void QLottieRasterRenderer::render(const QLottieLayer &layer)
{
    qCDebug(lcLottieQtLottieRender) << "Layer:" << layer.name()
                                    << "is matte:" << layer.isMatteLayer()
                                    << "using matte:" << layer.isUsingMatteLayer();
    if (layer.isMatteLayer())
        m_buildingClipRegion = true;
    else if (!m_clipPath.isEmpty()) {
        QTransform inv = m_painter->transform().inverted();
        if (layer.matteMode() == QLottieLayer::Alpha)
            m_painter->setClipPath(inv.map(m_clipPath));
        else if (layer.matteMode() == QLottieLayer::InvertedAlpha) {
            QPainterPath screen;
            screen.addRect(0, 0, m_painter->device()->width(),
                           m_painter->device()->height());
            m_painter->setClipPath(inv.map(screen - m_clipPath));
        }
        else {
            // Clipping is not applied to paths that have
            // not setting clipping parameters
            m_painter->setClipping(false);
        }
        m_buildingClipRegion = false;
        m_clipPath = QPainterPath();
    }
}

void QLottieRasterRenderer::render(const QLottieSolidLayer &layer)
{
    render(static_cast<const QLottieLayer &>(layer));
    m_painter->fillRect(QRect(QPoint(), layer.layerSize()), layer.color());
}

void QLottieRasterRenderer::render(const QLottieRect &rect)
{
    qCDebug(lcLottieQtLottieRender) << "Rect:" << rect.name()
                                    << rect.position() << rect.size();
    m_renderPath.addPath(rect.path());
}

void QLottieRasterRenderer::render(const QLottieEllipse &ellipse)
{
    qCDebug(lcLottieQtLottieRender) << "Ellipse:" << ellipse.name()
                                    << ellipse.position()
                                    << ellipse.size();
    m_renderPath.addPath(ellipse.path());
}

void QLottieRasterRenderer::render(const QLottiePolyStar &star)
{
    qCDebug(lcLottieQtLottieRender) << "PolyStar:" << star.name()
                                    << star.position() << star.pointCount()
                                    << star.outerRadius() << star.innerRadius();
    m_renderPath.addPath(star.path());
}

void QLottieRasterRenderer::render(const QLottieImage &image)
{
    const QImage img = image.image();
    if (img.isNull())
        return;

    m_painter->save();

    for (int i = 0; i < m_repeatCount; i++) {
        qCDebug(lcLottieQtLottieRender) << "Image" << image.name();

        applyRepeaterTransform(i);

        m_painter->drawImage(QRectF(QPointF(), image.size()), img);
    }

    m_painter->restore();
}


void QLottieRasterRenderer::render(const QLottieRound &round)
{
    Q_UNUSED(round);
    // Rounded Corners modifier, not implemented
}

void QLottieRasterRenderer::render(const QLottieFill &fill)
{
    qCDebug(lcLottieQtLottieRender) << "Fill:" <<fill.name()
                                       << fill.color();

    if (m_fillEffect)
        return;

    float alpha = fill.color().alphaF() * fill.opacity() / 100.0f;
    QColor color = fill.color();
    color.setAlphaF(alpha);
    m_painter->setBrush(color);
    m_renderPath.setFillRule(fill.fillRule());
    m_unitedPath.setFillRule(fill.fillRule());
    m_clipPath.setFillRule(fill.fillRule());
}

void QLottieRasterRenderer::render(const QLottieGFill &gradient)
{
    qCDebug(lcLottieQtLottieRender) << "Gradient:" << gradient.name()
                                       << gradient.value();

    if (m_fillEffect)
        return;

    if (gradient.value())
        m_painter->setBrush(*gradient.value());
    else
        qCWarning(lcLottieQtLottieRender) << "Gradient:"
                                             << gradient.name()
                                             << "Cannot draw gradient fill";
    m_renderPath.setFillRule(gradient.fillRule());
    m_unitedPath.setFillRule(gradient.fillRule());
    m_clipPath.setFillRule(gradient.fillRule());
}

void QLottieRasterRenderer::render(const QLottieStroke &stroke)
{
    qCDebug(lcLottieQtLottieRender) << "Stroke:" << stroke.name()
                                       << stroke.pen() << stroke.pen().miterLimit();

    if (m_fillEffect)
        return;

    m_painter->setPen(stroke.pen());
}

void QLottieRasterRenderer::render(const QLottieBasicTransform &transform)
{
    QTransform t = m_painter->transform();
    applyTransform(&t, transform);
    m_painter->setTransform(t);
    m_painter->setOpacity(m_painter->opacity() * transform.opacity());

    qCDebug(lcLottieQtLottieRender) << transform.name()
                                       << m_painter->transform()
                                       << "opacity:"  << m_painter->opacity();
}

void QLottieRasterRenderer::render(const QLottieShapeTransform &transform)
{
    qCDebug(lcLottieQtLottieRender) << "Shape transform:"  << transform.name()
                                       << "of" << transform.parent()->name();

    QTransform t = m_painter->transform();
    applyTransform(&t, transform, true);
    m_painter->setTransform(t);
    m_painter->setOpacity(m_painter->opacity() * transform.opacity());

    qCDebug(lcLottieQtLottieRender) << transform.name()
                                   << m_painter->transform()
                                   << m_painter->opacity();
}

void QLottieRasterRenderer::render(const QLottieFreeFormShape &shape)
{
    qCDebug(lcLottieQtLottieRender) << "Render shape:"
                                    << shape.name() << "of"
                                    << shape.parent()->name();
    m_renderPath.addPath(shape.path());
}

void QLottieRasterRenderer::render(const QLottieTrimPath &trimPath)
{
    // TODO: Move "Sequential" trimming to the prerendering thread
    // Now it is done in the GUI thread

    m_painter->save();

    for (int i = 0; i < m_repeatCount; i ++) {
        qCDebug(lcLottieQtLottieRender) << "Render shape:"
                                           << trimPath.name() << "of"
                                           << trimPath.parent()->name();
        applyRepeaterTransform(i);
        if (!trimPath.isParallel() && !qFuzzyCompare(qreal(0.0), m_unitedPath.length())) {
            qCDebug(lcLottieQtLottieRender) << "Render trim path in the GUI thread";
            QPainterPath tr = trimPath.trim(m_unitedPath);
            // Do not use the applied transform, as the transform
            // is already included in m_unitedPath
            m_painter->setTransform(QTransform());
            m_painter->drawPath(tr);
        }
    }

    m_painter->restore();
}

void QLottieRasterRenderer::render(const QLottieFillEffect &effect)
{
    qCDebug(lcLottieQtLottieRender) << "Fill:" <<effect.name()
                                       << effect.color();

    m_fillEffect = &effect;
    m_painter->setBrush(m_fillEffect->color());
    m_painter->setOpacity(m_painter->opacity() * m_fillEffect->opacity());
}

void QLottieRasterRenderer::render(const QLottieRepeater &repeater)
{
    qCDebug(lcLottieQtLottieRender) << "Repeater:" <<repeater.name()
                                       << "count:" << repeater.copies();

    if (m_repeaterTransform) {
        qCWarning(lcLottieQtLottieRender) << "Only one Repeater can be active at a time!";
        return;
    }

    m_repeatCount = repeater.copies();
    m_repeatOffset = repeater.offset();

    // Can store pointer to transform, although the transform
    // is managed by another thread. The object will be available
    // until the frame has been rendered
    m_repeaterTransform = &repeater.transform();

    m_painter->translate(m_repeatOffset * m_repeaterTransform->position().x(),
                         m_repeatOffset * m_repeaterTransform->position().y());
}

void QLottieRasterRenderer::renderPathElements(const QList<QLottieBase *> &pathElements)
{
    qCDebug(lcLottieQtLottieRender) << "PathElements: count:" << pathElements.size();

    m_painter->save();

    m_renderPath.clear();
    for (const QLottieBase *element : pathElements)
        element->render(*this); // Accumulates element paths in m_renderPath

    for (int i = 0; i < m_repeatCount; i++) {
        if (i > 0)
            qCDebug(lcLottieQtLottieRender) << "Render repeated, combined path, iteration" << i + 1;
        applyRepeaterTransform(i);
        if (trimmingState() == QLottieRenderer::Sequential) {
            QTransform t = m_painter->transform();
            QPainterPath tp = t.map(m_renderPath);
            tp.addPath(m_unitedPath);
            m_unitedPath = tp;
        } else if (m_buildingClipRegion) {
            QTransform t = m_painter->transform();
            QPainterPath tp = t.map(m_renderPath);
            tp.addPath(m_clipPath);
            m_clipPath = tp;
        } else {
            m_painter->drawPath(m_renderPath);
        }
    }

    m_painter->restore();
}

void QLottieRasterRenderer::applyRepeaterTransform(int instance)
{
    if (!m_repeaterTransform || instance == 0)
        return;

    QTransform t = m_painter->transform();

    QPointF anchors = -m_repeaterTransform->anchorPoint();
    QPointF position = m_repeaterTransform->position();
    QPointF anchoredCenter = anchors + position;

    t.translate(anchoredCenter.x(), anchoredCenter.y());
    t.rotate(m_repeaterTransform->rotation());
    t.scale(m_repeaterTransform->scale().x(),
            m_repeaterTransform->scale().y());
    m_painter->setTransform(t);

    qreal o =m_repeaterTransform->opacityAtInstance(instance);

    m_painter->setOpacity(m_painter->opacity() * o);
}

QT_END_NAMESPACE
