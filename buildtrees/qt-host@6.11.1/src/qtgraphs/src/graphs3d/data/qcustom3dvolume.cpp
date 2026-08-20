// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:execute-external-code

#include "qcustom3dvolume_p.h"
#include "qgraphs3dlogging_p.h"

#include <QtGui/qquaternion.h>

#include <qtgraphs_tracepoints_p.h>

QT_BEGIN_NAMESPACE

Q_TRACE_PREFIX(qtgraphs,
              "QT_BEGIN_NAMESPACE" \
              "#include <qnamespace.h>" \
              "class QCustom3DVolume;" \
              "QT_END_NAMESPACE"
          )

Q_TRACE_METADATA(qtgraphs, "ENUM { } Qt::Axis;")

Q_TRACE_POINT(qtgraphs, QGraphs3DCustomVolumeRenderSlice_entry, Qt::Axis axis, int index);
Q_TRACE_POINT(qtgraphs, QGraphs3DCustomVolumeRenderSlice_exit)

/*!
 * \class QCustom3DVolume
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief The QCustom3DVolume class adds a volume rendered object to a graph.
 *
 * A volume rendered
 * object is a box with a 3D texture. Three slice planes are supported for the
 * volume, one along each main axis of the volume.
 *
 * Rendering volume objects is very performance intensive, especially when the
 * volume is largely transparent, as the contents of the volume are ray-traced.
 * The performance scales nearly linearly with the amount of pixels that the
 * volume occupies on the screen, so showing the volume in a smaller view or
 * limiting the zoom level of the graph are easy ways to improve performance.
 * Similarly, the volume texture dimensions have a large impact on performance.
 * If the frame rate is more important than pixel-perfect rendering of the
 * volume contents, consider turning the high definition shader off by setting
 * the useHighDefShader property to \c{false}.
 *
 * \note Volumetric objects utilize 3D textures, which are not supported in
 * OpenGL ES2 environments.
 *
 * \note Only two formats are supported:
 * QImage::Format_Indexed8 and QImage::Format_ARGB32. If an indexed format is
 * specified, colorTable must also be set. Defaults to QImage::Format_ARGB32.
 *
 * \sa Q3DGraphsWidgetItem::addCustomItem(), useHighDefShader
 */

/*!
 * \qmltype Custom3DVolume
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \nativetype QCustom3DVolume
 * \inherits Custom3DItem
 * \brief Adds a volume rendered object to a graph.
 *
 * A volume rendered
 * object is a box with a 3D texture. Three slice planes are supported for the
 * volume, one along each main axis of the volume.
 *
 * Rendering volume objects is very performance intensive, especially when the
 * volume is largely transparent, as the contents of the volume are ray-traced.
 * The performance scales nearly linearly with the amount of pixels that the
 * volume occupies on the screen, so showing the volume in a smaller view or
 * limiting the zoom level of the graph are easy ways to improve performance.
 * Similarly, the volume texture dimensions have a large impact on performance.
 * If the frame rate is more important than pixel-perfect rendering of the
 * volume contents, consider turning the high definition shader off by setting
 * the useHighDefShader property to \c{false}.
 *
 * \note Filling in the volume data would not typically be efficient or
 * practical from pure QML, so properties directly related to that are not fully
 * supported from QML. Create a hybrid QML/C++ application if you want to use
 * volume objects with a Qt Quick UI.
 *
 * \note Volumetric objects utilize 3D textures, which are not supported in
 * OpenGL ES2 environments.
 *
 * \sa useHighDefShader
 */

/*! \qmlproperty int Custom3DVolume::textureWidth
 *
 * The width of the 3D texture defining the volume content in pixels. Defaults
 * to \c{0}.
 *
 * \note Changing this property from QML is not supported, as the texture data
 * cannot be resized accordingly.
 */

/*! \qmlproperty int Custom3DVolume::textureHeight
 *
 * The height of the 3D texture defining the volume content in pixels. Defaults
 * to \c{0}.
 *
 * \note Changing this property from QML is not supported, as the texture data
 * cannot be resized accordingly.
 */

/*! \qmlproperty int Custom3DVolume::textureDepth
 *
 * The depth of the 3D texture defining the volume content in pixels. Defaults
 * to \c{0}.
 *
 * \note Changing this property from QML is not supported, as the texture data
 * cannot be resized accordingly.
 */

/*! \qmlproperty int Custom3DVolume::sliceIndexX
 *
 * The x-dimension index into the texture data indicating which vertical slice
 * to show. Setting any dimension to negative indicates no slice or slice frame
 * for that dimension is drawn. If all dimensions are negative, no slices or
 * slice frames are drawn and the volume is drawn normally. Defaults to \c{-1}.
 *
 * \sa QCustom3DVolume::textureData, drawSlices, drawSliceFrames
 */

/*! \qmlproperty int Custom3DVolume::sliceIndexY
 *
 * The y-dimension index into the texture data indicating which horizontal slice
 * to show. Setting any dimension to negative indicates no slice or slice frame
 * for that dimension is drawn. If all dimensions are negative, no slices or
 * slice frames are drawn and the volume is drawn normally. Defaults to \c{-1}.
 *
 * \sa QCustom3DVolume::textureData, drawSlices, drawSliceFrames
 */

/*! \qmlproperty int Custom3DVolume::sliceIndexZ
 *
 * The z-dimension index into the texture data indicating which vertical slice
 * to show. Setting any dimension to negative indicates no slice or slice frame
 * for that dimension is drawn. If all dimensions are negative, no slices or
 * slice frames are drawn and the volume is drawn normally. Defaults to \c{-1}.
 *
 * \sa QCustom3DVolume::textureData, drawSlices, drawSliceFrames
 */

/*!
 * \qmlproperty real Custom3DVolume::alphaMultiplier
 *
 * The alpha value of every texel of the volume texture is multiplied with this
 * value at the render time. This can be used to introduce uniform transparency
 * to the volume. If preserveOpacity is \c{true}, only texels with at least some
 * transparency to begin with are affected, and fully opaque texels are not
 * affected. The value must not be negative. Defaults to \c{1.0}.
 *
 * \sa preserveOpacity
 */

/*!
 * \qmlproperty bool Custom3DVolume::preserveOpacity
 *
 * If this property value is \c{true}, alphaMultiplier is only applied to texels
 * that already have some transparency. If it is \c{false}, the multiplier is
 * applied to the alpha value of all texels. Defaults to \c{true}.
 *
 * \sa alphaMultiplier
 */

/*!
 * \qmlproperty bool Custom3DVolume::useHighDefShader
 *
 * If this property value is \c{true}, a high definition shader is used to
 * render the volume. If it is \c{false}, a low definition shader is used.
 *
 * The high definition shader guarantees that every visible texel of the volume
 * texture is sampled when the volume is rendered. The low definition shader
 * renders only a rough approximation of the volume contents, but at a much
 * higher frame rate. The low definition shader does not guarantee every texel
 * of the volume texture is sampled, so there may be flickering if the volume
 * contains distinct thin features.
 *
 * \note This value does not affect the level of detail when rendering the
 * slices of the volume.
 *
 * Defaults to \c{true}.
 */

/*!
 * \qmlproperty bool Custom3DVolume::drawSlices
 *
 * If this property value is \c{true}, the slices indicated by slice index
 * properties will be drawn instead of the full volume. If it is \c{false}, the
 * full volume will always be drawn. Defaults to \c{false}.
 *
 * \note The slices are always drawn along the item axes, so if the item is
 * rotated, the slices are rotated as well.
 *
 * \sa sliceIndexX, sliceIndexY, sliceIndexZ
 */

/*!
 * \qmlproperty bool Custom3DVolume::drawSliceFrames
 *
 * If this property value is \c{true}, the frames of slices indicated by slice
 * index properties will be drawn around the volume. If it is \c{false}, no
 * slice frames will be drawn. Drawing slice frames is independent of drawing
 * slices, so you can show the full volume and still draw the slice frames
 * around it. Defaults to \c{false}.
 *
 * \sa sliceIndexX, sliceIndexY, sliceIndexZ, drawSlices
 */

/*!
 * \qmlproperty color Custom3DVolume::sliceFrameColor
 *
 * The color of the slice frame. Transparent slice frame color is not supported.
 *
 * Defaults to black.
 *
 * \sa drawSliceFrames
 */

/*!
 * \qmlproperty vector3d Custom3DVolume::sliceFrameWidths
 *
 * The widths of the slice frame. The width can be different on different
 * dimensions, so you can for example omit drawing the frames on certain sides
 * of the volume by setting the value for that dimension to zero. The values are
 * fractions of the volume thickness in the same dimension. The values cannot be
 * negative.
 *
 * Defaults to \c{vector3d(0.01, 0.01, 0.01)}.
 *
 * \sa drawSliceFrames
 */

/*!
 * \qmlproperty vector3d Custom3DVolume::sliceFrameGaps
 *
 * The size of the air gap left between the volume itself and the frame in each
 * dimension. The gap can be different on different dimensions. The values are
 * fractions of the volume thickness in the same dimension. The values cannot be
 * negative.
 *
 * Defaults to \c{vector3d(0.01, 0.01, 0.01)}.
 *
 * \sa drawSliceFrames
 */

/*!
 * \qmlproperty vector3d Custom3DVolume::sliceFrameThicknesses
 *
 * The thickness of the slice frames for each dimension. The values are
 * fractions of the volume thickness in the same dimension. The values cannot be
 * negative.
 *
 * Defaults to \c{vector3d(0.01, 0.01, 0.01)}.
 *
 * \sa drawSliceFrames
 */

/*!
    \qmlsignal Custom3DVolume::textureWidthChanged(int value)

    This signal is emitted when textureWidth changes to \a value.
*/

/*!
    \qmlsignal Custom3DVolume::textureHeightChanged(int value)

    This signal is emitted when textureHeight changes to \a value.
*/

/*!
    \qmlsignal Custom3DVolume::textureDepthChanged(int value)

    This signal is emitted when textureDepth changes to \a value.
*/

/*!
    \qmlsignal Custom3DVolume::sliceIndexXChanged(int value)

    This signal is emitted when sliceIndexX changes to \a value.
*/

/*!
    \qmlsignal Custom3DVolume::sliceIndexYChanged(int value)

    This signal is emitted when sliceIndexY changes to \a value.
*/

/*!
    \qmlsignal Custom3DVolume::sliceIndexZChanged(int value)

    This signal is emitted when sliceIndexZ changes to \a value.
*/

/*!
    \qmlsignal Custom3DVolume::alphaMultiplierChanged(float mult)

    This signal is emitted when alphaMultiplier changes to \a mult.
*/

/*!
    \qmlsignal Custom3DVolume::preserveOpacityChanged(bool enabled)

    This signal is emitted when preserveOpacity changes to \a enabled.
*/

/*!
    \qmlsignal Custom3DVolume::useHighDefShaderChanged(bool enabled)

    This signal is emitted when useHighDefShader changes to \a enabled.
*/

/*!
    \qmlsignal Custom3DVolume::drawSlicesChanged(bool enabled)

    This signal is emitted when drawSlices changes to \a enabled.
*/

/*!
    \qmlsignal Custom3DVolume::drawSliceFramesChanged(bool enabled)

    This signal is emitted when drawSliceFrames changes to \a enabled.
*/

/*!
    \qmlsignal Custom3DVolume::sliceFrameColorChanged(color color)

    This signal is emitted when sliceFrameColor changes to \a color.
*/

/*!
    \qmlsignal Custom3DVolume::sliceFrameWidthsChanged(vector3d values)

    This signal is emitted when sliceFrameWidths changes to \a values.
*/

/*!
    \qmlsignal Custom3DVolume::sliceFrameGapsChanged(vector3d values)

    This signal is emitted when sliceFrameGaps changes to \a values.
*/

/*!
    \qmlsignal Custom3DVolume::sliceFrameThicknessesChanged(vector3d values)

    This signal is emitted when sliceFrameThicknesses changes to \a values.
*/

/*!
 * Constructs a custom 3D volume with the given \a parent.
 */
QCustom3DVolume::QCustom3DVolume(QObject *parent)
    : QCustom3DItem(*(new QCustom3DVolumePrivate()), parent)
{}

/*!
 * Constructs a custom 3D volume with the given \a position, \a scaling, \a
 * rotation, \a textureWidth, \a textureHeight, \a textureDepth, \a textureData,
 * \a textureFormat, \a colorTable, and optional \a parent.
 *
 * \sa textureData, setTextureFormat(), colorTable
 */
QCustom3DVolume::QCustom3DVolume(QVector3D position,
                                 QVector3D scaling,
                                 const QQuaternion &rotation,
                                 int textureWidth,
                                 int textureHeight,
                                 int textureDepth,
                                 QList<uchar> *textureData,
                                 QImage::Format textureFormat,
                                 const QList<QRgb> &colorTable,
                                 QObject *parent)
    : QCustom3DItem(*(new QCustom3DVolumePrivate(position,
                                                 scaling,
                                                 rotation,
                                                 textureWidth,
                                                 textureHeight,
                                                 textureDepth,
                                                 textureData,
                                                 textureFormat,
                                                 colorTable)),
                    parent)
{}

/*!
 * Deletes the custom 3D volume.
 */
QCustom3DVolume::~QCustom3DVolume() {}

/*! \property QCustom3DVolume::textureWidth
 *
 * \brief The width of the 3D texture defining the volume content in pixels.
 *
 * Defaults to \c{0}.
 *
 * \note The textureData value may need to be resized or recreated if this value
 * is changed.
 * Defaults to \c{0}.
 *
 * \sa textureData, textureHeight, textureDepth, setTextureFormat(),
 * textureDataWidth()
 */
void QCustom3DVolume::setTextureWidth(int value)
{
    Q_D(QCustom3DVolume);
    if (value >= 0) {
        if (d->m_textureWidth == value) {
            qCDebug(lcProperties3D, "%s value is already set to: %d",
                    qUtf8Printable(QLatin1String(__FUNCTION__)), value);
            return;
        }
        d->m_textureWidth = value;
        d->m_dirtyBitsVolume.textureDimensionsDirty = true;
        emit textureWidthChanged(value);
        emit needUpdate();
    } else {
        qCWarning(lcProperties3D, "%s cannot set negative value.",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
    }
}

int QCustom3DVolume::textureWidth() const
{
    Q_D(const QCustom3DVolume);
    return d->m_textureWidth;
}

/*! \property QCustom3DVolume::textureHeight
 *
 * \brief The height of the 3D texture defining the volume content in pixels.
 *
 * Defaults to \c{0}.
 *
 * \note The textureData value may need to be resized or recreated if this value
 * is changed.
 * Defaults to \c{0}.
 *
 * \sa textureData, textureWidth, textureDepth, setTextureFormat()
 */
void QCustom3DVolume::setTextureHeight(int value)
{
    Q_D(QCustom3DVolume);
    if (value >= 0) {
        if (d->m_textureHeight == value) {
            qCDebug(lcProperties3D, "%s value is already set to: %d",
                    qUtf8Printable(QLatin1String(__FUNCTION__)), value);
            return;
        }
        d->m_textureHeight = value;
        d->m_dirtyBitsVolume.textureDimensionsDirty = true;
        emit textureHeightChanged(value);
        emit needUpdate();
    } else {
        qCWarning(lcProperties3D, "%s cannot set negative value.",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
    }
}

int QCustom3DVolume::textureHeight() const
{
    Q_D(const QCustom3DVolume);
    return d->m_textureHeight;
}

/*! \property QCustom3DVolume::textureDepth
 *
 * \brief The depth of the 3D texture defining the volume content in pixels.
 *
 * Defaults to \c{0}.
 *
 * \note The textureData value may need to be resized or recreated if this value
 * is changed.
 * Defaults to \c{0}.
 *
 * \sa textureData, textureWidth, textureHeight, setTextureFormat()
 */
void QCustom3DVolume::setTextureDepth(int value)
{
    Q_D(QCustom3DVolume);
    if (value >= 0) {
        if (d->m_textureDepth == value) {
            qCDebug(lcProperties3D, "%s value is already set to: %d",
                    qUtf8Printable(QLatin1String(__FUNCTION__)), value);
            return;
        }

        d->m_textureDepth = value;
        d->m_dirtyBitsVolume.textureDimensionsDirty = true;
        emit textureDepthChanged(value);
        emit needUpdate();
    } else {
        qCWarning(lcProperties3D, "%s cannot set negative value.",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
    }
}

int QCustom3DVolume::textureDepth() const
{
    Q_D(const QCustom3DVolume);
    return d->m_textureDepth;
}

/*!
 * A convenience function for setting all three texture dimensions
 * (\a width, \a height, and \a depth) at once.
 *
 * \sa textureData
 */
void QCustom3DVolume::setTextureDimensions(int width, int height, int depth)
{
    setTextureWidth(width);
    setTextureHeight(height);
    setTextureDepth(depth);
}

/*!
 * Returns the actual texture data width. When the texture format is
 * QImage::Format_Indexed8, this value equals textureWidth aligned to a 32-bit
 * boundary. Otherwise, this value equals four times textureWidth.
 */
int QCustom3DVolume::textureDataWidth() const
{
    Q_D(const QCustom3DVolume);
    int dataWidth = d->m_textureWidth;

    if (d->m_textureFormat == QImage::Format_Indexed8)
        dataWidth += dataWidth % 4;
    else
        dataWidth *= 4;

    return dataWidth;
}

/*! \property QCustom3DVolume::sliceIndexX
 *
 * \brief The x-dimension index into the texture data indicating which vertical
 * slice to show.
 *
 * Setting any dimension to negative indicates no slice or slice frame for that
 * dimension is drawn. If all dimensions are negative, no slices or slice frames
 * are drawn and the volume is drawn normally.
 *
 * Defaults to \c{-1}.
 *
 * \sa textureData, drawSlices, drawSliceFrames
 */
void QCustom3DVolume::setSliceIndexX(int value)
{
    Q_D(QCustom3DVolume);
    if (d->m_sliceIndexX == value) {
        qCDebug(lcProperties3D, "%s value is already set to: %d",
                qUtf8Printable(QLatin1String(__FUNCTION__)), value);
        return;
    }

    d->m_sliceIndexX = value;
    d->m_dirtyBitsVolume.slicesDirty = true;
    emit sliceIndexXChanged(value);
    emit needUpdate();
}

int QCustom3DVolume::sliceIndexX() const
{
    Q_D(const QCustom3DVolume);
    return d->m_sliceIndexX;
}

/*! \property QCustom3DVolume::sliceIndexY
 *
 * \brief The y-dimension index into the texture data indicating which
 * horizontal slice to show.
 *
 * Setting any dimension to negative indicates no slice or slice frame for that
 * dimension is drawn. If all dimensions are negative, no slices or slice frames
 * are drawn and the volume is drawn normally.
 *
 * Defaults to \c{-1}.
 *
 * \sa textureData, drawSlices, drawSliceFrames
 */
void QCustom3DVolume::setSliceIndexY(int value)
{
    Q_D(QCustom3DVolume);
    if (d->m_sliceIndexY == value) {
        qCDebug(lcProperties3D, "%s value is already set to: %d",
                qUtf8Printable(QLatin1String(__FUNCTION__)), value);
    }

    d->m_sliceIndexY = value;
    d->m_dirtyBitsVolume.slicesDirty = true;
    emit sliceIndexYChanged(value);
    emit needUpdate();
}

int QCustom3DVolume::sliceIndexY() const
{
    Q_D(const QCustom3DVolume);
    return d->m_sliceIndexY;
}

/*! \property QCustom3DVolume::sliceIndexZ
 *
 * \brief The z-dimension index into the texture data indicating which vertical
 * slice to show.
 *
 * Setting any dimension to negative indicates no slice or slice frame for that
 * dimension is drawn. If all dimensions are negative, no slices or slice frames
 * are drawn and the volume is drawn normally.
 *
 * Defaults to \c{-1}.
 *
 * \sa textureData, drawSlices, drawSliceFrames
 */
void QCustom3DVolume::setSliceIndexZ(int value)
{
    Q_D(QCustom3DVolume);
    if (d->m_sliceIndexZ == value) {
        qCDebug(lcProperties3D, "%s value is already set to: %d",
                qUtf8Printable(QLatin1String(__FUNCTION__)), value);
        return;
    }
    d->m_sliceIndexZ = value;
    d->m_dirtyBitsVolume.slicesDirty = true;
    emit sliceIndexZChanged(value);
    emit needUpdate();
}

int QCustom3DVolume::sliceIndexZ() const
{
    Q_D(const QCustom3DVolume);
    return d->m_sliceIndexZ;
}

/*!
 * A convenience function for setting all three slice indices (\a x, \a y, and
 * \a z) at once.
 *
 * \sa textureData
 */
void QCustom3DVolume::setSliceIndices(int x, int y, int z)
{
    setSliceIndexX(x);
    setSliceIndexY(y);
    setSliceIndexZ(z);
}

/*! \property QCustom3DVolume::colorTable
 *
 * \brief The array containing the colors for indexed texture formats.
 *
 * If the texture format is not indexed, this array is not used and can be
 * empty.
 *
 * Defaults to \c{0}.
 *
 * \sa textureData, setTextureFormat(), QImage::colorTable()
 */
void QCustom3DVolume::setColorTable(const QList<QRgb> &colors)
{
    Q_D(QCustom3DVolume);
    if (d->m_colorTable == colors) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << colors;
        return;
    }

    d->m_colorTable = colors;
    d->m_dirtyBitsVolume.colorTableDirty = true;
    emit colorTableChanged();
    emit needUpdate();
}

QList<QRgb> QCustom3DVolume::colorTable() const
{
    Q_D(const QCustom3DVolume);
    return d->m_colorTable;
}

/*! \property QCustom3DVolume::textureData
 *
 * \brief The array containing the texture data in the format specified by
 * textureFormat.
 *
 * The size of this array must be at least
 * (\c{textureDataWidth * textureHeight * textureDepth * texture format color
 * depth in bytes}).
 *
 * A 3D texture is defined by a stack of 2D subtextures. Each subtexture must be
 * of identical size
 * (\c{textureDataWidth * textureHeight}), and the depth of the stack is defined
 * by the textureDepth property. The data in each 2D texture is identical to a
 * QImage data with the same format, so
 * QImage::bits() can be used to supply the data for each subtexture.
 *
 * Ownership of the new array transfers to the QCustom3DVolume instance.
 * If another array is set, the previous array is deleted.
 * If the same array is set again, it is assumed that the array contents have
 * been changed and the graph rendering is triggered.
 *
 * \note Each x-dimension line of the data needs to be 32-bit aligned.
 * If textureFormat is QImage::Format_Indexed8 and the textureWidth value is not
 * divisible by four, padding bytes might need to be added to each x-dimension
 * line of the \a data. The textureDataWidth() function returns the padded byte
 * count. The padding bytes should indicate a fully transparent color to avoid
 * rendering artifacts.
 *
 * Defaults to \c{0}.
 *
 * \sa colorTable, setTextureFormat(), setSubTextureData(), textureDataWidth()
 */
void QCustom3DVolume::setTextureData(QList<uchar> *data)
{
    Q_D(QCustom3DVolume);
    if (d->m_textureData != data)
        delete d->m_textureData;

    // Even if the pointer is same as previously, consider this property changed,
    // as the values can be changed unbeknownst to us via the array pointer.
    d->m_textureData = data;
    d->m_dirtyBitsVolume.textureDataDirty = true;
    emit textureDataChanged(data);
    emit needUpdate();
}

/*!
 * Creates a new texture data array from an array of \a images and sets it as
 * textureData for this volume object. The texture dimensions are also set
 * according to image and array dimensions. All of the images in the array must
 * be the same size. If the images are not all in the QImage::Format_Indexed8
 * format, all texture data will be converted into the QImage::Format_ARGB32
 * format. If the images are in the QImage::Format_Indexed8 format, the
 * colorTable value for the entire volume will be taken from the first image.
 *
 * Returns a pointer to the newly created array.
 *
 * \sa textureData, textureWidth, textureHeight, textureDepth,
 * setTextureFormat()
 */
QList<uchar> *QCustom3DVolume::createTextureData(const QList<QImage *> &images)
{
    Q_D(QCustom3DVolume);
    qsizetype imageCount = images.size();
    if (imageCount) {
        QImage *currentImage = images.at(0);
        int imageWidth = currentImage->width();
        int imageHeight = currentImage->height();
        QImage::Format imageFormat = currentImage->format();
        bool convert = false;
        if (imageFormat != QImage::Format_Indexed8 && imageFormat != QImage::Format_ARGB32) {
            convert = true;
            imageFormat = QImage::Format_ARGB32;
        } else {
            for (int i = 0; i < imageCount; i++) {
                currentImage = images.at(i);
                if (imageWidth != currentImage->width() || imageHeight != currentImage->height()) {
                    qCWarning(lcProperties3D, "%ls not all images were of the same size.",
                             qUtf16Printable(QString::fromUtf8(__func__)));
                    setTextureData(0);
                    setTextureWidth(0);
                    setTextureHeight(0);
                    setTextureDepth(0);
                    return 0;
                }
                if (currentImage->format() != imageFormat) {
                    convert = true;
                    imageFormat = QImage::Format_ARGB32;
                    break;
                }
            }
        }
        int colorBytes = (imageFormat == QImage::Format_Indexed8) ? 1 : 4;
        qsizetype imageByteWidth = (imageFormat == QImage::Format_Indexed8) ? currentImage->bytesPerLine()
                                                                            : imageWidth;
        qsizetype frameSize = imageByteWidth * imageHeight * colorBytes;
        QList<uchar> *newTextureData = new QList<uchar>;
        newTextureData->resize(frameSize * imageCount);
        uchar *texturePtr = newTextureData->data();
        QImage convertedImage;

        for (qsizetype i = 0; i < imageCount; i++) {
            currentImage = images.at(i);
            if (convert) {
                convertedImage = currentImage->convertToFormat(imageFormat);
                currentImage = &convertedImage;
            }
            memcpy(texturePtr, static_cast<void *>(currentImage->bits()), frameSize);
            texturePtr += frameSize;
        }

        if (imageFormat == QImage::Format_Indexed8)
            setColorTable(images.at(0)->colorTable());
        setTextureData(newTextureData);
        setTextureFormat(imageFormat);
        setTextureWidth(imageWidth);
        setTextureHeight(imageHeight);
        setTextureDepth(int(imageCount));
    } else {
        setTextureData(0);
        setTextureWidth(0);
        setTextureHeight(0);
        setTextureDepth(0);
    }
    return d->m_textureData;
}

QList<uchar> *QCustom3DVolume::textureData() const
{
    Q_D(const QCustom3DVolume);
    return d->m_textureData;
}

/*!
 * Sets a single 2D subtexture of the 3D texture along the specified
 * \a axis of the volume.
 * The \a index parameter specifies the subtexture to set.
 * The texture \a data must be in the format specified by the textureFormat
 * property and have the size of
 * the cross-section of the volume texture along the specified axis multiplied
 * by the texture format color depth in bytes. The \a data is expected to be
 * ordered similarly to the data in images produced by the renderSlice() method
 * along the same axis.
 *
 * \note Each x-dimension line of the data needs to be 32-bit aligned when
 * targeting the y-axis or z-axis. If textureFormat is QImage::Format_Indexed8
 * and the textureWidth value is not divisible by four, padding bytes might need
 * to be added to each x-dimension line of the \a data to properly align it. The
 * padding bytes should indicate a fully transparent color to avoid rendering
 * artifacts.
 *
 * \sa textureData, renderSlice()
 */
void QCustom3DVolume::setSubTextureData(Qt::Axis axis, int index, const uchar *data)
{
    Q_D(QCustom3DVolume);
    if (data) {
        int lineSize = textureDataWidth();
        int frameSize = lineSize * d->m_textureHeight;
        qsizetype dataSize = d->m_textureData->size();
        int pixelWidth = (d->m_textureFormat == QImage::Format_Indexed8) ? 1 : 4;
        int targetIndex;
        uchar *dataPtr = d->m_textureData->data();
        bool invalid = (index < 0);
        if (axis == Qt::XAxis) {
            targetIndex = index * pixelWidth;
            if (index >= d->m_textureWidth
                || (frameSize * (d->m_textureDepth - 1) + targetIndex) > dataSize) {
                invalid = true;
            }
        } else if (axis == Qt::YAxis) {
            targetIndex = (index * lineSize) + (frameSize * (d->m_textureDepth - 1));
            if (index >= d->m_textureHeight || (targetIndex + lineSize > dataSize))
                invalid = true;
        } else {
            targetIndex = index * frameSize;
            if (index >= d->m_textureDepth || ((targetIndex + frameSize) > dataSize))
                invalid = true;
        }

        if (invalid) {
            qCWarning(lcProperties3D, "%ls attempted to set invalid subtexture.",
                     qUtf16Printable(QString::fromUtf8(__func__)));
        } else {
            const uchar *sourcePtr = data;
            uchar *targetPtr = dataPtr + targetIndex;
            if (axis == Qt::XAxis) {
                int targetWidth = d->m_textureDepth;
                int targetHeight = d->m_textureHeight;
                for (int i = 0; i < targetHeight; i++) {
                    targetPtr = dataPtr + targetIndex + (lineSize * i);
                    for (int j = 0; j < targetWidth; j++) {
                        for (int k = 0; k < pixelWidth; k++)
                            *targetPtr++ = *sourcePtr++;
                        targetPtr += (frameSize - pixelWidth);
                    }
                }
            } else if (axis == Qt::YAxis) {
                int targetHeight = d->m_textureDepth;
                for (int i = 0; i < targetHeight; i++) {
                    for (int j = 0; j < lineSize; j++)
                        *targetPtr++ = *sourcePtr++;
                    targetPtr -= (frameSize + lineSize);
                }
            } else {
                void *subTexPtr = dataPtr + targetIndex;
                memcpy(subTexPtr, static_cast<const void *>(data), frameSize);
            }
            d->m_dirtyBitsVolume.textureDataDirty = true;
            emit textureDataChanged(d->m_textureData);
            emit needUpdate();
        }
    } else {
        qCDebug(lcProperties3D, "%s tried to set null data.",
                qUtf8Printable(QLatin1String(__FUNCTION__)));
    }
}

/*!
 * Sets a single 2D subtexture of the 3D texture along the specified
 * \a axis of the volume.
 * The \a index parameter specifies the subtexture to set.
 * The source \a image must be in the format specified by the textureFormat
 * property if textureFormat is indexed. If textureFormat is
 * QImage::Format_ARGB32, the image is converted to that format. The image must
 * have the size of the cross-section of the volume texture along the specified
 * axis. The orientation of the image should correspond to the orientation of
 * the slice image produced by renderSlice() method along the same axis.
 *
 * \note Each x-dimension line of the data needs to be 32-bit aligned when
 * targeting the y-axis or z-axis. If textureFormat is QImage::Format_Indexed8
 * and the textureWidth value is not divisible by four, padding bytes might need
 * to be added to each x-dimension line of the image to properly align it. The
 * padding bytes should indicate a fully transparent color to avoid rendering
 * artifacts. It is not guaranteed that QImage will do this automatically.
 *
 * \sa textureData, renderSlice()
 */
void QCustom3DVolume::setSubTextureData(Qt::Axis axis, int index, const QImage &image)
{
    Q_D(QCustom3DVolume);
    int sourceWidth = image.width();
    int sourceHeight = image.height();
    int targetWidth;
    int targetHeight;
    if (axis == Qt::XAxis) {
        targetWidth = d->m_textureDepth;
        targetHeight = d->m_textureHeight;
    } else if (axis == Qt::YAxis) {
        targetWidth = d->m_textureWidth;
        targetHeight = d->m_textureDepth;
    } else {
        targetWidth = d->m_textureWidth;
        targetHeight = d->m_textureHeight;
    }

    if (sourceWidth == targetWidth && sourceHeight == targetHeight
        && (image.format() == d->m_textureFormat || d->m_textureFormat == QImage::Format_ARGB32)) {
        QImage convertedImage;
        if (d->m_textureFormat == QImage::Format_ARGB32 && image.format() != QImage::Format_ARGB32) {
            convertedImage = image.convertToFormat(QImage::Format_ARGB32);
        } else {
            convertedImage = image;
        }
        setSubTextureData(axis, index, convertedImage.bits());
    } else {
        qCWarning(lcProperties3D, "%ls invalid image size or format.",
                qUtf16Printable(QString::fromUtf8(__func__)));
    }
}

// Note: textureFormat is not a Q_PROPERTY to work around an issue in meta
// object system that doesn't allow QImage::format to be a property type.
// Qt 5.2.1 at least has this problem.

/*!
 * Sets the format of the textureData property to \a format. Only two formats
 * are supported currently:
 * QImage::Format_Indexed8 and QImage::Format_ARGB32. If an indexed format is
 * specified, colorTable must also be set. Defaults to QImage::Format_ARGB32.
 *
 * \sa colorTable, textureData
 */
void QCustom3DVolume::setTextureFormat(QImage::Format format)
{
    Q_D(QCustom3DVolume);
    if (format == QImage::Format_ARGB32 || format == QImage::Format_Indexed8) {
        if (d->m_textureFormat == format) {
            qCDebug(lcProperties3D) << __FUNCTION__
                                    << "value is already set to:" << format;
            return;
        }
        d->m_textureFormat = format;
        d->m_dirtyBitsVolume.textureFormatDirty = true;
        emit textureFormatChanged(format);
        emit needUpdate();
    } else {
        qCWarning(lcProperties3D, "%ls attempted to set invalid texture format.",
                 qUtf16Printable(QString::fromUtf8(__func__)));
    }
}

/*!
 * Returns the format of the textureData property value.
 *
 * \sa setTextureFormat()
 */
QImage::Format QCustom3DVolume::textureFormat() const
{
    Q_D(const QCustom3DVolume);
    return d->m_textureFormat;
}

/*!
 * \fn void QCustom3DVolume::textureFormatChanged(QImage::Format format)
 *
 * This signal is emitted when the \a format of the textureData value changes.
 *
 * \sa setTextureFormat()
 */

/*!
 * \property QCustom3DVolume::alphaMultiplier
 *
 * \brief The value that the alpha value of every texel of the volume texture is
 * multiplied with at the render time.
 *
 * This property can be used to introduce uniform transparency to the volume.
 * If preserveOpacity is \c{true}, only texels with at least some transparency
 * to begin with are affected, and fully opaque texels are not affected. The
 * value must not be negative. Defaults to \c{1.0f}.
 *
 * \sa preserveOpacity, textureData
 */
void QCustom3DVolume::setAlphaMultiplier(float mult)
{
    Q_D(QCustom3DVolume);
    if (mult >= 0.0f) {
        if (qFuzzyCompare(d->m_alphaMultiplier + 1, mult + 1)) {
            qCDebug(lcProperties3D, "%s value is already set to: %.1f",
                    qUtf8Printable(QLatin1String(__FUNCTION__)), mult);
            return;
        }
        d->m_alphaMultiplier = mult;
        d->m_dirtyBitsVolume.alphaDirty = true;
        emit alphaMultiplierChanged(mult);
        emit needUpdate();
    } else {
        qCWarning(lcProperties3D, "%ls Attempted to set negative multiplier.",
                 qUtf16Printable(QString::fromUtf8(__func__)));
    }
}

float QCustom3DVolume::alphaMultiplier() const
{
    Q_D(const QCustom3DVolume);
    return d->m_alphaMultiplier;
}

/*!
 * \property QCustom3DVolume::preserveOpacity
 *
 * \brief Whether the alpha multiplier is applied to all texels.
 *
 * If this property value is \c{true}, alphaMultiplier is only applied to texels
 * that already have some transparency. If it is \c{false}, the multiplier is
 * applied to the alpha value of all texels. Defaults to \c{true}.
 *
 * \sa alphaMultiplier
 */
void QCustom3DVolume::setPreserveOpacity(bool enable)
{
    Q_D(QCustom3DVolume);
    if (d->m_preserveOpacity == enable) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << enable;
        return;
    }

    d->m_preserveOpacity = enable;
    d->m_dirtyBitsVolume.alphaDirty = true;
    emit preserveOpacityChanged(enable);
    emit needUpdate();
}

bool QCustom3DVolume::preserveOpacity() const
{
    Q_D(const QCustom3DVolume);
    return d->m_preserveOpacity;
}

/*!
 * \property QCustom3DVolume::useHighDefShader
 *
 * \brief Whether a high or low definition shader is used to render the volume.
 *
 * If this property value is \c{true}, a high definition shader is used.
 * If it is \c{false}, a low definition shader is used.
 *
 * The high definition shader guarantees that every visible texel of the volume
 * texture is sampled when the volume is rendered. The low definition shader
 * renders only a rough approximation of the volume contents, but at a much
 * higher frame rate. The low definition shader does not guarantee that every
 * texel of the volume texture is sampled, so there may be flickering if the
 * volume contains distinct thin features.
 *
 * \note This value does not affect the level of detail when rendering the
 * slices of the volume.
 *
 * Defaults to \c{true}.
 *
 * \sa renderSlice()
 */
void QCustom3DVolume::setUseHighDefShader(bool enable)
{
    Q_D(QCustom3DVolume);
    if (d->m_useHighDefShader == enable) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << enable;
        return;
    }

    d->m_useHighDefShader = enable;
    d->m_dirtyBitsVolume.shaderDirty = true;
    emit useHighDefShaderChanged(enable);
    emit needUpdate();
}

bool QCustom3DVolume::useHighDefShader() const
{
    Q_D(const QCustom3DVolume);
    return d->m_useHighDefShader;
}

/*!
 * \property QCustom3DVolume::drawSlices
 *
 * \brief Whether the specified slices are drawn instead of the full volume.
 *
 * If this property value is \c{true}, the slices indicated by slice index
 * properties will be drawn instead of the full volume. If it is \c{false}, the
 * full volume will always be drawn. Defaults to \c{false}.
 *
 * \note The slices are always drawn along the item axes, so if the item is
 * rotated, the slices are rotated as well.
 *
 * \sa sliceIndexX, sliceIndexY, sliceIndexZ
 */
void QCustom3DVolume::setDrawSlices(bool enable)
{
    Q_D(QCustom3DVolume);
    if (d->m_drawSlices == enable) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << enable;
        return;
    }
    d->m_drawSlices = enable;
    d->m_dirtyBitsVolume.slicesDirty = true;
    emit drawSlicesChanged(enable);
    emit needUpdate();
}

bool QCustom3DVolume::drawSlices() const
{
    Q_D(const QCustom3DVolume);
    return d->m_drawSlices;
}

/*!
 * \property QCustom3DVolume::drawSliceFrames
 *
 * \brief Whether slice frames are drawn around the volume.
 *
 * If this property value is \c{true}, the frames of slices indicated by slice
 * index properties will be drawn around the volume. If it is \c{false}, no
 * slice frames will be drawn.
 *
 * Drawing slice frames is independent of drawing slices, so you can show the
 * full volume and still draw the slice frames around it. This is useful when
 * using renderSlice() to display the slices outside the graph itself.
 *
 * Defaults to \c{false}.
 *
 * \sa sliceIndexX, sliceIndexY, sliceIndexZ, drawSlices, renderSlice()
 */
void QCustom3DVolume::setDrawSliceFrames(bool enable)
{
    Q_D(QCustom3DVolume);
    if (d->m_drawSliceFrames == enable) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << enable;
        return;
    }

    d->m_drawSliceFrames = enable;
    d->m_dirtyBitsVolume.slicesDirty = true;
    emit drawSliceFramesChanged(enable);
    emit needUpdate();
}

bool QCustom3DVolume::drawSliceFrames() const
{
    Q_D(const QCustom3DVolume);
    return d->m_drawSliceFrames;
}

/*!
 * \property QCustom3DVolume::sliceFrameColor
 *
 * \brief The color of the slice frame.
 *
 * Transparent slice frame color is not supported.
 *
 * Defaults to black.
 *
 * \sa drawSliceFrames
 */
void QCustom3DVolume::setSliceFrameColor(QColor color)
{
    Q_D(QCustom3DVolume);
    if (d->m_sliceFrameColor == color) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << color;
        return;
    }

    d->m_sliceFrameColor = color;
    d->m_dirtyBitsVolume.slicesDirty = true;
    emit sliceFrameColorChanged(color);
    emit needUpdate();
}

QColor QCustom3DVolume::sliceFrameColor() const
{
    Q_D(const QCustom3DVolume);
    return d->m_sliceFrameColor;
}

/*!
 * \property QCustom3DVolume::sliceFrameWidths
 *
 * \brief The width of the slice frame.
 *
 * The width can be different on different dimensions,
 * so you can for example omit drawing the frames on certain sides of the volume
 * by setting the value for that dimension to zero. The values are fractions of
 * the volume thickness in the same dimension. The values cannot be negative.
 *
 * Defaults to \c{QVector3D(0.01, 0.01, 0.01)}.
 *
 * \sa drawSliceFrames
 */
void QCustom3DVolume::setSliceFrameWidths(QVector3D values)
{
    Q_D(QCustom3DVolume);
    if (values.x() < 0.0f || values.y() < 0.0f || values.z() < 0.0f) {
        qCWarning(lcProperties3D, "%ls attempted to set negative values.",
                 qUtf16Printable(QString::fromUtf8(__func__)));
        return;
    } else if (d->m_sliceFrameWidths == values) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << values;
        return;
    }

    d->m_sliceFrameWidths = values;
    d->m_dirtyBitsVolume.slicesDirty = true;
    emit sliceFrameWidthsChanged(values);
    emit needUpdate();
}

QVector3D QCustom3DVolume::sliceFrameWidths() const
{
    Q_D(const QCustom3DVolume);
    return d->m_sliceFrameWidths;
}

/*!
 * \property QCustom3DVolume::sliceFrameGaps
 *
 * \brief The size of the air gap left between the volume itself and the frame
 * in each dimension.
 *
 * The gap can be different on different dimensions. The values are fractions of
 * the volume thickness in the same dimension. The values cannot be negative.
 *
 * Defaults to \c{QVector3D(0.01, 0.01, 0.01)}.
 *
 * \sa drawSliceFrames
 */
void QCustom3DVolume::setSliceFrameGaps(QVector3D values)
{
    Q_D(QCustom3DVolume);
    if (values.x() < 0.0f || values.y() < 0.0f || values.z() < 0.0f) {
        qCWarning(lcProperties3D, "%ls attempted to set negative values.",
                 qUtf16Printable(QString::fromUtf8(__func__)));
        return;
    } else if (d->m_sliceFrameGaps == values) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << values;
        return;
    }

    d->m_sliceFrameGaps = values;
    d->m_dirtyBitsVolume.slicesDirty = true;
    emit sliceFrameGapsChanged(values);
    emit needUpdate();
}

QVector3D QCustom3DVolume::sliceFrameGaps() const
{
    Q_D(const QCustom3DVolume);
    return d->m_sliceFrameGaps;
}

/*!
 * \property QCustom3DVolume::sliceFrameThicknesses
 *
 * \brief The thickness of the slice frames for each dimension.
 *
 * The values are fractions of
 * the volume thickness in the same dimension. The values cannot be negative.
 *
 * Defaults to \c{QVector3D(0.01, 0.01, 0.01)}.
 *
 * \sa drawSliceFrames
 */
void QCustom3DVolume::setSliceFrameThicknesses(QVector3D values)
{
    Q_D(QCustom3DVolume);
    if (values.x() < 0.0f || values.y() < 0.0f || values.z() < 0.0f) {
        qCWarning(lcProperties3D, "%ls attempted to set negative values.",
                 qUtf16Printable(QString::fromUtf8(__func__)));
        return;
    } else if (d->m_sliceFrameThicknesses == values) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << values;
        return;
    }

    d->m_sliceFrameThicknesses = values;
    d->m_dirtyBitsVolume.slicesDirty = true;
    emit sliceFrameThicknessesChanged(values);
    emit needUpdate();
}

QVector3D QCustom3DVolume::sliceFrameThicknesses() const
{
    Q_D(const QCustom3DVolume);
    return d->m_sliceFrameThicknesses;
}

/*!
 * Renders the slice specified by \a index along the axis specified by \a axis
 * into an image.
 * The texture format of this object is used.
 *
 * Returns the rendered image of the slice, or a null image if an invalid index
 * is specified.
 *
 * \sa setTextureFormat()
 */
QImage QCustom3DVolume::renderSlice(Qt::Axis axis, int index)
{
    Q_D(QCustom3DVolume);
    return d->renderSlice(axis, index);
}

QCustom3DVolumePrivate::QCustom3DVolumePrivate()
    : m_textureWidth(0)
    , m_textureHeight(0)
    , m_textureDepth(0)
    , m_sliceIndexX(-1)
    , m_sliceIndexY(-1)
    , m_sliceIndexZ(-1)
    , m_textureFormat(QImage::Format_ARGB32)
    , m_textureData(0)
    , m_alphaMultiplier(1.0f)
    , m_preserveOpacity(true)
    , m_useHighDefShader(true)
    , m_drawSlices(false)
    , m_drawSliceFrames(false)
    , m_sliceFrameColor(Qt::black)
    , m_sliceFrameWidths(QVector3D(0.01f, 0.01f, 0.01f))
    , m_sliceFrameGaps(QVector3D(0.01f, 0.01f, 0.01f))
    , m_sliceFrameThicknesses(QVector3D(0.01f, 0.01f, 0.01f))
{
    m_isVolumeItem = true;
    m_meshFile = QStringLiteral(":/defaultMeshes/barMeshFull");
}

QCustom3DVolumePrivate::QCustom3DVolumePrivate(QVector3D position,
                                               QVector3D scaling,
                                               const QQuaternion &rotation,
                                               int textureWidth,
                                               int textureHeight,
                                               int textureDepth,
                                               QList<uchar> *textureData,
                                               QImage::Format textureFormat,
                                               const QList<QRgb> &colorTable)
    : QCustom3DItemPrivate(QStringLiteral(":/defaultMeshes/barMeshFull"),
                           position,
                           scaling,
                           rotation)
    , m_textureWidth(textureWidth)
    , m_textureHeight(textureHeight)
    , m_textureDepth(textureDepth)
    , m_sliceIndexX(-1)
    , m_sliceIndexY(-1)
    , m_sliceIndexZ(-1)
    , m_textureFormat(textureFormat)
    , m_colorTable(colorTable)
    , m_textureData(textureData)
    , m_alphaMultiplier(1.0f)
    , m_preserveOpacity(true)
    , m_useHighDefShader(true)
    , m_drawSlices(false)
    , m_drawSliceFrames(false)
    , m_sliceFrameColor(Qt::black)
    , m_sliceFrameWidths(QVector3D(0.01f, 0.01f, 0.01f))
    , m_sliceFrameGaps(QVector3D(0.01f, 0.01f, 0.01f))
    , m_sliceFrameThicknesses(QVector3D(0.01f, 0.01f, 0.01f))
{
    m_isVolumeItem = true;
    m_shadowCasting = false;

    if (m_textureWidth < 0)
        m_textureWidth = 0;
    if (m_textureHeight < 0)
        m_textureHeight = 0;
    if (m_textureDepth < 0)
        m_textureDepth = 0;

    if (m_textureFormat != QImage::Format_Indexed8)
        m_textureFormat = QImage::Format_ARGB32;
}

QCustom3DVolumePrivate::~QCustom3DVolumePrivate()
{
    delete m_textureData;
}

void QCustom3DVolumePrivate::resetDirtyBits()
{
    QCustom3DItemPrivate::resetDirtyBits();

    m_dirtyBitsVolume.textureDimensionsDirty = false;
    m_dirtyBitsVolume.slicesDirty = false;
    m_dirtyBitsVolume.colorTableDirty = false;
    m_dirtyBitsVolume.textureDataDirty = false;
    m_dirtyBitsVolume.textureFormatDirty = false;
    m_dirtyBitsVolume.alphaDirty = false;
    m_dirtyBitsVolume.shaderDirty = false;
}

QImage QCustom3DVolumePrivate::renderSlice(Qt::Axis axis, int index)
{
    Q_Q(QCustom3DVolume);
    if (index < 0)
        return QImage();

    Q_TRACE_SCOPE(QGraphs3DCustomVolumeRenderSlice, axis, index);
    int x;
    int y;
    if (axis == Qt::XAxis) {
        if (index >= m_textureWidth)
            return QImage();
        x = m_textureDepth;
        y = m_textureHeight;
    } else if (axis == Qt::YAxis) {
        if (index >= m_textureHeight)
            return QImage();
        x = m_textureWidth;
        y = m_textureDepth;
    } else {
        if (index >= m_textureDepth)
            return QImage();
        x = m_textureWidth;
        y = m_textureHeight;
    }

    int padding = 0;
    int pixelWidth = 4;
    int dataWidth = q->textureDataWidth();
    if (m_textureFormat == QImage::Format_Indexed8) {
        padding = x % 4;
        pixelWidth = 1;
    }
    QList<uchar> data((x + padding) * y * pixelWidth);
    int frameSize = q->textureDataWidth() * m_textureHeight;

    int dataIndex = 0;
    if (axis == Qt::XAxis) {
        for (int i = 0; i < y; i++) {
            const uchar *p = m_textureData->constData() + (index * pixelWidth) + (dataWidth * i);
            for (int j = 0; j < x; j++) {
                for (int k = 0; k < pixelWidth; k++)
                    data[dataIndex++] = *(p + k);
                p += frameSize;
            }
        }
    } else if (axis == Qt::YAxis) {
        for (int i = y - 1; i >= 0; i--) {
            const uchar *p = m_textureData->constData() + (index * dataWidth) + (frameSize * i);
            for (int j = 0; j < (x * pixelWidth); j++) {
                data[dataIndex++] = *p;
                p++;
            }
        }
    } else {
        for (int i = 0; i < y; i++) {
            const uchar *p = m_textureData->constData() + (index * frameSize) + (dataWidth * i);
            for (int j = 0; j < (x * pixelWidth); j++) {
                data[dataIndex++] = *p;
                p++;
            }
        }
    }

    if (m_textureFormat != QImage::Format_Indexed8 && m_alphaMultiplier != 1.0f) {
        for (int i = pixelWidth - 1; i < data.size(); i += pixelWidth)
            data[i] = static_cast<uchar>(multipliedAlphaValue(data.at(i)));
    }

    QImage image(data.constData(), x, y, x * pixelWidth, m_textureFormat);
    image.bits(); // Call bits() to detach the new image from local data
    if (m_textureFormat == QImage::Format_Indexed8) {
        QList<QRgb> colorTable = m_colorTable;
        if (m_alphaMultiplier != 1.0f) {
            for (int i = 0; i < colorTable.size(); i++) {
                QRgb curCol = colorTable.at(i);
                int alpha = multipliedAlphaValue(qAlpha(curCol));
                if (alpha != qAlpha(curCol))
                    colorTable[i] = qRgba(qRed(curCol), qGreen(curCol), qBlue(curCol), alpha);
            }
        }
        image.setColorTable(colorTable);
    }

    return image;
}

int QCustom3DVolumePrivate::multipliedAlphaValue(int alpha)
{
    int modifiedAlpha = alpha;
    if (!m_preserveOpacity || alpha != 255) {
        modifiedAlpha = int(m_alphaMultiplier * float(alpha));
        modifiedAlpha = qMin(modifiedAlpha, 255);
    }
    return modifiedAlpha;
}

QT_END_NAMESPACE

#include "moc_qcustom3dvolume.cpp"
