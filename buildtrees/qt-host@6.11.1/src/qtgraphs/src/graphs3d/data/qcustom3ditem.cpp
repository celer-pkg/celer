// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtCore/qfileinfo.h>
#include "qcustom3ditem_p.h"
#include "qgraphs3dlogging_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \class QCustom3DItem
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief The QCustom3DItem class adds a custom item to a graph.
 *
 * A custom item has a custom mesh, position, scaling, rotation, and an optional
 * texture.
 *
 * \sa Q3DGraphsWidgetItem::addCustomItem()
 */

/*!
 * \qmltype Custom3DItem
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \nativetype QCustom3DItem
 * \brief Adds a custom item to a graph.
 *
 * A custom item has a custom mesh, position, scaling, rotation, and an optional
 * texture.
 */

/*! \qmlproperty string Custom3DItem::meshFile
 *
 * The item mesh file name. The item in the file must be mesh format.
 * The mesh files are recommended to include vertices, normals, and UVs.
 */

/*! \qmlproperty string Custom3DItem::textureFile
 *
 * The texture file name for the item. If left unset, a solid gray texture will
 * be used.
 *
 * \note To conserve memory, the QImage loaded from the file is cleared after a
 * texture is created.
 */

/*! \qmlproperty vector3d Custom3DItem::position
 *
 * The item position as a \l [QtQuick] vector3d. Defaults to \c {vector3d(0.0,
 * 0.0, 0.0)}.
 *
 * Item position is specified either in data coordinates or in absolute
 * coordinates, depending on the value of the positionAbsolute property. When
 * using absolute coordinates, values between \c{-1.0...1.0} are
 * within axis ranges.
 *
 * \note Items positioned outside any axis range are not rendered if
 * positionAbsolute is \c{false}, unless the item is a Custom3DVolume that would
 * be partially visible and scalingAbsolute is also \c{false}. In that case, the
 * visible portion of the volume will be rendered.
 *
 * \sa positionAbsolute, scalingAbsolute
 */

/*! \qmlproperty bool Custom3DItem::positionAbsolute
 *
 * Defines whether item position is to be handled in data coordinates or in
 * absolute coordinates. Defaults to \c{false}. Items with absolute coordinates
 * will always be rendered, whereas items with data coordinates are only
 * rendered if they are within axis ranges.
 *
 * \sa position
 */

/*! \qmlproperty vector3d Custom3DItem::scaling
 *
 * The item scaling as a \l [QtQuick] vector3d type. Defaults to
 * \c {vector3d(0.1, 0.1, 0.1)}.
 *
 * Item scaling is specified either in data values or in absolute values,
 * depending on the value of the scalingAbsolute property. The default vector
 * interpreted as absolute values sets the item to
 * 10% of the height of the graph, provided the item mesh is normalized and the
 * graph aspect ratios have not been changed from the defaults.
 *
 * \note Only absolute scaling is supported for Custom3DLabel items or for
 * custom items used in \l{GraphsItem3D::polar}{polar} graphs.
 *
 * \note In Qt 6.8 models were incorrectly assumed to be scaled to a size of 1 (-0.5...0.5)
 * by default, when they in reality are scaled to the size of 2 (-1...1). Because of this, all
 * custom items from Qt 6.9 onwards are twice the size compared to Qt 6.8
 *
 * \sa scalingAbsolute
 */

/*! \qmlproperty bool Custom3DItem::scalingAbsolute
 *
 * Defines whether item scaling is to be handled in data values or in absolute
 * values. Defaults to \c{true}. Items with absolute scaling will be rendered at
 * the same size, regardless of axis ranges. Items with data scaling will change
 * their apparent size according to the axis ranges. If positionAbsolute is
 * \c{true}, this property is ignored and scaling is interpreted as an absolute
 * value. If the item has rotation, the data scaling is calculated on the
 * unrotated item. Similarly, for Custom3DVolume items, the range clipping is
 * calculated on the unrotated item.
 *
 * \note Only absolute scaling is supported for Custom3DLabel items or for
 * custom items used in \l{GraphsItem3D::polar}{polar} graphs.
 *
 * \note The custom item's mesh must be normalized to the range \c{[-1 ,1]}, or
 * the data scaling will not be accurate.
 *
 * \sa scaling, positionAbsolute
 */

/*! \qmlproperty quaternion Custom3DItem::rotation
 *
 * The item rotation as a \l [QtQuick] quaternion. Defaults to
 * \c {quaternion(0.0, 0.0, 0.0, 0.0)}.
 */

/*! \qmlproperty bool Custom3DItem::rotationAbsolute
 *  \since 6.11
 *
 * Defines whether item rotation is to be handled in data values or in absolute
 * values. Defaults to \c{true}. Items with absolute rotation will be rotated with
 * the default coordinates, regardless of axis. Items with data rotation will rotate
 * according to the axis coordinates.
 *
 * \sa rotation
 */

/*! \qmlproperty bool Custom3DItem::visible
 *
 * The visibility of the item. Defaults to \c{true}.
 */

/*! \qmlproperty bool Custom3DItem::shadowCasting
 *
 * Defines whether shadow casting for the item is enabled. Defaults to \c{true}.
 * If \c{false}, the item does not cast shadows regardless of
 * \l{QtGraphs3D::ShadowQuality}{ShadowQuality}.
 */

/*!
 * \qmlmethod void Custom3DItem::setRotationAxisAndAngle(vector3d axis, real
 * angle)
 *
 * A convenience function to construct the rotation quaternion from \a axis and
 * \a angle.
 *
 * \sa rotation
 */

/*!
    \qmlsignal Custom3DItem::meshFileChanged(string meshFile)

    This signal is emitted when meshFile changes to \a meshFile.
*/
/*!
    \qmlsignal Custom3DItem::textureFileChanged(string textureFile)

    This signal is emitted when textureFile changes to \a textureFile.
*/
/*!
    \qmlsignal Custom3DItem::positionChanged(vector3d position)

    This signal is emitted when item \l position changes to \a position.
*/
/*!
    \qmlsignal Custom3DItem::positionAbsoluteChanged(bool positionAbsolute)

    This signal is emitted when positionAbsolute changes to \a positionAbsolute.
*/
/*!
    \qmlsignal Custom3DItem::scalingChanged(vector3d scaling)

    This signal is emitted when \l scaling changes to \a scaling.
*/
/*!
    \qmlsignal Custom3DItem::rotationChanged(quaternion rotation)

    This signal is emitted when \l rotation changes to \a rotation.
*/
/*!
    \qmlsignal Custom3DItem::visibleChanged(bool visible)

    This signal is emitted when \l visible changes to \a visible.
*/
/*!
    \qmlsignal Custom3DItem::shadowCastingChanged(bool shadowCasting)

    This signal is emitted when shadowCasting changes to \a shadowCasting.
*/
/*!
    \qmlsignal Custom3DItem::scalingAbsoluteChanged(bool scalingAbsolute)

    This signal is emitted when scalingAbsolute changes to \a scalingAbsolute.
*/

/*!
 * Constructs a custom 3D item with the specified \a parent.
 */
QCustom3DItem::QCustom3DItem(QObject *parent)
    : QObject(*(new QCustom3DItemPrivate()), parent)
{
    setTextureImage(QImage());
}

/*!
 * \internal
 */
QCustom3DItem::QCustom3DItem(QCustom3DItemPrivate &d, QObject *parent)
    : QObject(d, parent)
{
    setTextureImage(QImage());
}

/*!
 * Constructs a custom 3D item with the specified \a meshFile, \a position, \a
 * scaling, \a rotation, \a texture image, and optional \a parent.
 */
QCustom3DItem::QCustom3DItem(const QString &meshFile,
                             QVector3D position,
                             QVector3D scaling,
                             const QQuaternion &rotation,
                             const QImage &texture,
                             QObject *parent)
    : QObject(*(new QCustom3DItemPrivate(meshFile, position, scaling, rotation)), parent)
{
    setTextureImage(texture);
}

/*!
 * Deletes the custom 3D item.
 */
QCustom3DItem::~QCustom3DItem() {}

/*! \property QCustom3DItem::meshFile
 *
 * \brief The item mesh file name.
 *
 * The item in the file must be in mesh format. The other types
 * can be converted by \l {Balsam Asset Import Tool}{Balsam}
 * asset import tool. The mesh files are recommended to include
 * vertices, normals, and UVs.
 */
void QCustom3DItem::setMeshFile(const QString &meshFile)
{
    Q_D(QCustom3DItem);
    QFileInfo validfile(meshFile);

    if (!validfile.exists() || !validfile.isFile()) {
        qCWarning(lcProperties3D, "%s mesh file %s does not exist",
                qUtf8Printable(QLatin1String(__func__)), qUtf8Printable(meshFile));
        return;
    }
    if (d->m_meshFile == meshFile) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__func__)), qUtf8Printable(meshFile));
        return;
    }

    d->m_meshFile = meshFile;
    d->m_dirtyBits.meshDirty = true;
    emit meshFileChanged(meshFile);
    emit needUpdate();
}

QString QCustom3DItem::meshFile() const
{
    Q_D(const QCustom3DItem);
    return d->m_meshFile;
}

/*! \property QCustom3DItem::position
 *
 * \brief The item position as a QVector3D.
 *
 * Defaults to \c {QVector3D(0.0, 0.0, 0.0)}.
 *
 * Item position is specified either in data coordinates or in absolute
 * coordinates, depending on the
 * positionAbsolute property. When using absolute coordinates, values between
 * \c{-1.0...1.0} are within axis ranges.
 *
 * \note Items positioned outside any axis range are not rendered if
 * positionAbsolute is \c{false}, unless the item is a QCustom3DVolume that
 * would be partially visible and scalingAbsolute is also \c{false}. In that
 * case, the visible portion of the volume will be rendered.
 *
 * \sa positionAbsolute
 */
void QCustom3DItem::setPosition(QVector3D position)
{
    Q_D(QCustom3DItem);
    if (d->m_position == position) {
        qCDebug(lcProperties3D, "%s value is already set to: %.1f %.1f %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), position.x(), position.y(), position.z());
        return;
    }

    d->m_position = position;
    d->m_dirtyBits.positionDirty = true;
    emit positionChanged(position);
    emit needUpdate();
}

QVector3D QCustom3DItem::position() const
{
    Q_D(const QCustom3DItem);
    return d->m_position;
}

/*! \property QCustom3DItem::positionAbsolute
 *
 * \brief Whether item position is to be handled in data coordinates or in
 * absolute coordinates.
 *
 * Defaults to \c{false}. Items with absolute coordinates will always be
 * rendered, whereas items with data coordinates are only rendered if they are
 * within axis ranges.
 *
 * \sa position
 */
void QCustom3DItem::setPositionAbsolute(bool positionAbsolute)
{
    Q_D(QCustom3DItem);
    if (d->m_positionAbsolute == positionAbsolute) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << positionAbsolute;
        return;
    }

    d->m_positionAbsolute = positionAbsolute;
    d->m_dirtyBits.positionDirty = true;
    emit positionAbsoluteChanged(positionAbsolute);
    emit needUpdate();
}

bool QCustom3DItem::isPositionAbsolute() const
{
    Q_D(const QCustom3DItem);
    return d->m_positionAbsolute;
}

/*! \property QCustom3DItem::scaling
 *
 * \brief The item scaling as a QVector3D.
 *
 * Defaults to \c {QVector3D(0.1, 0.1, 0.1)}.
 *
 * Item scaling is either in data values or in absolute values, depending on the
 * scalingAbsolute property. The default vector interpreted as absolute values
 * sets the item to 10% of the height of the graph, provided the item mesh is
 * normalized and the graph aspect ratios have not been changed from the
 * defaults.
 *
 * \note In Qt 6.8 models were incorrectly assumed to be scaled to a size of 1 (-0.5...0.5)
 * by default, when they in reality are scaled to the size of 2 (-1...1). Because of this, all
 * custom items from Qt 6.9 onwards are twice the size compared to Qt 6.8
 *
 * \sa scalingAbsolute
 */
void QCustom3DItem::setScaling(QVector3D scaling)
{
    Q_D(QCustom3DItem);
    if (d->m_scaling == scaling) {
        qCDebug(lcProperties3D, "%s value is already set to: %.1f %.1f %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), scaling.x(), scaling.y(), scaling.z());
        return;
    }

    d->m_scaling = scaling;
    d->m_dirtyBits.scalingDirty = true;
    emit scalingChanged(scaling);
    emit needUpdate();
}

QVector3D QCustom3DItem::scaling() const
{
    Q_D(const QCustom3DItem);
    return d->m_scaling;
}

/*! \property QCustom3DItem::scalingAbsolute
 *
 * \brief Whether item scaling is to be handled in data values or in absolute
 * values.
 *
 * Defaults to \c{true}.
 *
 * Items with absolute scaling will be rendered at the same
 * size, regardless of axis ranges. Items with data scaling will change their
 * apparent size according to the axis ranges. If positionAbsolute is \c{true},
 * this property is ignored and scaling is interpreted as an absolute value. If
 * the item has rotation, the data scaling is calculated on the unrotated item.
 * Similarly, for QCustom3DVolume items, the range clipping is calculated on the
 * unrotated item.
 *
 * \note Only absolute scaling is supported for QCustom3DLabel items or for
 * custom items used in \l{Q3DGraphsWidgetItem::polar}{polar} graphs.
 *
 * \note The custom item's mesh must be normalized to the range \c{[-1 ,1]}, or
 * the data scaling will not be accurate.
 *
 * \sa scaling, positionAbsolute
 */
void QCustom3DItem::setScalingAbsolute(bool scalingAbsolute)
{
    Q_D(QCustom3DItem);
    if (d->m_isLabelItem && !scalingAbsolute) {
        qCWarning(lcProperties3D, "%ls data bounds are not supported for label items.",
                 qUtf16Printable(QString::fromUtf8(__func__)));
        return;
    } else if (d->m_scalingAbsolute == scalingAbsolute) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << scalingAbsolute;
        return;
    }

    d->m_scalingAbsolute = scalingAbsolute;
    d->m_dirtyBits.scalingDirty = true;
    emit scalingAbsoluteChanged(scalingAbsolute);
    emit needUpdate();
}

bool QCustom3DItem::isScalingAbsolute() const
{
    Q_D(const QCustom3DItem);
    return d->m_scalingAbsolute;
}

/*! \property QCustom3DItem::rotation
 *
 * \brief The item rotation as a QQuaternion.
 *
 * Defaults to \c {QQuaternion(0.0, 0.0, 0.0, 0.0)}.
 */
void QCustom3DItem::setRotation(const QQuaternion &rotation)
{
    Q_D(QCustom3DItem);
    if (d->m_rotation == rotation) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << rotation;
        return;
    }

    d->m_rotation = rotation;
    d->m_dirtyBits.rotationDirty = true;
    emit rotationChanged(rotation);
    emit needUpdate();
}

QQuaternion QCustom3DItem::rotation()
{
    Q_D(const QCustom3DItem);
    return d->m_rotation;
}

/*! \property QCustom3DItem::rotationAbsolute
 *  \since 6.11
 *
 * \brief Whether item rotation is to be handled in data axis coordinates or in absolute
 * coordinates.
 *
 * Defines whether item rotation is to be handled in data values or in absolute
 * values. Defaults to \c{true}. Items with absolute rotation will be rotated with
 * the default coordinates, regardless of axis. Items with data rotation will rotate
 * according to the axis coordinates.
 *
 * \sa rotation
 */
 void QCustom3DItem::setRotationAbsolute(bool rotationAbsolute)
 {
     Q_D(QCustom3DItem);
     if (d->m_rotationAbsolute == rotationAbsolute) {
         qCDebug(lcProperties3D) << __FUNCTION__
             << "value is already set to:" << rotationAbsolute;
         return;
     }

     d->m_rotationAbsolute = rotationAbsolute;
     d->m_dirtyBits.rotationDirty = true;
     emit rotationAbsoluteChanged(rotationAbsolute);
     emit needUpdate();
 }

 bool QCustom3DItem::isRotationAbsolute() const
 {
     Q_D(const QCustom3DItem);
     return d->m_rotationAbsolute;
 }

/*! \property QCustom3DItem::visible
 *
 * \brief The visibility of the item.
 *
 * Defaults to \c{true}.
 */
void QCustom3DItem::setVisible(bool visible)
{
    Q_D(QCustom3DItem);
    if (d->m_visible == visible) {
        qCDebug(lcProperties3D) << qUtf8Printable(QLatin1String(__FUNCTION__))
            << "value is already set to:" << visible;
        return;
    }

    d->m_visible = visible;
    d->m_dirtyBits.visibleDirty = true;
    emit visibleChanged(visible);
    emit needUpdate();
}

bool QCustom3DItem::isVisible() const
{
    Q_D(const QCustom3DItem);
    return d->m_visible;
}

/*! \property QCustom3DItem::shadowCasting
 *
 * \brief Whether shadow casting for the item is enabled.
 *
 * Defaults to \c{true}.
 * If \c{false}, the item does not cast shadows regardless of
 * Q3DGraphsWidgetItem::ShadowQuality.
 */
void QCustom3DItem::setShadowCasting(bool enabled)
{
    Q_D(QCustom3DItem);
    if (d->m_shadowCasting == enabled) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << enabled;
        return;
    }

    d->m_shadowCasting = enabled;
    d->m_dirtyBits.shadowCastingDirty = true;
    emit shadowCastingChanged(enabled);
    emit needUpdate();
}

bool QCustom3DItem::isShadowCasting() const
{
    Q_D(const QCustom3DItem);
    return d->m_shadowCasting;
}

/*!
 * A convenience function to construct the rotation quaternion from \a axis and
 * \a angle.
 *
 * \sa rotation
 */
void QCustom3DItem::setRotationAxisAndAngle(QVector3D axis, float angle)
{
    setRotation(QQuaternion::fromAxisAndAngle(axis, angle));
}

/*!
 * Sets the value of \a textureImage as a QImage for the item. The texture
 * defaults to solid gray.
 *
 * \note To conserve memory, the given QImage is cleared after a texture is
 * created.
 */
void QCustom3DItem::setTextureImage(const QImage &textureImage)
{
    Q_D(QCustom3DItem);
    if (textureImage == d->m_textureImage) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << textureImage;
        return;
    }

    if (textureImage.isNull()) {
        // Make a solid gray texture
        d->m_textureImage = QImage(2, 2, QImage::Format_RGB32);
        d->m_textureImage.fill(Qt::gray);
    } else {
        d->m_textureImage = textureImage;
    }

    if (!d->m_textureFile.isEmpty()) {
        d->m_textureFile.clear();
        emit textureFileChanged(d->m_textureFile);
    }
    d->m_dirtyBits.textureDirty = true;
    emit needUpdate();
}

/*! \property QCustom3DItem::textureFile
 *
 * \brief The texture file name for the item.
 *
 * If both this property and the texture image are unset, a solid
 * gray texture will be used.
 *
 * \note To conserve memory, the QImage loaded from the file is cleared after a
 * texture is created.
 */
void QCustom3DItem::setTextureFile(const QString &textureFile)
{
    Q_D(QCustom3DItem);
    if (d->m_textureFile == textureFile) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(textureFile));
        return;
    }

    d->m_textureFile = textureFile;
    if (!textureFile.isEmpty()) {
        d->m_textureImage = QImage(textureFile);
    } else {
        d->m_textureImage = QImage(2, 2, QImage::Format_RGB32);
        d->m_textureImage.fill(Qt::gray);
        qCWarning(lcProperties3D, "%s texture file was empty, texture defaults to grey", qUtf8Printable(textureFile));
    }
    emit textureFileChanged(textureFile);
    d->m_dirtyBits.textureDirty = true;
    emit needUpdate();
}

QString QCustom3DItem::textureFile() const
{
    Q_D(const QCustom3DItem);
    return d->m_textureFile;
}

QCustom3DItemPrivate::QCustom3DItemPrivate()
    : m_textureImage(QImage(1, 1, QImage::Format_ARGB32))
    , m_position(QVector3D(0.0f, 0.0f, 0.0f))
    , m_positionAbsolute(false)
    , m_scaling(QVector3D(0.1f, 0.1f, 0.1f))
    , m_scalingAbsolute(true)
    , m_rotation(QQuaternion())
    , m_visible(true)
    , m_shadowCasting(true)
    , m_isLabelItem(false)
    , m_isVolumeItem(false)
{}

QCustom3DItemPrivate::QCustom3DItemPrivate(const QString &meshFile,
                                           QVector3D position,
                                           QVector3D scaling,
                                           const QQuaternion &rotation)
    : m_textureImage(QImage(1, 1, QImage::Format_ARGB32))
    , m_meshFile(meshFile)
    , m_position(position)
    , m_positionAbsolute(false)
    , m_scaling(scaling)
    , m_scalingAbsolute(true)
    , m_rotation(rotation)
    , m_rotationAbsolute(true)
    , m_visible(true)
    , m_shadowCasting(true)
    , m_isLabelItem(false)
    , m_isVolumeItem(false)
{}

QCustom3DItemPrivate::~QCustom3DItemPrivate() {}

QImage QCustom3DItemPrivate::textureImage()
{
    return m_textureImage;
}

void QCustom3DItemPrivate::clearTextureImage()
{
    m_textureImage = QImage();
    m_textureFile.clear();
}

void QCustom3DItemPrivate::resetDirtyBits()
{
    m_dirtyBits.textureDirty = false;
    m_dirtyBits.meshDirty = false;
    m_dirtyBits.positionDirty = false;
    m_dirtyBits.scalingDirty = false;
    m_dirtyBits.rotationDirty = false;
    m_dirtyBits.visibleDirty = false;
    m_dirtyBits.shadowCastingDirty = false;
}

QT_END_NAMESPACE

#include "moc_qcustom3ditem.cpp"
