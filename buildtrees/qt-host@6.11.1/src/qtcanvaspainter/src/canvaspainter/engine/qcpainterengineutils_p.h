// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCPAINTERENGINEUTILS_P_H
#define QCPAINTERENGINEUTILS_P_H

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

#include <QList>
#include <QVarLengthArray>
#include <QTransform>
#include <QElapsedTimer>
#include <QTimer>
#include <QObject>
#include <QDebug>
#include <QMetaEnum>
#include <QVariantMap>
#include "qcanvaspainter.h"
#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
#include "engine/qcrhidistancefieldglyphcache_p.h"
#endif

QT_BEGIN_NAMESPACE

class QCPainterRhiRenderer;
class QCanvasCustomBrush;
struct QCDebugCounters;

// Enable this to get performance logging outputs
//#define QCPAINTER_PERF_DEBUG

enum QCanvasBrushType {
    BrushColor,
    BrushLinearGradient,
    BrushRadialGradient,
    BrushConicalGradient,
    BrushBoxGradient,
    BrushBoxShadow,
    BrushImage,
    BrushGrid,
};

struct QCColor {
    float r, g, b, a;
};

struct QCPaint {
    QTransform transform;
    QCColor innerColor = {0.0f, 0.0f, 0.0f, 1.0f};
    QCColor outerColor = {0.0f, 0.0f, 0.0f, 1.0f};
    QCanvasBrushType brushType = BrushColor;
    int imageId = 0;
    float extent[2];
    float radius = 0.0f;
    float feather = 1.0f;
    float alpha = 1.0f;
};

struct QCDebugCounters {
    // Draw
    int fillDrawCallCount = 0;
    int strokeDrawCallCount = 0;
    int textDrawCallCount = 0;
    int fillTriangleCount = 0;
    int strokeTriangleCount = 0;
    int textTriangleCount = 0;
    // Total amounts
    int drawCallCount = 0;
    int triangleCount = 0;
    // Images
    qsizetype imageMemoryUsage = 0;
    int imageCount = 0;
    // Path caching
    int pathGroupCount = 0;
    int cachedSubpathCount = 0;
    int cachedPathVertexDataSize = 0;
};

enum QCCommand : quint8 {
    MoveTo,
    LineTo,
    BezierTo,
    Close,
    WindingCW,
    WindingCCW,
};
typedef QVarLengthArray<QCCommand> QCCommands;
typedef QVarLengthArray<float> QCCommandsData;

enum QCTextureFormat {
    TextureFormatAlpha,
    TextureFormatRGBA
};

struct QCClip {
    QTransform transform;
    float extent[2];
    QRectF rect;
};

struct QCVertex {
    float x, y, u, v;
};
typedef QVarLengthArray<QCVertex> QCVertices;

struct QCPath {
    // Offset and count of fill & stroke vertices in the vertex cache
    int fillOffset = 0;
    int fillCount = 0;
    int strokeOffset = 0;
    int strokeCount = 0;
    int pointsOffset = 0;
    int pointsCount = 0;
    int bevelCount = 0;
    QCanvasPainter::PathWinding winding = QCanvasPainter::PathWinding::CounterClockWise;
    bool isConvex = false;
    bool isClosed = false;
};
typedef QVarLengthArray<QCPath> QCPaths;

struct QCState {
    QTransform transform;
    QTransform brushTransform;
    QFont font;
    QCClip clip;
    QCPaint fill;
    QCPaint stroke;
    QCanvasCustomBrush *customFill = nullptr;
    QCanvasCustomBrush *customStroke = nullptr;
    float strokeWidth;
    float antialias;
    float miterLimit;
    float alpha;
    float brightness;
    float contrast;
    float saturate;
    float textLineHeight;
    float textAntialias;
    QCanvasPainter::CompositeOperation compositeOperation;
    QCanvasPainter::LineJoin lineJoin;
    QCanvasPainter::LineCap lineCap;
    QCanvasPainter::PathWinding winding;
    QCanvasPainter::WrapMode textWrapMode = QCanvasPainter::WrapMode::NoWrap;
    QCanvasPainter::TextAlign textAlignment = QCanvasPainter::TextAlign::Start;
    QCanvasPainter::TextBaseline textBaseline = QCanvasPainter::TextBaseline::Alphabetic;
    QCanvasPainter::TextDirection textDirection = QCanvasPainter::TextDirection::Inherit;
    bool blendEnable;
};

struct QCPoint {
    float x;
    float y;
    float dx;
    float dy;
    float len;
    float dmx;
    float dmy;
    quint8 flags;
};
typedef QVarLengthArray<QCPoint> QCPoints;

// Variables to determine if the QCanvasPath has changed so that paths, points &
// vertices need to be recreated.
struct QCCachedPath
{
    int pathGroup = -1;
    int pathIterations = -1;
    int commandsCount = 0;
};

// Variables to determine if related state has changed so that paths, points &
// vertices need to be recreated.
struct QCCachedPathFillProperties
{
    float antialias = 1.0f;
    int renderHints = 0;
};

inline bool operator==(const QCCachedPathFillProperties &a, const QCCachedPathFillProperties &b) noexcept
{
    return qFuzzyCompare(a.antialias, b.antialias)
           && a.renderHints == b.renderHints;
}

inline bool operator!=(const QCCachedPathFillProperties &a, const QCCachedPathFillProperties &b) noexcept
{
    return !(a == b);
}

inline size_t qHash(const QCCachedPathFillProperties &s, size_t seed) noexcept
{
    return qHash(s.antialias, seed) ^ s.renderHints;
}

// Variables to determine if related state has changed so that paths, points &
// vertices need to be recreated.
struct QCCachedPathStrokeProperties
{
    float antialias = 1.0f;
    float strokeWidth = 1.0f;
    QCanvasPainter::LineCap lineCap = QCanvasPainter::LineCap::Butt;
    QCanvasPainter::LineJoin lineJoin = QCanvasPainter::LineJoin::Miter;
    int renderHints = 0;
};

inline bool operator==(const QCCachedPathStrokeProperties &a, const QCCachedPathStrokeProperties &b) noexcept
{
    return qFuzzyCompare(a.antialias, b.antialias)
           && qFuzzyCompare(a.strokeWidth, b.strokeWidth)
           && a.lineCap == b.lineCap
           && a.lineJoin == b.lineJoin
           && a.renderHints == b.renderHints;
}

inline bool operator!=(const QCCachedPathStrokeProperties &a, const QCCachedPathStrokeProperties &b) noexcept
{
    return !(a == b);
}

inline size_t qHash(const QCCachedPathStrokeProperties &s, size_t seed) noexcept
{
    return qHash(s.antialias, seed) ^ qHash(s.strokeWidth) ^ int(s.lineCap) ^ int(s.lineJoin) ^ s.renderHints;
}

struct QCContext {
    QCCommands commands;
    QCCommandsData commandsData;
    QCPoints points;
    QCPaths paths;
    QCVertices vertices;
    // Currently active path, so paths[pathsCount - 1]
    QCPath *currentPath = nullptr;
    // Currently prepared painter path.
    // Means that current commands & commandsData are from this path.
    const QCanvasPath *preparedPath = nullptr;
    // Transform which was used for preparedPainterPath
    QTransform preparedPathTransform;
    QHash<QCanvasPath *, QCCachedPath> cachedFillPaths;
    QHash<QCanvasPath *, QCCachedPath> cachedStrokePaths;
    QList<QCState> states;
    QRectF view;
    QRectF bounds;
    QCanvasPainter::RenderHints renderHints = QCanvasPainter::RenderHint::Antialiasing;
    QMatrix4x4 customMatrix;
    QCDebugCounters debugCounters;
    float dpr;
    float divsTol;
    float tessTol;
    float distTol;
    float devicePxRatio;
    float prevX;
    float prevY;
    // Effectively used amounts
    int commandsCount = 0;
    int commandsDataCount = 0;
    int verticesCount = 0;
    int pointsCount = 0;
    int pathsCount = 0;
    int fontId;
    float fontAlphaMin;
    float fontAlphaMax;
    int preparedPathIterations = -1;
    int preparedPathCommandsCount = 0;
    bool customMatrixValid;
    bool antialiasingEnabled = true;
};

#ifdef QCPAINTER_PERF_DEBUG

// Collects data of out how much CPU time in ms different rendering steps takes:
// - PAINT: Whole painting inside render.
// - FILL: All fill calls combined.
// - STROKE: All stroke calls combined.
//
// Call count tells how many times those methods were called during the inteval.

class QCPerfLogging : public QObject {
    Q_OBJECT
public:
    enum QCPerfCategory {
        PAINT,
        FILL,
        STROKE,
        C2P,
        CLEAR,
    };
    Q_ENUM(QCPerfCategory)

    // Call this once, to start logging
    void start() {
        const int QCPAINTER_PERF_LOG_INTERVAL = 60000;
        const int QCPAINTER_PERF_LOG_WARMUP_TIME = 10000;
        m_loggingTimer.setInterval(QCPAINTER_PERF_LOG_INTERVAL);
        QObject::connect(&m_loggingTimer, &QTimer::timeout, this, &QCPerfLogging::print);
        QTimer::singleShot(QCPAINTER_PERF_LOG_WARMUP_TIME, this, [this] () {
            m_loggingTimer.start();
            m_perfTimer.start();
        });
    }

    // Call this once per frame
    void tick() {
        m_frameCount++;
    }

    void logStart(QCPerfCategory category)
    {
        m_logStart[category] = m_perfTimer.nsecsElapsed();
    }

    void logEnd(QCPerfCategory category)
    {
        if (m_logStart.contains(category)) {
            qint64 startns = m_logStart.value(category);
            qint64 timens = m_perfTimer.nsecsElapsed() - startns;
            m_log[category] += timens;
            m_logCount[category] = ++m_logCount[category];
        }
    }

    void print() {
        QString log;
        QString callsLog;
        for (auto i = m_log.cbegin(), end = m_log.cend(); i != end; ++i) {
            auto category = i.key();
            int callCount = m_logCount.value(category);
            int count = m_frameCount;
            qint64 totalns = i.value();
            double avgms = 0.0;
            if (count > 0)
                avgms = double(totalns * 0.000001) / count;
            QString avg = QString::number(avgms, 'f', 4);
            QMetaEnum metaEnum = QMetaEnum::fromType<QCPerfCategory>();
            log += QString(metaEnum.key(category)) + ": " + avg + " | ";
            callsLog += QString(metaEnum.key(category)) + ": " + QString::number(callCount) + " | ";
            m_logCount[category] = 0;
            m_log[category] = 0;
        }
        m_frameCount = 0;
        qDebug() << qPrintable("TIME: " + log);
        //qDebug() << qPrintable("CALLS:" + callsLog);
        //exit(0);
    }

private:
    QMap<QCPerfCategory, qint64> m_logStart;
    QMap<QCPerfCategory, qint64> m_log;
    QMap<QCPerfCategory, int> m_logCount;
    int m_frameCount = 0;
    QElapsedTimer m_perfTimer;
    QTimer m_loggingTimer;
};
#endif // QCPAINTER_PERF_DEBUG


// Length proportional to radius of a cubic bezier handle for 90deg arcs.
const float KAPPA90 = 0.5522847498f;
const float COMP_KAPPA90 = 1.0f - KAPPA90;

// Returns true if the distance between points (x1, y1) and (x2, y2)
// is less than tol, so they can be considered to be equal.
inline constexpr bool pointsEquals(float x1, float y1, float x2, float y2, float tol = 0.01f) noexcept
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return (dx * dx) + (dy * dy) < (tol * tol);
}

// Normalizes point (x, y) and returns the length.
inline float normalizePoint(float *x, float *y) noexcept
{
    float length = std::sqrt((*x)*(*x) + (*y)*(*y));
    if (length > 1e-6f) {
        const float invDist = 1.0f / length;
        *x *= invDist;
        *y *= invDist;
    }
    return length;
}

// Calculates cross product of (dx0, dy0) and (dx1, dy1).
inline constexpr float crossProduct(float dx0, float dy0, float dx1, float dy1) noexcept
{
    return (dx1 * dy0) - (dx0 * dy1);
}

// Returns true if point (x, y) distance from segment (px, py) - (qx, qy)
// is less than tol, so point can be condered to be inside the segment.
inline constexpr bool pointInSegment(float x, float y, float px, float py, float qx, float qy, float tol = 0.01f) noexcept
{
    float pqx = qx - px;
    float pqy = qy - py;
    float dx = x - px;
    float dy = y - py;
    float d = pqx * pqx + pqy * pqy;
    float t = pqx * dx + pqy * dy;
    if (d > 0)
        t /= d;
    t = qBound(0.0f, t, 1.0f);
    dx = px + t * pqx - x;
    dy = py + t * pqy - y;
    return (dx * dx) + (dy * dy) < (tol * tol);
}

// Returns sign (1.0 or -1.0) of the (a).
inline constexpr float sign(float a) noexcept
{
    return a >= 0.0f ? 1.0f : -1.0f;
}

// Returns the average scaling and shearing factor
// in vertically and horizontally of the transform.
inline float getAverageScale(const QTransform &transform) noexcept
{
    const float m1 = transform.m11();
    const float m2 = transform.m21();
    const float m3 = transform.m12();
    const float m4 = transform.m22();
    const float sx = std::sqrt(m1 * m1 + m2 * m2);
    const float sy = std::sqrt(m3 * m3 + m4 * m4);
    return (sx + sy) * 0.5f;
}

// Returns the amount of divisions a round curve at
// line width r and resolution of tol requires.
inline int curveDivs(float r, float tol = 0.25f)
{
    float da = std::acos(r / (r + tol)) * 2.0f;
    return std::max(2, (int)ceilf(float(M_PI) / da));
}

namespace QCanvasPainterDebugCounterUtils {

void fillDebugCounters(QVariantMap *dst, const QCDebugCounters &src);

} // namespace QQCanvasPainterDebugCounterUtils

QT_END_NAMESPACE

#endif // QCPAINTERENGINEUTILS_P_H
