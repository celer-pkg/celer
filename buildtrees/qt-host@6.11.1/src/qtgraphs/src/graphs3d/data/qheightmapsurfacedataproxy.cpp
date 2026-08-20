// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtCore/qdebug.h>
#include <QtCore/qfileinfo.h>
#include "qheightmapsurfacedataproxy_p.h"
#include "qsurface3dseries_p.h"
#include "qgraphs3dlogging_p.h"

#include <qtgraphs_tracepoints_p.h>

QT_BEGIN_NAMESPACE

Q_TRACE_PREFIX(qtgraphs,
                   "QT_BEGIN_NAMESPACE" \
                   "class QHeightMapSurfaceDataProxy;" \
                   "QT_END_NAMESPACE"
               )

Q_TRACE_POINT(qtgraphs, QGraphs3DHeightMapSurfaceDataProxHandlePendingResolve_entry, bool is16Bit);
Q_TRACE_POINT(qtgraphs, QGraphs3DHeightMapSurfaceDataProxHandlePendingResolve_exit);

// Default ranges correspond value axis defaults
const float defaultMinValue = 0.0f;
const float defaultMaxValue = 10.0f;

/*!
 * \class QHeightMapSurfaceDataProxy
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief Base proxy class for Q3DSurfaceWidgetItem.
 *
 * QHeightMapSurfaceDataProxy takes care of the processing of height map data
 * related to surfaces. It provides the visualization of a height map as a
 * surface plot.
 *
 * Since height maps do not contain values for X or Z axes, those values need to
 * be given separately using the minXValue, maxXValue, minZValue, and maxZValue
 * properties. The X-value corresponds to image horizontal direction and the Z-value
 * to the vertical. Setting any of these properties triggers an asynchronous
 * re-resolution of any existing height map.
 *
 * \sa QSurfaceDataProxy, {Qt Graphs Data Handling with 3D}
 */

/*!
 * \qmltype HeightMapSurfaceDataProxy
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \nativetype QHeightMapSurfaceDataProxy
 * \inherits SurfaceDataProxy
 * \brief Base proxy type for Surface3D.
 *
 * QHeightMapSurfaceDataProxy takes care of the processing of height map data
 * related to surfaces. It provides the visualization of a height map as a
 * surface plot.
 *
 * For more complete description, see QHeightMapSurfaceDataProxy.
 *
 * \sa {Qt Graphs Data Handling with 3D}
 */

/*!
 * \qmlproperty string HeightMapSurfaceDataProxy::heightMapFile
 *
 * A file with a height map image to be visualized. Setting this property
 * replaces current data with height map data.
 *
 * There are several formats the image file can be given in, but if it is not in
 * a directly usable format, a conversion is made.
 *
 * \note If the result seems wrong, the automatic conversion failed
 * and you should try converting the image yourself before setting it. Preferred
 * format is QImage::Format_RGB32 in grayscale.
 *
 * The height of the image is read from the red component of the pixels if the
 * image is in grayscale. Otherwise, it is an average calculated from the red, green,
 * and blue components of the pixels. Using grayscale images may improve data
 * conversion speed for large images.
 *
 * Since height maps do not contain values for X or Z axes, these values need to
 * be given separately using the minXValue, maxXValue, minZValue, and maxZValue
 * properties. The X-value corresponds to the image's horizontal direction,
 * and the Z-value to the vertical. Setting any of these properties triggers
 * an asynchronous re-resolution of any existing height map.
 *
 * Not recommended formats: all mono formats (for example QImage::Format_Mono).
 */

/*!
 * \qmlproperty real HeightMapSurfaceDataProxy::minXValue
 *
 * The minimum X value for the generated surface points. Defaults to \c{0.0}.
 * When setting this property the corresponding maximum value is adjusted if
 * necessary, to ensure that the range remains valid.
 */

/*!
 * \qmlproperty real HeightMapSurfaceDataProxy::maxXValue
 *
 * The maximum X value for the generated surface points. Defaults to \c{10.0}.
 * When setting this property the corresponding minimum value is adjusted if
 * necessary, to ensure that the range remains valid.
 */

/*!
 * \qmlproperty real HeightMapSurfaceDataProxy::minZValue
 *
 * The minimum Z value for the generated surface points. Defaults to \c{0.0}.
 * When setting this property the corresponding maximum value is adjusted if
 * necessary, to ensure that the range remains valid.
 */

/*!
 * \qmlproperty real HeightMapSurfaceDataProxy::maxZValue
 *
 * The maximum Z value for the generated surface points. Defaults to \c{10.0}.
 * When setting this property the corresponding minimum value is adjusted if
 * necessary, to ensure that the range remains valid.
 */

/*!
 * \qmlproperty real HeightMapSurfaceDataProxy::minYValue
 *
 * The minimum Y value for the generated surface points. Defaults to \c{0.0}.
 * When setting this property the corresponding maximum value is adjusted if
 * necessary, to ensure that the range remains valid.
 */

/*!
 * \qmlproperty real HeightMapSurfaceDataProxy::maxYValue
 *
 * The maximum Y value for the generated surface points. Defaults to \c{10.0}.
 * When setting this property the corresponding minimum value is adjusted if
 * necessary, to ensure that the range remains valid.
 */

/*!
 * \qmlproperty real HeightMapSurfaceDataProxy::autoScaleY
 *
 * Scale height values to Y-axis. Defaults to \c{false}. When this property is
 * set to \c{true}, the height values are scaled to fit on the Y-axis between
 * \c{minYValue} and \c{maxYValue}.
 */

/*!
    \qmlsignal HeightMapSurfaceDataProxy::heightMapFileChanged(string filename)

    This signal is emitted when heightMapFile changes to \a filename.
*/

/*!
    \qmlsignal HeightMapSurfaceDataProxy::minXValueChanged(real value)

    This signal is emitted when minXValue changes to \a value.
*/

/*!
    \qmlsignal HeightMapSurfaceDataProxy::maxXValueChanged(real value)

    This signal is emitted when maxXValue changes to \a value.
*/

/*!
    \qmlsignal HeightMapSurfaceDataProxy::minZValueChanged(real value)

    This signal is emitted when minZValue changes to \a value.
*/

/*!
    \qmlsignal HeightMapSurfaceDataProxy::maxZValueChanged(real value)

    This signal is emitted when maxZValue changes to \a value.
*/

/*!
    \qmlsignal HeightMapSurfaceDataProxy::minYValueChanged(real value)

    This signal is emitted when minYValue changes to \a value.
*/

/*!
    \qmlsignal HeightMapSurfaceDataProxy::maxYValueChanged(real value)

    This signal is emitted when maxYValue changes to \a value.
*/

/*!
    \qmlsignal HeightMapSurfaceDataProxy::autoScaleYChanged(bool enabled)

    This signal is emitted when autoScaleY changes to \a enabled.
*/

/*!
 * Constructs QHeightMapSurfaceDataProxy with the given \a parent.
 */
QHeightMapSurfaceDataProxy::QHeightMapSurfaceDataProxy(QObject *parent)
    : QSurfaceDataProxy(*(new QHeightMapSurfaceDataProxyPrivate()), parent)
{
    Q_D(QHeightMapSurfaceDataProxy);
    QObject::connect(&d->m_resolveTimer,
                     &QTimer::timeout,
                     this,
                     &QHeightMapSurfaceDataProxy::handlePendingResolve);
}

/*!
 * Constructs QHeightMapSurfaceDataProxy with the given \a image and \a parent.
 * Height map is set by calling setHeightMap() with \a image.
 *
 * \sa heightMap
 */
QHeightMapSurfaceDataProxy::QHeightMapSurfaceDataProxy(const QImage &image, QObject *parent)
    : QSurfaceDataProxy(*(new QHeightMapSurfaceDataProxyPrivate()), parent)
{
    Q_D(QHeightMapSurfaceDataProxy);
    QObject::connect(&d->m_resolveTimer,
                     &QTimer::timeout,
                     this,
                     &QHeightMapSurfaceDataProxy::handlePendingResolve);
    setHeightMap(image);
}

/*!
 * Constructs QHeightMapSurfaceDataProxy from the given image \a filename and \a
 * parent. Height map is set by calling setHeightMapFile() with \a filename.
 *
 * \sa heightMapFile
 */
QHeightMapSurfaceDataProxy::QHeightMapSurfaceDataProxy(const QString &filename, QObject *parent)
    : QSurfaceDataProxy(*(new QHeightMapSurfaceDataProxyPrivate()), parent)
{
    Q_D(QHeightMapSurfaceDataProxy);
    QObject::connect(&d->m_resolveTimer,
                     &QTimer::timeout,
                     this,
                     &QHeightMapSurfaceDataProxy::handlePendingResolve);
    setHeightMapFile(filename);
}

/*!
 * \internal
 */
QHeightMapSurfaceDataProxy::QHeightMapSurfaceDataProxy(QHeightMapSurfaceDataProxyPrivate &d,
                                                       QObject *parent)
    : QSurfaceDataProxy(d, parent)
{}

/*!
 * Destroys QHeightMapSurfaceDataProxy.
 */
QHeightMapSurfaceDataProxy::~QHeightMapSurfaceDataProxy() {}

/*!
 * \property QHeightMapSurfaceDataProxy::heightMap
 *
 * \brief The height map image to be visualized.
 */

/*!
 * Replaces current data with the height map data specified by \a image.
 *
 * There are several formats the \a image can be given in, but if it is not in a
 * directly usable format, a conversion is made.
 *
 * \note If the result seems wrong, the automatic conversion failed
 * and you should try converting the \a image yourself before setting it.
 * Preferred format is QImage::Format_RGB32 in grayscale.
 *
 * The height of the \a image is read from the red component of the pixels if
 * the \a image is in grayscale. Otherwise it is an average calculated from the red,
 * green, and blue components of the pixels. Using grayscale images may improve
 * data conversion speed for large images.
 *
 * Not recommended formats: all mono formats (for example QImage::Format_Mono).
 *
 * The height map is resolved asynchronously. QSurfaceDataProxy::arrayReset() is
 * emitted when the data has been resolved.
 */
void QHeightMapSurfaceDataProxy::setHeightMap(const QImage &image)
{
    Q_D(QHeightMapSurfaceDataProxy);
    d->m_heightMap = image;

    // We do resolving asynchronously to make qml onArrayReset handlers actually
    // get the initial reset
    if (!d->m_resolveTimer.isActive())
        d->m_resolveTimer.start(0);
}

QImage QHeightMapSurfaceDataProxy::heightMap() const
{
    Q_D(const QHeightMapSurfaceDataProxy);
    return d->m_heightMap;
}

/*!
 * \property QHeightMapSurfaceDataProxy::heightMapFile
 *
 * \brief The name of the file with a height map image to be visualized.
 */

/*!
 * Replaces current data with height map data from the file specified by
 * \a filename.
 *
 * \sa heightMap
 */
void QHeightMapSurfaceDataProxy::setHeightMapFile(const QString &filename)
{
    Q_D(QHeightMapSurfaceDataProxy);
    QFileInfo validfile(filename);
    // Check if the filename is empty, in which case we should clear the height map,
    // or if not, it's an actual file that can be found
    if (!filename.isEmpty() && (!validfile.exists() || !validfile.isFile())) {
        qCWarning(lcProperties3D, "%s height map file %s does not exist.",
                  qUtf8Printable(QLatin1String(__func__)), qUtf8Printable(filename));
        return;
    }
    if (d->m_heightMapFile == filename) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(filename));
        return;
    }
    d->m_heightMapFile = filename;
    setHeightMap(QImage(filename));
    emit heightMapFileChanged(filename);
}

QString QHeightMapSurfaceDataProxy::heightMapFile() const
{
    Q_D(const QHeightMapSurfaceDataProxy);
    return d->m_heightMapFile;
}

/*!
 * A convenience function for setting all minimum (\a minX and \a minZ) and
 * maximum
 * (\a maxX and \a maxZ) values at the same time. The minimum values must be
 * smaller than the corresponding maximum value. Otherwise the values get
 * adjusted so that they are valid.
 */
void QHeightMapSurfaceDataProxy::setValueRanges(float minX, float maxX, float minZ, float maxZ)
{
    Q_D(QHeightMapSurfaceDataProxy);
    d->setValueRanges(minX, maxX, minZ, maxZ);
}

/*!
 * \property QHeightMapSurfaceDataProxy::minXValue
 *
 * \brief The minimum X value for the generated surface points.
 *
 * Defaults to \c{0.0}.
 *
 * When setting this property the corresponding maximum value is adjusted if
 * necessary, to ensure that the range remains valid.
 */
void QHeightMapSurfaceDataProxy::setMinXValue(float min)
{
    Q_D(QHeightMapSurfaceDataProxy);
    d->setMinXValue(min);
}

float QHeightMapSurfaceDataProxy::minXValue() const
{
    Q_D(const QHeightMapSurfaceDataProxy);
    return d->m_minXValue;
}

/*!
 * \property QHeightMapSurfaceDataProxy::maxXValue
 *
 * \brief The maximum X value for the generated surface points.
 *
 * Defaults to \c{10.0}.
 *
 * When setting this property the corresponding minimum value is adjusted if
 * necessary, to ensure that the range remains valid.
 */
void QHeightMapSurfaceDataProxy::setMaxXValue(float max)
{
    Q_D(QHeightMapSurfaceDataProxy);
    d->setMaxXValue(max);
}

float QHeightMapSurfaceDataProxy::maxXValue() const
{
    Q_D(const QHeightMapSurfaceDataProxy);
    return d->m_maxXValue;
}

/*!
 * \property QHeightMapSurfaceDataProxy::minZValue
 *
 * \brief The minimum Z value for the generated surface points.
 *
 * Defaults to \c{0.0}.
 *
 * When setting this property the corresponding maximum value is adjusted if
 * necessary, to ensure that the range remains valid.
 */
void QHeightMapSurfaceDataProxy::setMinZValue(float min)
{
    Q_D(QHeightMapSurfaceDataProxy);
    d->setMinZValue(min);
}

float QHeightMapSurfaceDataProxy::minZValue() const
{
    Q_D(const QHeightMapSurfaceDataProxy);
    return d->m_minZValue;
}

/*!
 * \property QHeightMapSurfaceDataProxy::maxZValue
 *
 * \brief The maximum Z value for the generated surface points.
 *
 * Defaults to \c{10.0}.
 *
 * When setting this property the corresponding minimum value is adjusted if
 * necessary, to ensure that the range remains valid.
 */
void QHeightMapSurfaceDataProxy::setMaxZValue(float max)
{
    Q_D(QHeightMapSurfaceDataProxy);
    d->setMaxZValue(max);
}

float QHeightMapSurfaceDataProxy::maxZValue() const
{
    Q_D(const QHeightMapSurfaceDataProxy);
    return d->m_maxZValue;
}

/*!
 * \property QHeightMapSurfaceDataProxy::minYValue
 *
 * \brief The minimum Y value for the generated surface points.
 *
 * Defaults to \c{0.0}.
 *
 * When setting this property the corresponding maximum value is adjusted if
 * necessary, to ensure that the range remains valid.
 *
 * \sa autoScaleY
 */
void QHeightMapSurfaceDataProxy::setMinYValue(float min)
{
    Q_D(QHeightMapSurfaceDataProxy);
    d->setMinYValue(min);
}

float QHeightMapSurfaceDataProxy::minYValue() const
{
    Q_D(const QHeightMapSurfaceDataProxy);
    return d->m_minYValue;
}

/*!
 * \property QHeightMapSurfaceDataProxy::maxYValue
 *
 * \brief The maximum Y value for the generated surface points.
 *
 * Defaults to \c{10.0}.
 *
 * When setting this property the corresponding minimum value is adjusted if
 * necessary, to ensure that the range remains valid.
 *
 * \sa autoScaleY
 */
void QHeightMapSurfaceDataProxy::setMaxYValue(float max)
{
    Q_D(QHeightMapSurfaceDataProxy);
    d->setMaxYValue(max);
}

float QHeightMapSurfaceDataProxy::maxYValue() const
{
    Q_D(const QHeightMapSurfaceDataProxy);
    return d->m_maxYValue;
}

/*!
 * \property QHeightMapSurfaceDataProxy::autoScaleY
 *
 * \brief Scale height values to Y-axis.
 *
 * Defaults to \c{false}.
 *
 * When this property is set to \c{true},
 * the height values are scaled to fit on the Y-axis between minYValue and
 * maxYValue.
 *
 * \sa minYValue, maxYValue
 */
void QHeightMapSurfaceDataProxy::setAutoScaleY(bool enabled)
{
    Q_D(QHeightMapSurfaceDataProxy);
    d->setAutoScaleY(enabled);
}

bool QHeightMapSurfaceDataProxy::autoScaleY() const
{
    Q_D(const QHeightMapSurfaceDataProxy);
    return d->m_autoScaleY;
}

/*!
 * \internal
 */
void QHeightMapSurfaceDataProxy::handlePendingResolve()
{
    Q_D(QHeightMapSurfaceDataProxy);
    d->handlePendingResolve();
}

//  QHeightMapSurfaceDataProxyPrivate

QHeightMapSurfaceDataProxyPrivate::QHeightMapSurfaceDataProxyPrivate()
    : m_minXValue(defaultMinValue)
    , m_maxXValue(defaultMaxValue)
    , m_minZValue(defaultMinValue)
    , m_maxZValue(defaultMaxValue)
    , m_minYValue(defaultMinValue)
    , m_maxYValue(defaultMaxValue)
    , m_autoScaleY(false)
{
    m_resolveTimer.setSingleShot(true);
}

QHeightMapSurfaceDataProxyPrivate::~QHeightMapSurfaceDataProxyPrivate() {}

void QHeightMapSurfaceDataProxyPrivate::setValueRanges(float minX,
                                                       float maxX,
                                                       float minZ,
                                                       float maxZ)
{
    Q_Q(QHeightMapSurfaceDataProxy);
    bool minXChanged = false;
    bool maxXChanged = false;
    bool minZChanged = false;
    bool maxZChanged = false;
    if (m_minXValue != minX) {
        m_minXValue = minX;
        minXChanged = true;
    }
    if (m_minZValue != minZ) {
        m_minZValue = minZ;
        minZChanged = true;
    }
    if (m_maxXValue != maxX || minX >= maxX) {
        if (minX >= maxX) {
            m_maxXValue = minX + 1.0f;
            qCWarning(lcProperties3D, "%s tried to set invalid range for X value range."
                     " Range automatically adjusted to a valid one: %f - %f --> %f - %f",
                     qUtf8Printable(QLatin1String(__FUNCTION__)),
                     minX,
                     maxX,
                     m_minXValue,
                     m_maxXValue);
        } else {
            m_maxXValue = maxX;
        }
        maxXChanged = true;
    }
    if (m_maxZValue != maxZ || minZ >= maxZ) {
        if (minZ >= maxZ) {
            m_maxZValue = minZ + 1.0f;
            qCWarning(lcProperties3D, "%s tried to set invalid range for Z value range."
                     " Range automatically adjusted to a valid one: %f - %f --> %f - %f",
                     qUtf8Printable(QLatin1String(__FUNCTION__)),
                     minZ,
                     maxZ,
                     m_minZValue,
                     m_maxZValue);
        } else {
            m_maxZValue = maxZ;
        }
        maxZChanged = true;
    }

    if (minXChanged)
        emit q->minXValueChanged(m_minXValue);
    if (minZChanged)
        emit q->minZValueChanged(m_minZValue);
    if (maxXChanged)
        emit q->maxXValueChanged(m_maxXValue);
    if (maxZChanged)
        emit q->maxZValueChanged(m_maxZValue);

    if ((minXChanged || minZChanged || maxXChanged || maxZChanged) && !m_resolveTimer.isActive())
        m_resolveTimer.start(0);
}

void QHeightMapSurfaceDataProxyPrivate::setMinXValue(float min)
{
    Q_Q(QHeightMapSurfaceDataProxy);
    if (min != m_minXValue) {
        bool maxChanged = false;
        if (min >= m_maxXValue) {
            float oldMax = m_maxXValue;
            m_maxXValue = min + 1.0f;
            qCWarning(lcProperties3D, "%s tried to set minimum X to equal or larger"
                     " than maximum X for value "
                     "range. Maximum automatically adjusted to a valid one: %f --> %f",
                     qUtf8Printable(QLatin1String(__func__)),
                     oldMax,
                     m_maxXValue);
            maxChanged = true;
        }
        m_minXValue = min;
        emit q->minXValueChanged(m_minXValue);
        if (maxChanged)
            emit q->maxXValueChanged(m_maxXValue);

        if (!m_resolveTimer.isActive())
            m_resolveTimer.start(0);
    }
}

void QHeightMapSurfaceDataProxyPrivate::setMaxXValue(float max)
{
    Q_Q(QHeightMapSurfaceDataProxy);
    if (m_maxXValue != max) {
        bool minChanged = false;
        if (max <= m_minXValue) {
            float oldMin = m_minXValue;
            m_minXValue = max - 1.0f;
            qCWarning(lcProperties3D, "%s tried to set maximum X to equal or smaller than minimum X"
                     " for value range. Minimum automatically adjusted to a valid one: %f --> %f",
                     qUtf8Printable(QLatin1String(__func__)),
                     oldMin,
                     m_minXValue);
            minChanged = true;
        }
        m_maxXValue = max;
        emit q->maxXValueChanged(m_maxXValue);
        if (minChanged)
            emit q->minXValueChanged(m_minXValue);

        if (!m_resolveTimer.isActive())
            m_resolveTimer.start(0);
    }
}

void QHeightMapSurfaceDataProxyPrivate::setMinZValue(float min)
{
    Q_Q(QHeightMapSurfaceDataProxy);
    if (min != m_minZValue) {
        bool maxChanged = false;
        if (min >= m_maxZValue) {
            float oldMax = m_maxZValue;
            m_maxZValue = min + 1.0f;
            qCWarning(lcProperties3D, "%s tried to set minimum Z to equal or larger than maximum Z"
                     " for value range. Maximum automatically adjusted to a valid one: %f --> %f",
                     qUtf8Printable(QLatin1String(__func__)),
                     oldMax,
                     m_maxZValue);
            maxChanged = true;
        }
        m_minZValue = min;
        emit q->minZValueChanged(m_minZValue);
        if (maxChanged)
            emit q->maxZValueChanged(m_maxZValue);

        if (!m_resolveTimer.isActive())
            m_resolveTimer.start(0);
    }
}

void QHeightMapSurfaceDataProxyPrivate::setMaxZValue(float max)
{
    Q_Q(QHeightMapSurfaceDataProxy);
    if (m_maxZValue != max) {
        bool minChanged = false;
        if (max <= m_minZValue) {
            float oldMin = m_minZValue;
            m_minZValue = max - 1.0f;
            qCWarning(lcProperties3D, "%s tried to set maximum Z to equal or smaller than minimum Z"
                     " for value range. Minimum automatically adjusted to a valid one: %f --> %f",
                     qUtf8Printable(QLatin1String(__func__)),
                     oldMin,
                     m_minZValue);
            minChanged = true;
        }
        m_maxZValue = max;
        emit q->maxZValueChanged(m_maxZValue);
        if (minChanged)
            emit q->minZValueChanged(m_minZValue);

        if (!m_resolveTimer.isActive())
            m_resolveTimer.start(0);
    }
}

void QHeightMapSurfaceDataProxyPrivate::setMinYValue(float min)
{
    Q_Q(QHeightMapSurfaceDataProxy);
    if (m_minYValue != min) {
        bool maxChanged = false;
        if (min >= m_maxYValue) {
            float oldMax = m_maxYValue;
            m_maxYValue = min + 1.0f;
            qCWarning(lcProperties3D, "%s tried to set minimum Y to equal or larger than maximum Y"
                     " for value range. Maximum automatically adjusted to a valid one: %f --> %f",
                     qUtf8Printable(QLatin1String(__func__)),
                     oldMax,
                     m_maxYValue);
            maxChanged = true;
        }
        m_minYValue = min;
        emit q->minYValueChanged(m_minYValue);
        if (maxChanged)
            emit q->maxYValueChanged(m_maxYValue);

        if (!m_resolveTimer.isActive())
            m_resolveTimer.start(0);
    }
}

void QHeightMapSurfaceDataProxyPrivate::setMaxYValue(float max)
{
    Q_Q(QHeightMapSurfaceDataProxy);
    if (m_maxYValue != max) {
        bool minChanged = false;
        if (max <= m_minYValue) {
            float oldMin = m_minYValue;
            m_minYValue = max - 1.0f;
            qCWarning(lcProperties3D, "%s tried to set maximum Y to equal or smaller than minimum Y"
                     " for value range. Minimum automatically adjusted to a valid one: %f --> %f",
                     qUtf8Printable(QLatin1String(__func__)),
                     oldMin,
                     m_minYValue);
            minChanged = true;
        }
        m_maxYValue = max;
        emit q->maxYValueChanged(m_maxYValue);
        if (minChanged)
            emit q->minYValueChanged(m_minYValue);

        if (!m_resolveTimer.isActive())
            m_resolveTimer.start(0);
    }
}

void QHeightMapSurfaceDataProxyPrivate::setAutoScaleY(bool enabled)
{
    Q_Q(QHeightMapSurfaceDataProxy);
    if (enabled == m_autoScaleY) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << enabled;
        return;
    }

    m_autoScaleY = enabled;
    emit q->autoScaleYChanged(m_autoScaleY);

    if (!m_resolveTimer.isActive())
        m_resolveTimer.start(0);
}

void QHeightMapSurfaceDataProxyPrivate::handlePendingResolve()
{
    Q_Q(QHeightMapSurfaceDataProxy);
    QImage heightImage = m_heightMap;
    int bytesInChannel = 1;
    float yMul = 1.0f / UINT8_MAX;

    bool is16bit = (heightImage.format() == QImage::Format_RGBX64
                    || heightImage.format() == QImage::Format_RGBA64
                    || heightImage.format() == QImage::Format_RGBA64_Premultiplied
                    || heightImage.format() == QImage::Format_Grayscale16);

    Q_TRACE(QGraphs3DHeightMapSurfaceDataProxHandlePendingResolve_entry, is16bit);
    // Convert to RGB32 to be sure we're reading the right bytes
    if (is16bit) {
        if (heightImage.format() != QImage::Format_RGBX64)
            heightImage = heightImage.convertToFormat(QImage::Format_RGBX64);

        bytesInChannel = 2;
        yMul = 1.0f / UINT16_MAX;
    } else if (heightImage.format() != QImage::Format_RGB32) {
        heightImage = heightImage.convertToFormat(QImage::Format_RGB32);
    }

    uchar *bits = heightImage.bits();

    int imageHeight = heightImage.height();
    int imageWidth = heightImage.width();
    int bitCount = imageWidth * 4 * (imageHeight - 1) * bytesInChannel;
    int widthBits = imageWidth * 4 * bytesInChannel;
    float height = 0;

    // Do not recreate array if dimensions have not changed
    QSurfaceDataArray dataArray = q->series()->dataArray();
    if (imageWidth != q->columnCount() || imageHeight != dataArray.size()) {
        dataArray.clear();
        dataArray.reserve(imageHeight);
        for (int i = 0; i < imageHeight; i++) {
            QSurfaceDataRow newProxyRow(imageWidth);
            dataArray.append(newProxyRow);
        }
    }
    yMul *= m_maxYValue - m_minYValue;
    float xMul = (m_maxXValue - m_minXValue) / float(imageWidth - 1);
    float zMul = (m_maxZValue - m_minZValue) / float(imageHeight - 1);

    // Last row and column are explicitly set to max values, as relying
    // on multiplier can cause rounding errors, resulting in the value being
    // slightly over the specified maximum, which in turn can lead to it not
    // getting rendered.
    int lastRow = imageHeight - 1;
    int lastCol = imageWidth - 1;
    if (heightImage.isGrayscale()) {
        // Grayscale, it's enough to read Red byte
        for (int i = 0; i < imageHeight; i++, bitCount -= widthBits) {
            QSurfaceDataRow &newRow = dataArray[i];
            float zVal;
            if (i == lastRow)
                zVal = m_maxZValue;
            else
                zVal = (float(i) * zMul) + m_minZValue;
            int j = 0;
            float yVal = 0;
            uchar *pixelptr;
            for (; j < lastCol; j++) {
                pixelptr = (uchar *) (bits + bitCount + (j * 4 * bytesInChannel));
                if (!m_autoScaleY)
                    yVal = *pixelptr;
                else
                    yVal = float(*pixelptr) * yMul + m_minYValue;
                newRow[j].setPosition(QVector3D((float(j) * xMul) + m_minXValue, yVal, zVal));
            }
            newRow[j].setPosition(QVector3D(m_maxXValue, yVal, zVal));
        }
    } else {
        // Not grayscale, we'll need to calculate height from RGB
        for (int i = 0; i < imageHeight; i++, bitCount -= widthBits) {
            QSurfaceDataRow &newRow = dataArray[i];
            float zVal;
            if (i == lastRow)
                zVal = m_maxZValue;
            else
                zVal = (float(i) * zMul) + m_minZValue;
            int j = 0;
            float yVal = 0;
            for (; j < lastCol; j++) {
                int nextpixel = j * 4 * bytesInChannel;
                uchar *pixelptr = (uchar *) (bits + bitCount + nextpixel);
                if (is16bit) {
                    height = (float(*((ushort *) pixelptr)) + float(*(((ushort *) pixelptr) + 1))
                              + float(*(((ushort *) pixelptr) + 2)));
                } else {
                    height = (float(*pixelptr) + float(*(pixelptr + 1)) + float(*(pixelptr + 2)));
                }
                if (!m_autoScaleY)
                    yVal = height / 3.0f;
                else
                    yVal = (height / 3.0f * yMul) + m_minYValue;

                newRow[j].setPosition(QVector3D((float(j) * xMul) + m_minXValue, yVal, zVal));
            }
            newRow[j].setPosition(QVector3D(m_maxXValue, yVal, zVal));
        }
    }

    Q_TRACE(QGraphs3DHeightMapSurfaceDataProxHandlePendingResolve_exit);
    q->resetArray(dataArray);
    emit q->heightMapChanged(m_heightMap);
}

QT_END_NAMESPACE

#include "moc_qheightmapsurfacedataproxy.cpp"
