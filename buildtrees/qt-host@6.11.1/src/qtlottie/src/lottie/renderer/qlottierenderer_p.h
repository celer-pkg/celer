// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIERENDERER_H
#define QLOTTIERENDERER_H

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

#include <QStack>
// #include <private/qglobal_p.h>

#include <QtLottie/qtlottieexports.h>

QT_BEGIN_NAMESPACE

class QLottieBase;
class QLottieLayer;
class QLottieSolidLayer;
class QLottieGroup;
class QLottieRect;
class QLottieFill;
class QLottieGFill;
class QLottieImage;
class QLottieStroke;
class QLottieBasicTransform;
class QLottieLayerTransform;
class QLottieShapeTransform;
class QLottieRepeaterTransform;
class QLottieShapeLayer;
class QLottieEllipse;
class QLottiePolyStar;
class QLottieRound;
class QLottieFreeFormShape;
class QLottieTrimPath;
class QLottieFillEffect;
class QLottieRepeater;

class Q_LOTTIE_EXPORT QLottieRenderer
{
public:
    enum TrimmingState { Off = 0, Parallel, Sequential };

    virtual ~QLottieRenderer() = default;

    virtual void saveState() = 0;
    virtual void restoreState() = 0;

    virtual void setTrimmingState(TrimmingState state);
    virtual TrimmingState trimmingState() const;

    virtual void render(const QLottieLayer &layer) = 0;
    virtual void render(const QLottieSolidLayer &layer) = 0;
    virtual void render(const QLottieGroup &) {}
    virtual void render(const QLottieRect &rect) = 0;
    virtual void render(const QLottieEllipse &ellipse) = 0;
    virtual void render(const QLottiePolyStar &star) = 0;
    virtual void render(const QLottieRound &round) = 0;
    virtual void render(const QLottieFill &fill) = 0;
    virtual void render(const QLottieGFill &fill) = 0;
    virtual void render(const QLottieImage &image) = 0;
    virtual void render(const QLottieStroke &stroke) = 0;
    virtual void render(const QLottieBasicTransform &trans) = 0;
    virtual void render(const QLottieShapeTransform &trans) = 0;
    virtual void render(const QLottieFreeFormShape &shape) = 0;
    virtual void render(const QLottieTrimPath &trans) = 0;
    virtual void render(const QLottieFillEffect &effect) = 0;
    virtual void render(const QLottieRepeater &repeater) = 0;
    virtual void renderPathElements(const QList<QLottieBase *> &pathElements) = 0;

    virtual void finish(const QLottieLayer &) {}
    virtual void finish(const QLottieGroup &) {}

protected:
    void saveTrimmingState();
    void restoreTrimmingState();
    static void applyTransform(QTransform *xf, const QLottieBasicTransform &lottieXf, bool isShapeTransform = false);

    TrimmingState m_trimmingState = Off;

private:
    QStack<QLottieRenderer::TrimmingState> m_trimStateStack;
};

QT_END_NAMESPACE

#endif // QLOTTIERENDERER_H
