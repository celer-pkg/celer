// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIERASTERRENDERER_H
#define QLOTTIERASTERRENDERER_H

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

#include <QPainterPath>
#include <QPainter>
#include <QStack>
#include <QRegion>

#include <QtLottie/private/qlottierenderer_p.h>

QT_BEGIN_NAMESPACE

class QPainter;

class QLottieRasterRenderer : public QLottieRenderer
{
public:
    explicit QLottieRasterRenderer(QPainter *m_painter);
    ~QLottieRasterRenderer() override = default;

    void saveState() override;
    void restoreState() override;

    void render(const QLottieLayer &layer) override;
    void render(const QLottieSolidLayer &layer) override;
    void render(const QLottieRect &rect) override;
    void render(const QLottieEllipse &ellipse) override;
    void render(const QLottiePolyStar &star) override;
    void render(const QLottieRound &round) override;
    void render(const QLottieFill &fill) override;
    void render(const QLottieGFill &shape) override;
    void render(const QLottieImage &image) override;
    void render(const QLottieStroke &stroke) override;
    void render(const QLottieBasicTransform &transform) override;
    void render(const QLottieShapeTransform &transform) override;
    void render(const QLottieFreeFormShape &shape) override;
    void render(const QLottieTrimPath &trans) override;
    void render(const QLottieFillEffect &effect) override;
    void render(const QLottieRepeater &repeater) override;
    void renderPathElements(const QList<QLottieBase *> &pathElements) override;

protected:
    QPainter *m_painter = nullptr;
    QPainterPath m_renderPath;
    QPainterPath m_unitedPath;
    // TODO: create a context to handle paths and effect
    // instead of pushing each to a stack independently
    QStack<QPainterPath> m_pathStack;
    QStack<const QLottieFillEffect*> m_fillEffectStack;
    const QLottieFillEffect *m_fillEffect = nullptr;
    const QLottieRepeaterTransform *m_repeaterTransform = nullptr;
    int m_repeatCount = 1;
    qreal m_repeatOffset = 0.0;
    bool m_buildingClipRegion = false;
    QPainterPath m_clipPath;

private:
    void applyRepeaterTransform(int instance);
};

QT_END_NAMESPACE

#endif // QLOTTIERASTERRENDERER_H

