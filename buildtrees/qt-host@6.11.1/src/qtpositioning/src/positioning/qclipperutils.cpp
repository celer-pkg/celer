// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qclipperutils_p.h"
#include <clip2tri.h>

QT_BEGIN_NAMESPACE

class QClipperUtilsPrivate
{
public:
    c2t::clip2tri m_clipper;
    Path m_cachedPolygon;
};

static const double kClipperScaleFactor = 281474976710656.0;  // 48 bits of precision
static const double kClipperScaleFactorInv = 1.0 / kClipperScaleFactor;

static IntPoint toIntPoint(const QDoubleVector2D &p)
{
    return IntPoint(cInt(p.x() * kClipperScaleFactor), cInt(p.y() * kClipperScaleFactor));
}

static QDoubleVector2D toVector2D(const IntPoint &p)
{
    return QDoubleVector2D(double(p.X) * kClipperScaleFactorInv, double(p.Y) * kClipperScaleFactorInv);
}

static QList<QDoubleVector2D> pathToQList(const Path &path)
{
    QList<QDoubleVector2D> res;
    res.reserve(int(path.size()));
    for (const IntPoint &ip: path)
        res.append(toVector2D(ip));
    return res;
}

static QList<QList<QDoubleVector2D>> pathsToQList(const Paths &paths)
{
    QList<QList<QDoubleVector2D> > res;
    res.reserve(int(paths.size()));
    for (const Path &p: paths) {
        res.append(pathToQList(p));
    }
    return res;
}

static Path qListToPath(const QList<QDoubleVector2D> &list)
{
    Path res;
    res.reserve(list.size());
    for (const QDoubleVector2D &p: list)
        res.push_back(toIntPoint(p));
    return res;
}

QClipperUtils::QClipperUtils() : d_ptr(new QClipperUtilsPrivate)
{
}

QClipperUtils::QClipperUtils(const QClipperUtils &other) : d_ptr(new QClipperUtilsPrivate)
{
    d_ptr->m_cachedPolygon = other.d_ptr->m_cachedPolygon;
}

QClipperUtils::~QClipperUtils()
{
    delete d_ptr;
}

double QClipperUtils::clipperScaleFactor()
{
    return kClipperScaleFactor;
}

int QClipperUtils::pointInPolygon(const QDoubleVector2D &point, const QList<QDoubleVector2D> &polygon)
{
    if (polygon.isEmpty())
        qWarning("No vertices are specified for the polygon!");
    return c2t::clip2tri::pointInPolygon(toIntPoint(point), qListToPath(polygon));
}

void QClipperUtils::clearClipper()
{
    d_ptr->m_clipper.clearClipper();
}

void QClipperUtils::addSubjectPath(const QList<QDoubleVector2D> &path, bool closed)
{
    d_ptr->m_clipper.addSubjectPath(qListToPath(path), closed);
}

void QClipperUtils::addClipPolygon(const QList<QDoubleVector2D> &path)
{
    d_ptr->m_clipper.addClipPolygon(qListToPath(path));
}

QList<QList<QDoubleVector2D>> QClipperUtils::execute(QClipperUtils::Operation op,
                                                     QClipperUtils::PolyFillType subjFillType,
                                                     QClipperUtils::PolyFillType clipFillType)
{
    auto result = d_ptr->m_clipper.execute(static_cast<c2t::clip2tri::Operation>(op),
                                           static_cast<QtClipperLib::PolyFillType>(subjFillType),
                                           static_cast<QtClipperLib::PolyFillType>(clipFillType));
    return pathsToQList(result);
}

void QClipperUtils::setPolygon(const QList<QDoubleVector2D> &polygon)
{
    d_ptr->m_cachedPolygon = qListToPath(polygon);
}

int QClipperUtils::pointInPolygon(const QDoubleVector2D &point) const
{
    if (d_ptr->m_cachedPolygon.empty())
        qWarning("No vertices are specified for the polygon!");
    return c2t::clip2tri::pointInPolygon(toIntPoint(point), d_ptr->m_cachedPolygon);
}

QT_END_NAMESPACE
