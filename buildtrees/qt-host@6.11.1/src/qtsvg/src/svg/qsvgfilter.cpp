// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qsvgfilter_p.h"

#include "qsvggraphics_p.h"
#include "qsvgnode_p.h"
#include "qsvgdocument_p.h"
#include "qpainter.h"

#include <QLoggingCategory>
#include <QtGui/qimageiohandler.h>
#include <QVector4D>

QT_BEGIN_NAMESPACE

QSvgFeFilterPrimitive::QSvgFeFilterPrimitive(QSvgNode *parent, const QString &input,
                                             const QString &result, const QSvgRectF &rect)
    : QSvgStructureNode(parent)
    , m_input(input)
    , m_result(result)
    , m_rect(rect)
{

}

bool QSvgFeFilterPrimitive::shouldDrawNode(QPainter *, QSvgExtraStates &) const
{
    return false;
}

QRectF QSvgFeFilterPrimitive::localSubRegion(const QRectF &itemBounds, const QRectF &filterBounds,
                                             QtSvg::UnitTypes primitiveUnits, QtSvg::UnitTypes) const
{
    // 15.7.3 Filter primitive subregion
    // https://www.w3.org/TR/SVG11/filters.html#FilterPrimitiveSubRegion

    QRectF clipRect = m_rect.resolveRelativeLengths(itemBounds, primitiveUnits);

    // the default subregion is 0%,0%,100%,100%, where as a special-case the percentages are
    // relative to the dimensions of the filter region, thus making the the default filter primitive
    // subregion equal to the filter region.
    if (m_rect.unitX() == QtSvg::UnitTypes::unknown)
        clipRect.setX(filterBounds.x());
    if (m_rect.unitY() == QtSvg::UnitTypes::unknown)
        clipRect.setY(filterBounds.y());
    if (m_rect.unitW() == QtSvg::UnitTypes::unknown)
        clipRect.setWidth(filterBounds.width());
    if (m_rect.unitH() == QtSvg::UnitTypes::unknown)
        clipRect.setHeight(filterBounds.height());

    clipRect = clipRect.intersected(filterBounds);

    return clipRect;
}

QRectF QSvgFeFilterPrimitive::globalSubRegion(QPainter *p,
                                              const QRectF &itemBounds, const QRectF &filterBounds,
                                              QtSvg::UnitTypes primitiveUnits, QtSvg::UnitTypes filterUnits) const
{
    return p->transform().mapRect(localSubRegion(itemBounds, filterBounds, primitiveUnits, filterUnits));
}

void QSvgFeFilterPrimitive::clipToTransformedBounds(QImage *buffer, QPainter *p, const QRectF &localRect) const
{
    QPainter painter(buffer);
    painter.setRenderHints(p->renderHints());
    painter.translate(-buffer->offset());
    QPainterPath clipPath;
    clipPath.setFillRule(Qt::OddEvenFill);
    clipPath.addRect(QRect(buffer->offset(), buffer->size()).adjusted(-10, -10, 20, 20));
    clipPath.addPolygon(p->transform().map(QPolygonF(localRect)));
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillPath(clipPath, Qt::transparent);
}

bool QSvgFeFilterPrimitive::requiresSourceAlpha() const
{
    return m_input == QLatin1StringView("SourceAlpha");
}

const QSvgFeFilterPrimitive *QSvgFeFilterPrimitive::castToFilterPrimitive(const QSvgNode *node)
{
    if (node->type() == QSvgNode::FeMerge ||
        node->type() == QSvgNode::FeColormatrix ||
        node->type() == QSvgNode::FeGaussianblur ||
        node->type() == QSvgNode::FeOffset ||
        node->type() == QSvgNode::FeComposite ||
        node->type() == QSvgNode::FeFlood ||
        node->type() == QSvgNode::FeBlend ) {
        return reinterpret_cast<const QSvgFeFilterPrimitive*>(node);
    } else {
        return nullptr;
    }
}

typedef QGenericMatrix<3, 3, qreal> Matrix3x3;

QSvgFeColorMatrix::QSvgFeColorMatrix(QSvgNode *parent, const QString &input, const QString &result,
                                     const QSvgRectF &rect, ColorShiftType type,
                                     const Matrix &matrix)
    : QSvgFeFilterPrimitive(parent, input, result, rect)
    , m_matrix(matrix)
{
    //Magic numbers see SVG 1.1(Second edition)
    constexpr qreal v1[] = {0.213, 0.213, 0.213,
                            0.715, 0.715, 0.715,
                            0.072, 0.072, 0.072};
    static const Matrix3x3 m1(v1);

    constexpr qreal v2[] = { 0.787, -0.213, -0.213,
                            -0.715,  0.285, -0.715,
                            -0.072, -0.072,  0.928};
    static const Matrix3x3 m2(v2);

    constexpr qreal v3[] = {-0.213,  0.143, -0.787,
                            -0.715,  0.140,  0.715,
                             0.928, -0.283,  0.072};
    static const Matrix3x3 m3(v3);

    if (type == ColorShiftType::Saturate) {
        qreal s = qBound(0., m_matrix.data()[0], 1.);

        m_matrix.fill(0);

        const Matrix3x3 m = m1 + m2 * s;

        for (int j = 0; j < 3; ++j)
            for (int i = 0; i < 3; ++i)
                m_matrix.data()[i+j*5] = m.data()[i+j*3];
        m_matrix.data()[3+3*5] = 1;

    } else if (type == ColorShiftType::HueRotate){
        qreal angle = m_matrix.data()[0]/180.*M_PI;
        qreal s = sin(angle);
        qreal c = cos(angle);

        m_matrix.fill(0);

        const Matrix3x3 m = m1 + m2 * c + m3 * s;

        for (int j = 0; j < 3; ++j)
            for (int i = 0; i < 3; ++i)
                m_matrix.data()[i+j*5] = m.data()[i+j*3];
        m_matrix.data()[3+3*5] = 1;
    } else if (type == ColorShiftType::LuminanceToAlpha){
        m_matrix.fill(0);

        m_matrix.data()[0+3*5] = 0.2125;
        m_matrix.data()[1+3*5] = 0.7154;
        m_matrix.data()[2+3*5] = 0.0721;
    }
}

QSvgNode::Type QSvgFeColorMatrix::type() const
{
    return QSvgNode::FeColormatrix;
}

QImage QSvgFeColorMatrix::apply(const QMap<QString, QImage> &sources, QPainter *p,
                                const QRectF &itemBounds, const QRectF &filterBounds,
                                QtSvg::UnitTypes primitiveUnits, QtSvg::UnitTypes filterUnits) const
{
    if (!sources.contains(m_input))
        return QImage();
    QImage source = sources[m_input];

    QRect clipRectGlob = globalSubRegion(p, itemBounds, filterBounds, primitiveUnits, filterUnits).toRect();
    if (clipRectGlob.isEmpty())
        return QImage();

    QImage result;
    if (!QImageIOHandler::allocateImage(clipRectGlob.size(), QImage::Format_ARGB32_Premultiplied, &result)) {
        qCWarning(lcSvgDraw) << "The requested filter buffer is too big, ignoring";
        return QImage();
    }
    result.setOffset(clipRectGlob.topLeft());
    result.fill(Qt::transparent);

    Q_ASSERT(source.depth() == 32);

    const Matrix transposedMatrix = m_matrix.transposed();

    for (int i = 0; i < result.height(); i++) {
        int sourceI = i - source.offset().y() + result.offset().y();

        if (sourceI < 0 || sourceI >= source.height())
            continue;

        QRgb *sourceLine = reinterpret_cast<QRgb *>(source.scanLine(sourceI));
        QRgb *resultLine = reinterpret_cast<QRgb *>(result.scanLine(i));

        for (int j = 0; j < result.width(); j++) {
            int sourceJ = j - source.offset().x() + result.offset().x();

            if (sourceJ < 0 || sourceJ >= source.width())
                continue;

            QRgb sourceColor = qUnpremultiply(sourceLine[sourceJ]);
            const qreal values[] = {qreal(qRed(sourceColor)),
                                    qreal(qGreen(sourceColor)),
                                    qreal(qBlue(sourceColor)),
                                    qreal(qAlpha(sourceColor)),
                                    255.};
            const QGenericMatrix<1, 5, qreal> sourceVector(values);
            const QGenericMatrix<1, 5, qreal> resultVector = transposedMatrix * sourceVector;

            QRgb rgba = qRgba(qBound(0, int(resultVector(0, 0)), 255),
                              qBound(0, int(resultVector(1, 0)), 255),
                              qBound(0, int(resultVector(2, 0)), 255),
                              qBound(0, int(resultVector(3, 0)), 255));
            resultLine[j] = qPremultiply(rgba);
        }
    }

    clipToTransformedBounds(&result, p, localSubRegion(itemBounds, filterBounds, primitiveUnits, filterUnits));
    return result;
}

QSvgFeGaussianBlur::QSvgFeGaussianBlur(QSvgNode *parent, const QString &input,
                                       const QString &result, const QSvgRectF &rect,
                                       qreal stdDeviationX, qreal stdDeviationY, EdgeMode edgemode)
    : QSvgFeFilterPrimitive(parent, input, result, rect)
    , m_stdDeviationX(stdDeviationX)
    , m_stdDeviationY(stdDeviationY)
    , m_edgemode(edgemode)
{
    Q_UNUSED(m_edgemode);
}

QSvgNode::Type QSvgFeGaussianBlur::type() const
{
    return QSvgNode::FeGaussianblur;
}

class ColorValues {
    // This class is not designed for general use. It is is only meant to improve the calling code's
    // readability by replacing a two-dimensial array of integers with a one-dimensial array of
    // objects which perform repeated calculations on the single values by just one call.
public:
    ColorValues() = default;
    explicit ColorValues(QRgb rgb) : b(qBlue(rgb)), g(qGreen(rgb)), r(qRed(rgb)), a(qAlpha(rgb)){}

    ColorValues &operator+=(const ColorValues &other)
    {
        b += other.b;
        g += other.g;
        r += other.r;
        a += other.a;
        return *this;
    }

    ColorValues operator+(const ColorValues &other)
    {
        ColorValues result(*this);
        result+=(other);
        return result;
    }

    ColorValues &operator-=(const ColorValues &other)
    {
        b -= other.b;
        g -= other.g;
        r -= other.r;
        a -= other.a;
        return *this;
    }

    ColorValues operator-(const ColorValues &other)
    {
        ColorValues result(*this);
        result -= other;
        return result;
    }

    ColorValues operator/=(uint64_t div)
    {
        // This does neither avoid nor handle division by zero. It
        // relies on the calling code to never pass a zero value.
        b /= div;
        g /= div;
        r /= div;
        a /= div;
        return *this;
    }

    QRgb toRgb()
    {
        return qRgba(r, g, b, a);
    }

private:
    uint64_t b = 0;
    uint64_t g = 0;
    uint64_t r = 0;
    uint64_t a = 0;
};

QImage QSvgFeGaussianBlur::apply(const QMap<QString, QImage> &sources, QPainter *p,
                                 const QRectF &itemBounds, const QRectF &filterBounds,
                                 QtSvg::UnitTypes primitiveUnits, QtSvg::UnitTypes filterUnits) const
{
    if (!sources.contains(m_input))
        return QImage();
    QImage source = sources[m_input];
    Q_ASSERT(source.depth() == 32);

    if (m_stdDeviationX == 0 && m_stdDeviationY == 0)
        return source;

    const qreal scaleX = qHypot(p->transform().m11(), p->transform().m21());
    const qreal scaleY = qHypot(p->transform().m12(), p->transform().m22());

    qreal sigma_x = scaleX * m_stdDeviationX;
    qreal sigma_y = scaleY * m_stdDeviationY;
    if (primitiveUnits == QtSvg::UnitTypes::objectBoundingBox) {
        sigma_x *= itemBounds.width();
        sigma_y *= itemBounds.height();
    }

    constexpr double sd = 3. * M_SQRT1_2 / M_2_SQRTPI; // 3 * sqrt(2 * pi) / 4
    const int dx = floor(sigma_x * sd + 0.5);
    const int dy = floor(sigma_y * sd + 0.5);

    const QTransform scaleXr = QTransform::fromScale(scaleX, scaleY);
    const QTransform restXr = scaleXr.inverted() * p->transform();

    QRect clipRectGlob = scaleXr.mapRect(localSubRegion(itemBounds, filterBounds, primitiveUnits, filterUnits)).toRect();
    if (clipRectGlob.isEmpty())
        return QImage();

    QImage tempSource;
    if (!QImageIOHandler::allocateImage(clipRectGlob.size(), QImage::Format_ARGB32_Premultiplied, &tempSource)) {
        qCWarning(lcSvgDraw) << "The requested filter buffer is too big, ignoring";
        return QImage();
    }
    tempSource.setOffset(clipRectGlob.topLeft());
    tempSource.fill(Qt::transparent);
    QPainter copyPainter(&tempSource);
    copyPainter.translate(-tempSource.offset());
    copyPainter.setTransform(restXr.inverted(), true);
    copyPainter.drawImage(source.offset(), source);
    copyPainter.end();

    QVarLengthArray<ColorValues, 32 * 32> buffer(tempSource.width() * tempSource.height());

    const int sourceHeight = tempSource.height();
    const int sourceWidth = tempSource.width();
    QRgb *rawImage = reinterpret_cast<QRgb *>(tempSource.bits());

    // https://www.w3.org/TR/SVG11/filters.html#feGaussianBlurElement:
    // Three successive box-blurs build a piece-wise quadratic convolution kernel,
    // which approximates the Gaussian kernel
    for (int m = 0; m < 3; m++) {
        // https://www.w3.org/TR/SVG11/filters.html#feGaussianBlurElement:
        // if d is odd, use three box-blurs of size 'd', centered on the output pixel.
        // if d is even, two box-blurs of size 'd' (the first one centered on the pixel boundary
        // between the output pixel and the one to the left, the second one centered on the pixel
        // boundary between the output pixel and the one to the right) and one box blur of size
        // 'd+1' centered on the output pixel.
        auto adjustD = [](int d, int iteration) {
            d = qMax(1, d);     // Treat d == 0 just like d == 1
            std::pair<int, int> result;
            if (d % 2 == 1)
                result = {d / 2 + 1, d / 2};
            else if (iteration == 0)
                result = {d / 2 + 1, d / 2 - 1};
            else if (iteration == 1)
                result = {d / 2, d / 2};
            else
                result = {d / 2 + 1, d / 2};
            Q_ASSERT(result.first + result.second > 0);
            return result;
        };

        const auto [dxleft, dxright] = adjustD(dx, m);
        const auto [dytop, dybottom] = adjustD(dy, m);

        // Generating the partial sum of color values from the top left corner
        // These sums can be combined to yield the partial sum of any rectangular subregion
        for (int j = 0; j < sourceHeight; j++) {
            for (int i = 0; i < sourceWidth; i++) {
                ColorValues colorValues(rawImage[i + j * sourceWidth]);
                if (i > 0)
                    colorValues += buffer[(i - 1) + j * sourceWidth];
                if (j > 0)
                    colorValues += buffer[i + (j - 1) * sourceWidth];
                if (i > 0 && j > 0)
                    colorValues -= buffer[(i - 1) + (j - 1) * sourceWidth];
                buffer[i + j * sourceWidth] = colorValues;
            }
        }

        for (int j = 0; j < sourceHeight; j++) {
            const int j1 = qMax(0, j - dytop);
            const int j2 = qMin(sourceHeight - 1, j + dybottom);
            for (int i = 0; i < sourceWidth; i++) {
                const int i1 = qMax(0, i - dxleft);
                const int i2 = qMin(sourceWidth - 1, i + dxright);
                ColorValues colorValues =   buffer[i2 + j2 * sourceWidth]
                                          - buffer[i1 + j2 * sourceWidth]
                                          - buffer[i2 + j1 * sourceWidth]
                                          + buffer[i1 + j1 * sourceWidth];
                colorValues /= uint64_t(dxleft + dxright) * uint64_t(dytop + dybottom);
                rawImage[i + j * sourceWidth] = colorValues.toRgb();
            }
        }
    }

    QRectF trueClipRectGlob = globalSubRegion(p, itemBounds, filterBounds, primitiveUnits, filterUnits);

    QImage result;
    if (!QImageIOHandler::allocateImage(trueClipRectGlob.toRect().size(), QImage::Format_ARGB32_Premultiplied, &result)) {
        qCWarning(lcSvgDraw) << "The requested filter buffer is too big, ignoring";
        return QImage();
    }
    result.setOffset(trueClipRectGlob.toRect().topLeft());
    result.fill(Qt::transparent);
    QPainter transformPainter(&result);
    transformPainter.setRenderHint(QPainter::Antialiasing, true);

    transformPainter.translate(-result.offset());
    transformPainter.setTransform(restXr, true);
    transformPainter.drawImage(clipRectGlob.topLeft(), tempSource);
    transformPainter.end();

    clipToTransformedBounds(&result, p, localSubRegion(itemBounds, filterBounds, primitiveUnits, filterUnits));
    return result;
}

QSvgFeOffset::QSvgFeOffset(QSvgNode *parent, const QString &input, const QString &result,
                           const QSvgRectF &rect, qreal dx, qreal dy)
    : QSvgFeFilterPrimitive(parent, input, result, rect)
    , m_dx(dx)
    , m_dy(dy)
{

}

QSvgNode::Type QSvgFeOffset::type() const
{
    return QSvgNode::FeOffset;
}

QImage QSvgFeOffset::apply(const QMap<QString, QImage> &sources, QPainter *p,
                           const QRectF &itemBounds, const QRectF &filterBounds,
                           QtSvg::UnitTypes primitiveUnits, QtSvg::UnitTypes filterUnits) const
{
    if (!sources.contains(m_input))
        return QImage();

    const QImage &source = sources[m_input];

    QRectF clipRect = localSubRegion(itemBounds, filterBounds, primitiveUnits, filterUnits);
    QRect clipRectGlob = p->transform().mapRect(clipRect).toRect();

    QPoint offset(m_dx, m_dy);
    if (primitiveUnits == QtSvg::UnitTypes::objectBoundingBox) {
        offset = QPoint(m_dx * itemBounds.width(),
                        m_dy * itemBounds.height());
    }
    offset = p->transform().map(offset) - p->transform().map(QPoint(0, 0));

    if (clipRectGlob.isEmpty())
        return QImage();

    QImage result;
    if (!QImageIOHandler::allocateImage(clipRectGlob.size(), QImage::Format_ARGB32_Premultiplied, &result)) {
        qCWarning(lcSvgDraw) << "The requested filter buffer is too big, ignoring";
        return QImage();
    }
    result.setOffset(clipRectGlob.topLeft());
    result.fill(Qt::transparent);

    QPainter copyPainter(&result);
    copyPainter.drawImage(source.offset()
                        - result.offset() + offset, source);
    copyPainter.end();

    clipToTransformedBounds(&result, p, clipRect);
    return result;
}


QSvgFeMerge::QSvgFeMerge(QSvgNode *parent, const QString &input,
                         const QString &result, const QSvgRectF &rect)
    : QSvgFeFilterPrimitive(parent, input, result, rect)
{

}

QSvgNode::Type QSvgFeMerge::type() const
{
    return QSvgNode::FeMerge;
}

QImage QSvgFeMerge::apply(const QMap<QString, QImage> &sources, QPainter *p,
                          const QRectF &itemBounds, const QRectF &filterBounds,
                          QtSvg::UnitTypes primitiveUnits, QtSvg::UnitTypes filterUnits) const
{
    QList<QImage> mergeNodeResults;
    for (const auto &node : renderers()) {
        if (node->type() == QSvgNode::FeMergenode) {
            const QSvgFeMergeNode &filter = static_cast<const QSvgFeMergeNode&>(*node);
            mergeNodeResults.append(filter.apply(sources, p, itemBounds, filterBounds, primitiveUnits, filterUnits));
        }
    }

    QRectF clipRect = localSubRegion(itemBounds, filterBounds, primitiveUnits, filterUnits);
    QRect clipRectGlob = p->transform().mapRect(clipRect).toRect();
    if (clipRectGlob.isEmpty())
        return QImage();

    QImage result;
    if (!QImageIOHandler::allocateImage(clipRectGlob.size(), QImage::Format_ARGB32_Premultiplied, &result)) {
        qCWarning(lcSvgDraw) << "The requested filter buffer is too big, ignoring";
        return QImage();
    }
    result.setOffset(clipRectGlob.topLeft());
    result.fill(Qt::transparent);

    QPainter proxyPainter(&result);
    for (const QImage &i : mergeNodeResults) {
        proxyPainter.drawImage(QRect(i.offset() - result.offset(), i.size()), i);
    }
    proxyPainter.end();

    clipToTransformedBounds(&result, p, clipRect);
    return result;
}

bool QSvgFeMerge::requiresSourceAlpha() const
{
    for (const auto &node : renderers()) {
        if (node->type() == QSvgNode::FeMergenode) {
            const QSvgFeMergeNode &filter = static_cast<const QSvgFeMergeNode&>(*node);
            if (filter.requiresSourceAlpha())
                return true;
        }
    }
    return false;
}

QSvgFeMergeNode::QSvgFeMergeNode(QSvgNode *parent, const QString &input,
                                 const QString &result, const QSvgRectF &rect)
    : QSvgFeFilterPrimitive(parent, input, result, rect)
{

}

QSvgNode::Type QSvgFeMergeNode::type() const
{
    return QSvgNode::FeMergenode;
}

QImage QSvgFeMergeNode::apply(const QMap<QString, QImage> &sources, QPainter *,
                              const QRectF &, const QRectF &, QtSvg::UnitTypes, QtSvg::UnitTypes) const
{
    return sources.value(m_input);
}

QSvgFeComposite::QSvgFeComposite(QSvgNode *parent, const QString &input, const QString &result,
                                 const QSvgRectF &rect, const QString &input2, Operator op,
                                 const QVector4D &k)
    : QSvgFeFilterPrimitive(parent, input, result, rect)
    , m_input2(input2)
    , m_operator(op)
    , m_k(k)
{

}

QSvgNode::Type QSvgFeComposite::type() const
{
    return QSvgNode::FeComposite;
}

QImage QSvgFeComposite::apply(const QMap<QString, QImage> &sources, QPainter *p,
                              const QRectF &itemBounds, const QRectF &filterBounds,
                              QtSvg::UnitTypes primitiveUnits, QtSvg::UnitTypes filterUnits) const
{
    if (!sources.contains(m_input))
        return QImage();
    if (!sources.contains(m_input2))
        return QImage();
    QImage source1 = sources[m_input];
    QImage source2 = sources[m_input2];
    Q_ASSERT(source1.depth() == 32);
    Q_ASSERT(source2.depth() == 32);

    QRectF clipRect = localSubRegion(itemBounds, filterBounds, primitiveUnits, filterUnits);
    QRect clipRectGlob = p->transform().mapRect(clipRect).toRect();

    if (clipRectGlob.isEmpty())
        return QImage();

    QImage result;
    if (!QImageIOHandler::allocateImage(clipRectGlob.size(), QImage::Format_ARGB32_Premultiplied, &result)) {
        qCWarning(lcSvgDraw) << "The requested filter buffer is too big, ignoring";
        return QImage();
    }
    result.setOffset(clipRectGlob.topLeft());
    result.fill(Qt::transparent);

    if (m_operator == Operator::Arithmetic) {
        const qreal k1 = m_k.x();
        const qreal k2 = m_k.y();
        const qreal k3 = m_k.z();
        const qreal k4 = m_k.w();

        for (int j = 0; j < result.height(); j++) {
            int jj1 = j - source1.offset().y() + result.offset().y();
            int jj2 = j - source2.offset().y() + result.offset().y();

            QRgb *resultLine = reinterpret_cast<QRgb *>(result.scanLine(j));
            QRgb *source1Line = nullptr;
            QRgb *source2Line = nullptr;

            if (jj1 >= 0 && jj1 < source1.size().height())
                source1Line = reinterpret_cast<QRgb *>(source1.scanLine(jj1));
            if (jj2 >= 0 && jj2 < source2.size().height())
                 source2Line = reinterpret_cast<QRgb *>(source2.scanLine(jj2));

            for (int i = 0; i < result.width(); i++) {
                int ii1 = i - source1.offset().x() + result.offset().x();
                int ii2 = i - source2.offset().x() + result.offset().x();

                QVector4D s1 = QVector4D(0, 0, 0, 0);
                QVector4D s2 = QVector4D(0, 0, 0, 0);

                if (ii1 >= 0 && ii1 < source1.size().width() && source1Line) {
                    QRgb pixel1 = source1Line[ii1];
                    s1 = QVector4D(qRed(pixel1),
                                   qGreen(pixel1),
                                   qBlue(pixel1),
                                   qAlpha(pixel1));
                }

                if (ii2 >= 0 && ii2 < source2.size().width() && source2Line) {
                    QRgb pixel2 = source2Line[ii2];
                    s2 = QVector4D(qRed(pixel2),
                                   qGreen(pixel2),
                                   qBlue(pixel2),
                                   qAlpha(pixel2));
                }

                int r = k1 * s1.x() * s2.x() / 255. + k2 * s1.x() + k3 * s2.x() + k4 * 255.;
                int g = k1 * s1.y() * s2.y() / 255. + k2 * s1.y() + k3 * s2.y() + k4 * 255.;
                int b = k1 * s1.z() * s2.z() / 255. + k2 * s1.z() + k3 * s2.z() + k4 * 255.;
                int a = k1 * s1.w() * s2.w() / 255. + k2 * s1.w() + k3 * s2.w() + k4 * 255.;

                a = qBound(0, a, 255);
                resultLine[i] =  qRgba(qBound(0, r, a),
                                       qBound(0, g, a),
                                       qBound(0, b, a),
                                       a);
            }
        }
    } else {
        QPainter proxyPainter(&result);
        proxyPainter.drawImage(QRect(source1.offset() - result.offset(), source1.size()), source1);

        switch (m_operator) {
        case Operator::In:
            proxyPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            break;
        case Operator::Out:
            proxyPainter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            break;
        case Operator::Xor:
            proxyPainter.setCompositionMode(QPainter::CompositionMode_Xor);
            break;
        case Operator::Lighter:
            proxyPainter.setCompositionMode(QPainter::CompositionMode_Lighten);
            break;
        case Operator::Atop:
            proxyPainter.setCompositionMode(QPainter::CompositionMode_DestinationAtop);
            break;
        case Operator::Over:
            proxyPainter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
            break;
        case Operator::Arithmetic: // handled above
            Q_UNREACHABLE();
            break;
        }
        proxyPainter.drawImage(QRect(source2.offset()-result.offset(), source2.size()), source2);
        proxyPainter.end();
    }

    clipToTransformedBounds(&result, p, clipRect);
    return result;
}

bool QSvgFeComposite::requiresSourceAlpha() const
{
    if (QSvgFeFilterPrimitive::requiresSourceAlpha())
        return true;
    return m_input2 == QLatin1StringView("SourceAlpha");
}


QSvgFeFlood::QSvgFeFlood(QSvgNode *parent, const QString &input, const QString &result,
                         const QSvgRectF &rect, const QColor &color)
    : QSvgFeFilterPrimitive(parent, input, result, rect)
    , m_color(color)
{

}

QSvgNode::Type QSvgFeFlood::type() const
{
    return QSvgNode::FeFlood;
}

QImage QSvgFeFlood::apply(const QMap<QString, QImage> &,
                          QPainter *p, const QRectF &itemBounds, const QRectF &filterBounds,
                          QtSvg::UnitTypes primitiveUnits, QtSvg::UnitTypes filterUnits) const
{

    QRectF clipRect = localSubRegion(itemBounds, filterBounds, primitiveUnits, filterUnits);
    QRect clipRectGlob = p->transform().mapRect(clipRect).toRect();

    QImage result;
    if (!QImageIOHandler::allocateImage(clipRectGlob.size(), QImage::Format_ARGB32_Premultiplied, &result)) {
        qCWarning(lcSvgDraw) << "The requested filter buffer is too big, ignoring";
        return QImage();
    }
    result.setOffset(clipRectGlob.topLeft());
    result.fill(m_color);

    clipToTransformedBounds(&result, p, clipRect);
    return result;
}

QSvgFeBlend::QSvgFeBlend(QSvgNode *parent, const QString &input, const QString &result,
                         const QSvgRectF &rect, const QString &input2, Mode mode)
    : QSvgFeFilterPrimitive(parent, input, result, rect)
      , m_input2(input2)
      , m_mode(mode)
{

}

QSvgNode::Type QSvgFeBlend::type() const
{
    return QSvgNode::FeBlend;
}

QImage QSvgFeBlend::apply(const QMap<QString, QImage> &sources, QPainter *p,
                          const QRectF &itemBounds, const QRectF &filterBounds,
                          QtSvg::UnitTypes primitiveUnits, QtSvg::UnitTypes filterUnits) const
{
    if (!sources.contains(m_input))
        return QImage();
    if (!sources.contains(m_input2))
        return QImage();
    QImage source1 = sources[m_input];
    QImage source2 = sources[m_input2];
    Q_ASSERT(source1.depth() == 32);
    Q_ASSERT(source2.depth() == 32);

    QRectF clipRect = localSubRegion(itemBounds, filterBounds, primitiveUnits, filterUnits);
    QRect clipRectGlob = p->transform().mapRect(clipRect).toRect();

    QImage result;
    if (!QImageIOHandler::allocateImage(clipRectGlob.size(), QImage::Format_ARGB32_Premultiplied, &result)) {
        qCWarning(lcSvgDraw) << "The requested filter buffer is too big, ignoring";
        return QImage();
    }
    result.setOffset(clipRectGlob.topLeft());
    result.fill(Qt::transparent);

    for (int j = 0; j < result.height(); j++) {
        int jj1 = j - source1.offset().y() + result.offset().y();
        int jj2 = j - source2.offset().y() + result.offset().y();

        QRgb *resultLine = reinterpret_cast<QRgb *>(result.scanLine(j));
        QRgb *source1Line = nullptr;
        QRgb *source2Line = nullptr;

        if (jj1 >= 0 && jj1 < source1.size().height())
            source1Line = reinterpret_cast<QRgb *>(source1.scanLine(jj1));
        if (jj2 >= 0 && jj2 < source2.size().height())
            source2Line = reinterpret_cast<QRgb *>(source2.scanLine(jj2));

        for (int i = 0; i < result.width(); i++) {
            int ii1 = i - source1.offset().x() + result.offset().x();
            int ii2 = i - source2.offset().x() + result.offset().x();

            QRgb pixel1 = (ii1 >= 0 && ii1 < source1.size().width() && source1Line) ?
                    source1Line[ii1] : qRgba(0, 0, 0, 0);
            QRgb pixel2 = (ii2 >= 0 && ii2 < source2.size().width() && source2Line) ?
                    source2Line[ii2] : qRgba(0, 0, 0, 0);

            qreal r = 0, g = 0, b = 0;
            qreal red1 = qRed(pixel1);
            qreal red2 = qRed(pixel2);
            qreal green1 = qGreen(pixel1);
            qreal green2 = qGreen(pixel2);
            qreal blue1 = qBlue(pixel1);
            qreal blue2 = qBlue(pixel2);
            qreal alpha1 = qAlpha(pixel1) / 255.;
            qreal alpha2 = qAlpha(pixel2) / 255.;
            qreal a = 255 - (1 - alpha1) * (1 - alpha2) * 255.;

            switch (m_mode) {
            case Mode::Normal:
                r = (1 - alpha1) * red2 + red1;
                g = (1 - alpha1) * green2 + green1;
                b = (1 - alpha1) * blue2 + blue1;
                break;
            case Mode::Multiply:
                r = (1 - alpha1) * red2 + (1 - alpha2) * red1 + red1 * red2 / 255.;
                g = (1 - alpha1) * green2 + (1 - alpha2) * green1 + green1 * green2 / 255.;
                b = (1 - alpha1) * blue2 + (1 - alpha2) * blue1 + blue1 * blue2 / 255.;
                break;
            case Mode::Screen:
                r = red2 + red1 - red1 * red2 / 255.;
                g = green2 + green1 - green1 * green2 / 255.;
                b = blue2 + blue1 - blue1 * blue2 / 255.;
                break;
            case Mode::Darken:
                r = qMin((1 - alpha1) * red2 + red1, (1 - alpha2) * red1 + red2);
                g = qMin((1 - alpha1) * green2 + green1, (1 - alpha2) * green1 + green2);
                b = qMin((1 - alpha1) * blue2 + blue1, (1 - alpha2) * blue1 + blue2);
                break;
            case Mode::Lighten:
                r = qMax((1 - alpha1) * red2 + red1, (1 - alpha2) * red1 + red2);
                g = qMax((1 - alpha1) * green2 + green1, (1 - alpha2) * green1 + green2);
                b = qMax((1 - alpha1) * blue2 + blue1, (1 - alpha2) * blue1 + blue2);
                break;
            }
            a = qBound(0., a, 255.);
            resultLine[i] = qRgba(qBound(0., r, a),
                                  qBound(0., g, a),
                                  qBound(0., b, a),
                                  a);
        }
    }
    clipToTransformedBounds(&result, p, clipRect);
    return result;
}

bool QSvgFeBlend::requiresSourceAlpha() const
{
    if (QSvgFeFilterPrimitive::requiresSourceAlpha())
        return true;
    return m_input2 == QLatin1StringView("SourceAlpha");
}

QSvgFeUnsupported::QSvgFeUnsupported(QSvgNode *parent, const QString &input, const QString &result,
                         const QSvgRectF &rect)
    : QSvgFeFilterPrimitive(parent, input, result, rect)
{
}

QSvgNode::Type QSvgFeUnsupported::type() const
{
    return QSvgNode::FeUnsupported;
}

QImage QSvgFeUnsupported::apply(const QMap<QString, QImage> &,
                          QPainter *, const QRectF &, const QRectF &,
                          QtSvg::UnitTypes, QtSvg::UnitTypes) const
{
    qCDebug(lcSvgDraw) << "Unsupported filter primitive should not be applied.";
    return QImage();
}

QT_END_NAMESPACE
