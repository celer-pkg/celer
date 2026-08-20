// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qabstract3dseries_p.h"
#include "qabstractdataproxy_p.h"
#include "qquickgraphsitem_p.h"
#include "utils_p.h"
#include "qgraphs3dlogging_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \class QAbstract3DSeries
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief The QAbstract3DSeries class is a base class for all 3D data series.
 *
 * There are inherited classes for each supported series type: QBar3DSeries,
 * QScatter3DSeries, and QSurface3DSeries.
 *
 * For more information, see \l{Qt Graphs Data Handling with 3D}.
 */

/*!
 * \class QAbstract3DSeriesChangeBitField
 * \internal
 */

/*!
 * \class QAbstract3DSeriesThemeOverrideBitField
 * \internal
 */

/*!
 * \qmltype Abstract3DSeries
 * \qmlabstract
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \nativetype QAbstract3DSeries
 * \brief A base type for all 3D data series.
 *
 * This abstract class serves as a base class for the following subtypes:
 * Bar3DSeries, Scatter3DSeries, and Surface3DSeries.
 *
 * For more information, see \l{Qt Graphs Data Handling with 3D}.
 */

/*!
 * \enum QAbstract3DSeries::SeriesType
 *
 * Type of the series.
 *
 * \value None
 *        No series type.
 * \value Bar
 *        Series type for Q3DBarsWidgetItem.
 * \value Scatter
 *        Series type for Q3DScatterWidgetItem.
 * \value Surface
 *        Series type for Q3DSurfaceWidgetItem.
 */


/*!
 *  \enum QAbstract3DSeries::Mesh
 *
 *  Predefined mesh types. All styles are not usable with all graphs types.
 *
 *  \value UserDefined
 *         User defined mesh, set via QAbstract3DSeries::userDefinedMesh property.
 *  \value Bar
 *         Basic rectangular bar.
 *  \value Cube
 *         Basic cube.
 *  \value Pyramid
 *         Four-sided pyramid.
 *  \value Cone
 *         Basic cone.
 *  \value Cylinder
 *         Basic cylinder.
 *  \value BevelBar
 *         Slightly beveled (rounded) rectangular bar.
 *  \value BevelCube
 *         Slightly beveled (rounded) cube.
 *  \value Sphere
 *         Sphere.
 *  \value Minimal
 *         The minimal 3D mesh: a triangular pyramid. Usable only with Q3DScatterWidgetItem.
 *  \value Arrow
 *         Arrow pointing upwards.
 *  \value Point
 *         2D point. Usable only with Q3DScatterWidgetItem.
 *         Shadows do not affect this style. Color style QGraphsTheme::ColorStyle::ObjectGradient
 *         is not supported by this style.
 */

/*!
 *  \enum QAbstract3DSeries::LightingMode
 *
 *  Predefined lighting modes
 *
 *  \value Shaded
 *      Graphs respond to real time lighting
 *  \value Unshaded
 *      Graphs do not respond to real time lighting
 */

/*!
 * \qmlproperty Abstract3DSeries.SeriesType Abstract3DSeries::type
 * The type of the series. One of the QAbstract3DSeries::SeriesType values.
 *
 */

/*!
 * \qmlproperty string Abstract3DSeries::itemLabelFormat
 *
 * The label format for data items in this series. This format is used for
 * single item labels, for example, when an item is selected. How the format is
 * interpreted depends on series type.
 *
 * \sa Bar3DSeries, Scatter3DSeries, Surface3DSeries.
 */

/*!
 * \qmlproperty bool Abstract3DSeries::visible
 * The visibility of the series. If \c false, the series is not rendered.
 */

/*!
 * \qmlproperty Abstract3DSeries.Mesh Abstract3DSeries::mesh
 *
 * The mesh of the items in the series, or the selection pointer in case of
 * Surface3DSeries. If the mesh is
 * \l{QAbstract3DSeries::Mesh::UserDefined}{Abstract3DSeries.Mesh.UserDefined},
 * then the userDefinedMesh property must also be set for items to render
 * properly. The default value depends on the graph type.
 *
 * \sa QAbstract3DSeries::Mesh
 */

/*!
 * \qmlproperty bool Abstract3DSeries::meshSmooth
 *
 * If \c true, smooth versions of predefined meshes set via the \l mesh property
 * are used. This property does not affect custom meshes used when the mesh is
 * set to
 * \l{QAbstract3DSeries::Mesh::UserDefined}{Abstract3DSeries.Mesh.UserDefined}.
 * Defaults to \c{false}.
 */

/*!
 * \qmlproperty quaternion Abstract3DSeries::meshRotation
 *
 * The mesh rotation that is applied to all items of the series.
 * The rotation should be a normalized quaternion.
 * For those series types that support item specific rotation, the rotations are
 * multiplied together.
 * Bar3DSeries ignores any rotation that is not around the y-axis.
 * Surface3DSeries applies the rotation only to the selection pointer.
 * Defaults to no rotation.
 */

/*!
 * \qmlproperty string Abstract3DSeries::userDefinedMesh
 *
 * The filename for a user defined custom mesh for objects that is used
 * when \l mesh is
 * \l{QAbstract3DSeries::Mesh::UserDefined}{Abstract3DSeries.Mesh.UserDefined}.
 * \note The file needs to be in the QtQuick3D mesh format. Use the \c balsam
 * conversion tool to create a mesh from other 3D model formats.
 */

/*!
 * \qmlproperty GraphsTheme.ColorStyle Abstract3DSeries::colorStyle
 *
 * The color style for the series.
 *
 * \sa {QGraphsTheme::ColorStyle}{GraphsTheme.ColorStyle}
 */

/*!
 * \qmlproperty color Abstract3DSeries::baseColor
 *
 * The base color of the series.
 *
 * \sa colorStyle, {GraphsTheme::seriesColors}{GraphsTheme.seriesColors}
 */

/*!
 * \qmlproperty Gradient Abstract3DSeries::baseGradient
 *
 * The base gradient of the series.
 *
 * \sa colorStyle
 */

/*!
 * \qmlproperty color Abstract3DSeries::singleHighlightColor
 *
 * The single item highlight color of the series.
 *
 * \sa colorStyle, {GraphsTheme::singleHighlightColor}{GraphsTheme.singleHighlightColor}
 */

/*!
 * \qmlproperty Gradient Abstract3DSeries::singleHighlightGradient
 *
 * The single item highlight gradient of the series.
 *
 * \sa colorStyle,
 * {GraphsTheme::singleHighlightGradient}{GraphsTheme.singleHighlightGradient}
 */

/*!
 * \qmlproperty color Abstract3DSeries::multiHighlightColor
 *
 * The multiple item highlight color of the series.
 *
 * \sa colorStyle, {GraphsTheme::multiHighlightColor}{GraphsTheme.multiHighlightColor}
 */

/*!
 * \qmlproperty Gradient Abstract3DSeries::multiHighlightGradient
 *
 * The multiple item highlight gradient of the series.
 *
 * \sa colorStyle,
 * {GraphsTheme::multiHighlightGradient}{GraphsTheme.multiHighlightGradient}
 */

/*!
 * \qmlproperty Abstract3DSeries.LightingMode Abstract3DSeries::lightingMode
 * \since 6.10
 *
 * The lighting mode of the items in the series.
 * The default value is \l{QAbstract3DSeries::LightingMode::Shaded}
 *
 * \sa QAbstract3DSeries::LightingMode
 */

/*!
 * \qmlproperty string Abstract3DSeries::name
 *
 * The series name.
 * It can be used in item label format with the tag \c{@seriesName}.
 *
 * \sa itemLabelFormat
 */

/*!
 * \qmlproperty string Abstract3DSeries::itemLabel
 *
 * The formatted item label. If there is no selected item or the selected item
 * is not visible, returns an empty string.
 *
 * \sa itemLabelFormat
 */

/*!
 * \qmlproperty bool Abstract3DSeries::itemLabelVisible
 *
 * If \c true, item labels are drawn as floating labels in the graph. Otherwise,
 * item labels are not drawn. To show the item label in an external control,
 * this property is set to \c false. Defaults to \c true.
 *
 * \sa itemLabelFormat, itemLabel
 */

/*!
 * \qmlmethod void Abstract3DSeries::setMeshAxisAndAngle(vector3d axis, real
 * angle)
 *
 * A convenience function to construct a mesh rotation quaternion from \a axis
 * and \a angle.
 *
 * \sa meshRotation
 */

/*!
    \qmlsignal Abstract3DSeries::itemLabelFormatChanged(string format)

    This signal is emitted when itemLabelFormat changes to \a format.
*/
/*!
    \qmlsignal Abstract3DSeries::visibilityChanged(bool visible)

    This signal is emitted when the series visibility changes to \a visible.
*/
/*!
    \qmlsignal Abstract3DSeries::meshChanged(Abstract3DSeries.Mesh mesh)

    This signal is emitted when \l mesh changes to \a mesh.
*/
/*!
    \qmlsignal Abstract3DSeries::meshSmoothChanged(bool enabled)

    This signal is emitted when meshSmooth changes to \a enabled.
*/
/*!
    \qmlsignal Abstract3DSeries::meshRotationChanged(quaternion rotation)

    This signal is emitted when meshRotation changes to \a rotation.
*/
/*!
    \qmlsignal Abstract3DSeries::userDefinedMeshChanged(string fileName)

    This signal is emitted when userDefinedMesh changes to \a fileName.
*/
/*!
    \qmlsignal Abstract3DSeries::colorStyleChanged(GraphsTheme.ColorStyle style)

    This signal is emitted when colorStyle changes to \a style.
*/
/*!
    \qmlsignal Abstract3DSeries::baseColorChanged(color color)

    This signal is emitted when baseColor changes to \a color.
*/
/*!
    \qmlsignal Abstract3DSeries::baseGradientChanged(Gradient gradient)

    This signal is emitted when baseGradient changes to \a gradient.
*/
/*!
    \qmlsignal Abstract3DSeries::singleHighlightColorChanged(color color)

    This signal is emitted when singleHighlightColor changes to \a color.
*/
/*!
    \qmlsignal Abstract3DSeries::singleHighlightGradientChanged(Gradient gradient)

    This signal is emitted when singleHighlightGradient changes to \a gradient.
*/
/*!
    \qmlsignal Abstract3DSeries::multiHighlightColorChanged(color color)

    This signal is emitted when multiHighlightColor changes to \a color.
*/
/*!
    \qmlsignal Abstract3DSeries::multiHighlightGradientChanged(Gradient gradient)

    This signal is emitted when multiHighlightGradient changes to \a gradient.
*/
/*!
    \qmlsignal Abstract3DSeries::lightingModeChanged(Abstract3DSeries.LightingMode lightingMode)

    This signal is emitted when \l lightingMode changes to \a lightingMode.
    \since 6.10
*/
/*!
    \qmlsignal Abstract3DSeries::nameChanged(string name)

    This signal is emitted when \l name changes to \a name.
*/
/*!
    \qmlsignal Abstract3DSeries::itemLabelChanged(string label)

    This signal is emitted when itemLabel changes to \a label.
*/
/*!
    \qmlsignal Abstract3DSeries::itemLabelVisibilityChanged(bool visible)

    This signal is emitted when itemLabelVisibility changes to \a visible.
*/

/*!
 * \internal
 */
QAbstract3DSeries::QAbstract3DSeries(QAbstract3DSeriesPrivate &d, QObject *parent)
    : QObject(d, parent)
{}

/*!
 * Deletes the abstract 3D series.
 */
QAbstract3DSeries::~QAbstract3DSeries() {}

/*!
 * \property QAbstract3DSeries::type
 *
 * \brief The type of the series.
 */
QAbstract3DSeries::SeriesType QAbstract3DSeries::type() const
{
    Q_D(const QAbstract3DSeries);
    return d->m_type;
}

/*!
 * \property QAbstract3DSeries::itemLabelFormat
 *
 * \brief The label format for data items in this series.
 *
 * This format is used for single item labels,
 * for example, when an item is selected. How the format is interpreted depends
 * on series type.
 *
 * \sa QBar3DSeries, QScatter3DSeries, QSurface3DSeries.
 */
void QAbstract3DSeries::setItemLabelFormat(const QString &format)
{
    Q_D(QAbstract3DSeries);
    if (d->m_itemLabelFormat == format) {
        qCDebug(lcProperties3D, "%s value: %s is same than it already was.",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(format));
        return;
    }

    d->setItemLabelFormat(format);
    emit itemLabelFormatChanged(format);
}

QString QAbstract3DSeries::itemLabelFormat() const
{
    Q_D(const QAbstract3DSeries);
    return d->m_itemLabelFormat;
}

/*!
 * \property QAbstract3DSeries::visible
 *
 * \brief The visibility of the series.
 *
 * If this property is \c false, the series is not rendered.
 * Defaults to \c{true}.
 */
void QAbstract3DSeries::setVisible(bool visible)
{
    Q_D(QAbstract3DSeries);
    if (d->m_visible == visible) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << visible;
        return;
    }

    d->setVisible(visible);
    emit visibleChanged(visible);
}

bool QAbstract3DSeries::isVisible() const
{
    Q_D(const QAbstract3DSeries);
    return d->m_visible;
}

/*!
 * \property QAbstract3DSeries::mesh
 *
 * \brief The mesh of the items in the series.
 *
 * For QSurface3DSeries, this property holds the selection pointer.
 *
 * If the mesh is MeshUserDefined, then the userDefinedMesh property
 * must also be set for items to render properly. The default value depends on
 * the graph type.
 */
void QAbstract3DSeries::setMesh(QAbstract3DSeries::Mesh mesh)
{
    Q_D(QAbstract3DSeries);
    if ((mesh == QAbstract3DSeries::Mesh::Point || mesh == QAbstract3DSeries::Mesh::Minimal
         || mesh == QAbstract3DSeries::Mesh::Arrow)
        && type() != QAbstract3DSeries::SeriesType::Scatter) {
        qCWarning(lcProperties3D, "%s specified style is only supported for QScatter3DSeries.",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
        return;
    } else if (d->m_mesh == mesh) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << mesh;
        return;
    }

    d->setMesh(mesh);
    emit meshChanged(mesh);
}

QAbstract3DSeries::Mesh QAbstract3DSeries::mesh() const
{
    Q_D(const QAbstract3DSeries);
    return d->m_mesh;
}

/*!
 * \property QAbstract3DSeries::meshSmooth
 *
 * \brief Whether smooth versions of predefined meshes are used.
 *
 * If \c true, smooth versions set via the \l mesh property are used.
 * This property does not affect custom meshes used when the mesh is set to
 * MeshUserDefined. Defaults to \c{false}.
 */
void QAbstract3DSeries::setMeshSmooth(bool enable)
{
    Q_D(QAbstract3DSeries);
    if (d->m_meshSmooth == enable) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << enable;
        return;
    }

    d->setMeshSmooth(enable);
    emit meshSmoothChanged(enable);
}

bool QAbstract3DSeries::isMeshSmooth() const
{
    Q_D(const QAbstract3DSeries);
    return d->m_meshSmooth;
}

/*!
 * \property QAbstract3DSeries::meshRotation
 *
 * \brief The mesh rotation that is applied to all items of the series.
 *
 * The rotation should be a normalized QQuaternion.
 * For those series types that support item specific rotation, the rotations are
 * multiplied together.
 * QBar3DSeries ignores any rotation that is not around the y-axis.
 * QSurface3DSeries applies the rotation only to the selection pointer.
 * Defaults to no rotation.
 */
void QAbstract3DSeries::setMeshRotation(const QQuaternion &rotation)
{
    Q_D(QAbstract3DSeries);
    if (d->m_meshRotation == rotation) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << rotation;
        return;
    }

    d->setMeshRotation(rotation);
    emit meshRotationChanged(rotation);
}

QQuaternion QAbstract3DSeries::meshRotation() const
{
    Q_D(const QAbstract3DSeries);
    return d->m_meshRotation;
}

/*!
 * A convenience function to construct a mesh rotation quaternion from
 * \a axis and \a angle.
 *
 * \sa meshRotation
 */
void QAbstract3DSeries::setMeshAxisAndAngle(QVector3D axis, float angle)
{
    setMeshRotation(QQuaternion::fromAxisAndAngle(axis, angle));
}

/*!
 * \property QAbstract3DSeries::userDefinedMesh
 *
 * \brief The filename for a user defined custom mesh for objects.
 *
 * The custom mesh is used when \l mesh is MeshUserDefined.
 * \note The file needs to be in the QtQuick3D mesh format. Use the \c balsam
 * conversion tool to create a mesh from other 3D model formats.
 */
void QAbstract3DSeries::setUserDefinedMesh(const QString &fileName)
{
    Q_D(QAbstract3DSeries);
    if (d->m_userDefinedMesh == fileName) {
        qCDebug(lcProperties3D, "%s value %s is same than what is already being used.",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(fileName));
        return;
    }

    d->setUserDefinedMesh(fileName);
    emit userDefinedMeshChanged(fileName);
}

QString QAbstract3DSeries::userDefinedMesh() const
{
    Q_D(const QAbstract3DSeries);
    return d->m_userDefinedMesh;
}

/*!
 * \property QAbstract3DSeries::colorStyle
 *
 * \brief The color style for the series.
 *
 * \sa QGraphsTheme::ColorStyle
 */
void QAbstract3DSeries::setColorStyle(QGraphsTheme::ColorStyle style)
{
    Q_D(QAbstract3DSeries);
    if (d->m_colorStyle == style) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << style;
        d->m_themeTracker.colorStyleOverride = true;
        return;
    }

    d->setColorStyle(style);
    emit colorStyleChanged(style);

    d->m_themeTracker.colorStyleOverride = true;
}

QGraphsTheme::ColorStyle QAbstract3DSeries::colorStyle() const
{
    Q_D(const QAbstract3DSeries);
    return d->m_colorStyle;
}

/*!
 * \property QAbstract3DSeries::baseColor
 *
 * \brief The base color of the series.
 *
 * \sa colorStyle, QGraphsTheme::seriesColors
 */
void QAbstract3DSeries::setBaseColor(QColor color)
{
    Q_D(QAbstract3DSeries);
    if (d->m_baseColor == color) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(color.name()));
        d->m_themeTracker.baseColorOverride = true;
        return;
    }

    d->setBaseColor(color);
    emit baseColorChanged(color);

    d->m_themeTracker.baseColorOverride = true;
}

QColor QAbstract3DSeries::baseColor() const
{
    Q_D(const QAbstract3DSeries);
    return d->m_baseColor;
}

/*!
 * \property QAbstract3DSeries::baseGradient
 *
 * \brief The base gradient of the series.
 *
 * \sa colorStyle, QGraphsTheme::seriesGradients
 */
void QAbstract3DSeries::setBaseGradient(const QLinearGradient &gradient)
{
    Q_D(QAbstract3DSeries);
    if (d->m_baseGradient == gradient) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << gradient;
        d->m_themeTracker.baseGradientOverride = true;
        return;
    }
    d->setBaseGradient(gradient);
    emit baseGradientChanged(gradient);

    d->m_themeTracker.baseGradientOverride = true;
}

QLinearGradient QAbstract3DSeries::baseGradient() const
{
    Q_D(const QAbstract3DSeries);
    return d->m_baseGradient;
}

/*!
 * \property QAbstract3DSeries::singleHighlightColor
 *
 * \brief The single item highlight color of the series.
 *
 * \sa colorStyle, QGraphsTheme::singleHighlightColor
 */
void QAbstract3DSeries::setSingleHighlightColor(QColor color)
{
    Q_D(QAbstract3DSeries);
    if (d->m_singleHighlightColor == color) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(color.name()));
        d->m_themeTracker.singleHighlightColorOverride = true;
        return;
    }

    d->setSingleHighlightColor(color);
    emit singleHighlightColorChanged(color);
    d->m_themeTracker.singleHighlightColorOverride = true;
}

QColor QAbstract3DSeries::singleHighlightColor() const
{
    Q_D(const QAbstract3DSeries);
    return d->m_singleHighlightColor;
}

/*!
 * \property QAbstract3DSeries::singleHighlightGradient
 *
 * \brief The single item highlight gradient of the series.
 *
 * \sa colorStyle, QGraphsTheme::singleHighlightGradient
 */
void QAbstract3DSeries::setSingleHighlightGradient(const QLinearGradient &gradient)
{
    Q_D(QAbstract3DSeries);
    if (d->m_singleHighlightGradient == gradient) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << gradient;
        d->m_themeTracker.singleHighlightGradientOverride = true;
        return;
    }

    d->setSingleHighlightGradient(gradient);
    emit singleHighlightGradientChanged(gradient);
    d->m_themeTracker.singleHighlightGradientOverride = true;
}

QLinearGradient QAbstract3DSeries::singleHighlightGradient() const
{
    Q_D(const QAbstract3DSeries);
    return d->m_singleHighlightGradient;
}

/*!
 * \property QAbstract3DSeries::multiHighlightColor
 *
 * \brief The multiple item highlight color of the series.
 *
 * \sa colorStyle, QGraphsTheme::multiHighlightColor
 */
void QAbstract3DSeries::setMultiHighlightColor(QColor color)
{
    Q_D(QAbstract3DSeries);
    if (d->m_multiHighlightColor == color) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(color.name()));
        d->m_themeTracker.multiHighlightColorOverride = true;
        return;
    }

    d->setMultiHighlightColor(color);
    emit multiHighlightColorChanged(color);
    d->m_themeTracker.multiHighlightColorOverride = true;
}

QColor QAbstract3DSeries::multiHighlightColor() const
{
    Q_D(const QAbstract3DSeries);
    return d->m_multiHighlightColor;
}

/*!
 * \property QAbstract3DSeries::multiHighlightGradient
 *
 * \brief The multiple item highlight gradient of the series.
 *
 * \sa colorStyle, QGraphsTheme::multiHighlightGradient
 */
void QAbstract3DSeries::setMultiHighlightGradient(const QLinearGradient &gradient)
{
    Q_D(QAbstract3DSeries);
    if (d->m_multiHighlightGradient == gradient) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << gradient;
        d->m_themeTracker.multiHighlightGradientOverride = true;
        return;
    }

    d->setMultiHighlightGradient(gradient);
    emit multiHighlightGradientChanged(gradient);
    d->m_themeTracker.multiHighlightGradientOverride = true;
}

QLinearGradient QAbstract3DSeries::multiHighlightGradient() const
{
    Q_D(const QAbstract3DSeries);
    return d->m_multiHighlightGradient;
}

/*!
 * \property QAbstract3DSeries::lightingMode
 *
 * \brief The lighting mode of the series
 * \since 6.10
 * \sa LightingMode
 */
void QAbstract3DSeries::setLightingMode(LightingMode lightingMode)
{
    Q_D(QAbstract3DSeries);
    if (d->m_lightingMode != lightingMode) {
        d->setLightingMode(lightingMode);
        emit lightingModeChanged(lightingMode);
    }
}

QAbstract3DSeries::LightingMode QAbstract3DSeries::lightingMode() const
{
    Q_D(const QAbstract3DSeries);
    return d->m_lightingMode;
}

/*!
 * \property QAbstract3DSeries::name
 *
 * \brief The series name.
 *
 * The series name can be used in item label format with the tag
 * \c{@seriesName}.
 *
 * \sa itemLabelFormat
 */
void QAbstract3DSeries::setName(const QString &name)
{
    Q_D(QAbstract3DSeries);
    if (d->m_name == name) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(name));
        return;
    }

    d->setName(name);
    emit nameChanged(name);
}

QString QAbstract3DSeries::name() const
{
    Q_D(const QAbstract3DSeries);
    return d->m_name;
}

/*!
 * \property QAbstract3DSeries::itemLabel
 *
 * \brief The formatted item label.
 *
 * If there is no selected item or the selected item is not
 * visible, returns an empty string.
 *
 * \sa itemLabelFormat
 */
QString QAbstract3DSeries::itemLabel()
{
    Q_D(QAbstract3DSeries);
    return d->itemLabel();
}

/*!
 * \property QAbstract3DSeries::itemLabelVisible
 *
 * \brief The visibility of item labels in the graph.
 *
 * If \c true, item labels are drawn as floating labels in the graph. Otherwise,
 * item labels are not drawn. To show the item label in an external control,
 * this property is set to \c false. Defaults to \c true.
 *
 * \sa itemLabelFormat, itemLabel
 */
void QAbstract3DSeries::setItemLabelVisible(bool visible)
{
    Q_D(QAbstract3DSeries);
    if (d->m_itemLabelVisible == visible) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << visible;
        return;
    }

    d->setItemLabelVisible(visible);
    emit itemLabelVisibleChanged(visible);
}

bool QAbstract3DSeries::isItemLabelVisible() const
{
    Q_D(const QAbstract3DSeries);
    return d->m_itemLabelVisible;
}

// QAbstract3DSeriesPrivate

QAbstract3DSeriesPrivate::QAbstract3DSeriesPrivate(QAbstract3DSeries::SeriesType type)
    : m_type(type)
    , m_dataProxy(0)
    , m_visible(true)
    , m_graph(0)
    , m_mesh(QAbstract3DSeries::Mesh::Cube)
    , m_meshSmooth(false)
    , m_colorStyle(QGraphsTheme::ColorStyle::Uniform)
    , m_baseColor(Qt::black)
    , m_singleHighlightColor(Qt::black)
    , m_multiHighlightColor(Qt::black)
    , m_itemLabelDirty(true)
    , m_itemLabelVisible(true)
    , m_lightingMode(QAbstract3DSeries::LightingMode::Shaded)
{}

QAbstract3DSeriesPrivate::~QAbstract3DSeriesPrivate() {}

QAbstractDataProxy *QAbstract3DSeriesPrivate::dataProxy() const
{
    return m_dataProxy;
}

void QAbstract3DSeriesPrivate::setDataProxy(QAbstractDataProxy *proxy)
{
    Q_Q(QAbstract3DSeries);
    Q_ASSERT(proxy && proxy != m_dataProxy && !proxy->d_func()->series());

    delete m_dataProxy;
    m_dataProxy = proxy;

    proxy->d_func()->setSeries(q); // also sets parent

    if (m_graph) {
        connectGraphAndProxy(m_graph);
        m_graph->markDataDirty();
    }
}

void QAbstract3DSeriesPrivate::setGraph(QQuickGraphsItem *graph)
{
    Q_Q(QAbstract3DSeries);
    m_graph = graph;
    q->setParent(graph);
    connectGraphAndProxy(graph);
    markItemLabelDirty();
}

void QAbstract3DSeriesPrivate::setItemLabelFormat(const QString &format)
{
    m_itemLabelFormat = format;
    markItemLabelDirty();
}

void QAbstract3DSeriesPrivate::setVisible(bool visible)
{
    m_visible = visible;
    markItemLabelDirty();
}

void QAbstract3DSeriesPrivate::setMesh(QAbstract3DSeries::Mesh mesh)
{
    m_mesh = mesh;
    m_changeTracker.meshChanged = true;
    if (m_graph) {
        m_graph->markSeriesVisualsDirty();

        if (m_graph->optimizationHint() == QtGraphs3D::OptimizationHint::Default)
            m_graph->markDataDirty();
    }
}

void QAbstract3DSeriesPrivate::setMeshSmooth(bool enable)
{
    m_meshSmooth = enable;
    m_changeTracker.meshSmoothChanged = true;
    if (m_graph) {
        m_graph->markSeriesVisualsDirty();

        if (m_graph->optimizationHint() == QtGraphs3D::OptimizationHint::Default)
            m_graph->markDataDirty();
    }
}

void QAbstract3DSeriesPrivate::setMeshRotation(const QQuaternion &rotation)
{
    m_meshRotation = rotation;
    m_changeTracker.meshRotationChanged = true;
    if (m_graph) {
        m_graph->markSeriesVisualsDirty();

        if (m_graph->optimizationHint() == QtGraphs3D::OptimizationHint::Default)
            m_graph->markDataDirty();
    }
}

void QAbstract3DSeriesPrivate::setUserDefinedMesh(const QString &meshFile)
{
    m_userDefinedMesh = meshFile;
    m_changeTracker.userDefinedMeshChanged = true;
    if (m_graph) {
        m_graph->markSeriesVisualsDirty();

        if (m_graph->optimizationHint() == QtGraphs3D::OptimizationHint::Default)
            m_graph->markDataDirty();
    }
}

void QAbstract3DSeriesPrivate::setColorStyle(QGraphsTheme::ColorStyle style)
{
    m_colorStyle = style;
    m_changeTracker.colorStyleChanged = true;
    if (m_graph)
        m_graph->markSeriesVisualsDirty();
}

void QAbstract3DSeriesPrivate::setBaseColor(QColor color)
{
    m_baseColor = color;
    m_changeTracker.baseColorChanged = true;
    if (m_graph)
        m_graph->markSeriesVisualsDirty();
}

void QAbstract3DSeriesPrivate::setBaseGradient(const QLinearGradient &gradient)
{
    m_baseGradient = gradient;
    Utils::verifyGradientCompleteness(m_baseGradient);
    m_changeTracker.baseGradientChanged = true;
    if (m_graph)
        m_graph->markSeriesVisualsDirty();
}

void QAbstract3DSeriesPrivate::setSingleHighlightColor(QColor color)
{
    m_singleHighlightColor = color;
    m_changeTracker.singleHighlightColorChanged = true;
    if (m_graph)
        m_graph->markSeriesVisualsDirty();
}

void QAbstract3DSeriesPrivate::setSingleHighlightGradient(const QLinearGradient &gradient)
{
    m_singleHighlightGradient = gradient;
    Utils::verifyGradientCompleteness(m_singleHighlightGradient);
    m_changeTracker.singleHighlightGradientChanged = true;
    if (m_graph)
        m_graph->markSeriesVisualsDirty();
}

void QAbstract3DSeriesPrivate::setMultiHighlightColor(QColor color)
{
    m_multiHighlightColor = color;
    m_changeTracker.multiHighlightColorChanged = true;
    if (m_graph)
        m_graph->markSeriesVisualsDirty();
}

void QAbstract3DSeriesPrivate::setMultiHighlightGradient(const QLinearGradient &gradient)
{
    m_multiHighlightGradient = gradient;
    Utils::verifyGradientCompleteness(m_multiHighlightGradient);
    m_changeTracker.multiHighlightGradientChanged = true;
    if (m_graph)
        m_graph->markSeriesVisualsDirty();
}

void QAbstract3DSeriesPrivate::setLightingMode(QAbstract3DSeries::LightingMode lightingMode)
{
    m_lightingMode = lightingMode;
    if (m_graph)
        m_graph->markSeriesVisualsDirty();
    m_changeTracker.lightingModeChanged = true;
}

void QAbstract3DSeriesPrivate::setName(const QString &name)
{
    m_name = name;
    markItemLabelDirty();
    m_changeTracker.nameChanged = true;
}

void QAbstract3DSeriesPrivate::resetToTheme(const QGraphsTheme &theme, qsizetype seriesIndex, bool force)
{
    Q_Q(QAbstract3DSeries);
    qsizetype themeIndex = seriesIndex;
    if (force || !m_themeTracker.colorStyleOverride) {
        q->setColorStyle(theme.colorStyle());
        m_themeTracker.colorStyleOverride = false;
    }
    if (theme.seriesColors().size() && (force || !m_themeTracker.baseColorOverride)) {
        if (theme.seriesColors().size() <= seriesIndex)
            themeIndex = seriesIndex % theme.seriesColors().size();
        q->setBaseColor(theme.seriesColors().at(themeIndex));
        m_themeTracker.baseColorOverride = false;
    }
    if (theme.seriesGradients().size() && (force || !m_themeTracker.baseGradientOverride)) {
        if (theme.seriesGradients().size() <= seriesIndex)
            themeIndex = seriesIndex % theme.seriesGradients().size();
        q->setBaseGradient(theme.seriesGradients().at(themeIndex));
        m_themeTracker.baseGradientOverride = false;
    }
    if (force || !m_themeTracker.singleHighlightColorOverride) {
        q->setSingleHighlightColor(theme.singleHighlightColor());
        m_themeTracker.singleHighlightColorOverride = false;
    }
    if (force || !m_themeTracker.singleHighlightGradientOverride) {
        q->setSingleHighlightGradient(theme.singleHighlightGradient());
        m_themeTracker.singleHighlightGradientOverride = false;
    }
    if (force || !m_themeTracker.multiHighlightColorOverride) {
        q->setMultiHighlightColor(theme.multiHighlightColor());
        m_themeTracker.multiHighlightColorOverride = false;
    }
    if (force || !m_themeTracker.multiHighlightGradientOverride) {
        q->setMultiHighlightGradient(theme.multiHighlightGradient());
        m_themeTracker.multiHighlightGradientOverride = false;
    }
}

QString QAbstract3DSeriesPrivate::itemLabel()
{
    Q_Q(QAbstract3DSeries);
    if (m_itemLabelDirty) {
        QString oldLabel = m_itemLabel;
        if (m_graph && m_visible)
            createItemLabel();
        else
            m_itemLabel = QString();
        m_itemLabelDirty = false;

        if (oldLabel != m_itemLabel)
            emit q->itemLabelChanged(m_itemLabel);
    }

    return m_itemLabel;
}

void QAbstract3DSeriesPrivate::markItemLabelDirty()
{
    m_itemLabelDirty = true;
    m_changeTracker.itemLabelChanged = true;
}

void QAbstract3DSeriesPrivate::setItemLabelVisible(bool visible)
{
    m_itemLabelVisible = visible;
    markItemLabelDirty();
    m_changeTracker.itemLabelVisibilityChanged = true;
}

bool QAbstract3DSeriesPrivate::isUsingGradient()
{
    return m_colorStyle != QGraphsTheme::ColorStyle::Uniform ? true : false;
}

QT_END_NAMESPACE

#include "moc_qabstract3dseries.cpp"
