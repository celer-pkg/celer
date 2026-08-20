// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvasbrush.h"
#include "qcanvasbrush_p.h"
#include "engine/qcpainterengineutils_p.h"
#include "qdatastream.h"
#include <qdebug.h>

QT_BEGIN_NAMESPACE

/*!
    \class QCanvasBrush
    \since 6.11
    \brief QCanvasBrush is the base class for all QCanvasPainter fill / stroke brushes.
    \inmodule QtCanvasPainter

    QCanvasBrush is the base class for all styles used for \l QCanvasPainter::fill()
    and \l QCanvasPainter::stroke().
*/

/*!
   Constructs an invalid brush.
*/

QCanvasBrush::QCanvasBrush()
{
}

/*!
    Constructs a brush that is a copy of the given \a brush.
*/
QCanvasBrush::QCanvasBrush(const QCanvasBrush &brush)
    : baseData(brush.baseData)
{
}

QCanvasBrush::~QCanvasBrush() = default;

/*!
    Assigns the given \a brush to this brush and returns a reference to
    this brush.
*/

QCanvasBrush &QCanvasBrush::operator=(const QCanvasBrush &brush)
{
    QCanvasBrush(brush).swap(*this);
    return *this;
}

/*!
    \fn QCanvasBrush::QCanvasBrush(QCanvasImage &&other) noexcept

    Move-constructs a new QCanvasBrush from \a other.
*/

/*!
    \fn QCanvasBrush &QCanvasBrush::operator=(QCanvasBrush &&other)

    Move-assigns \a other to this QCanvasBrush instance.
*/

/*!
    \fn void QCanvasBrush::swap(QCanvasBrush &other)
    \memberswap{brush}
*/


/*!
    \enum QCanvasBrush::BrushType

    Specifies the type of brush.

    \value Invalid - Empty brush.

    \value LinearGradient - Interpolates colors between start and end points
    (QCanvasLinearGradient)

    \value RadialGradient - Interpolates colors between a focal point and end
    points on a circle surrounding it (QCanvasRadialGradient).

    \value ConicalGradient - Interpolates colors around a center point
    (QCanvasConicalGradient).

    \value BoxGradient - Interpolates colors on a round rectangle
    (QCanvasBoxGradient).

    \value BoxShadow - Creates a soft round rectangle shadow (QCanvasBoxShadow).

    \value ImagePattern - Creates a pattern using the specified image and
    repetition (QCanvasImagePattern).

    \value GridPattern - Creates a pattern using the specified grid (QCanvasGridPattern).

    \value Custom - Creates a custom shader brush (QCanvasCustomBrush).

    \sa type()
*/

/*!
    Returns the type of the brush.
*/

QCanvasBrush::BrushType QCanvasBrush::type() const
{
    return baseData ? baseData->type : QCanvasBrush::BrushType::Invalid;
}

// ***** Private *****

/*!
   \internal
*/
void QCanvasBrush::detach()
{
    if (baseData)
        baseData.detach();
}

QCanvasBrush::QCanvasBrush(QCanvasBrushPrivate *priv)
    : baseData(priv)
{
}

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCanvasBrushPrivate);

/*!
   \internal
*/
QCPaint QCanvasBrush::createPaint(QCanvasPainter *painter) const
{
    Q_UNUSED(painter)
    if (baseData)
        return baseData->createPaint(painter);
    QCPaint empty;
    return empty;
}

#ifndef QT_NO_DEBUG_STREAM
/*!
  \internal
*/
QDebug operator<<(QDebug dbg, const QCanvasBrush &b)
{
    QDebugStateSaver saver(dbg);
    const auto t = b.type();
    dbg.nospace() << "QCanvasBrush(" << t << ")";
    return dbg;
}
#endif
QT_END_NAMESPACE
