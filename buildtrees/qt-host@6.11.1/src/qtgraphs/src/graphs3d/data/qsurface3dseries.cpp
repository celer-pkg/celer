// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcategory3daxis.h"
#include "qquickgraphssurface_p.h"
#include "qsurface3dseries_p.h"
#include "qvalue3daxis.h"
#include "qgraphs3dlogging_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \class QSurface3DSeries
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief The QSurface3DSeries class represents a data series in a 3D surface
 * graph.
 *
 * This class manages the series-specific visual elements, as well as the series
 * data (via a data proxy).
 *
 * Regarding the proxy-series relationship, it is crucial to highlight
 * a couple of key points. In this context, data is stored in series and
 * users can access the dataset through the series. This series is controlled
 * or represented by a proxy object. Thus, the proxy can be used to manage various
 * operations on the data and update the actual dataset. However, it is necessary
 * to create a series associated with this proxy to edit the dataset.
 *
 * If no data proxy is set explicitly for the series, the series creates a
 * default proxy. Setting another proxy will destroy the existing proxy and all
 * data added to the series.
 *
 * The object mesh set via the QAbstract3DSeries::mesh property defines the
 * selection pointer shape in a surface series.
 *
 * QSurface3DSeries supports the following format tags for QAbstract3DSeries::setItemLabelFormat():
 * \table
 *   \row
 *     \li @xTitle    \li Title from x-axis
 *   \row
 *     \li @yTitle    \li Title from y-axis
 *   \row
 *     \li @zTitle    \li Title from z-axis
 *   \row
 *     \li @xLabel    \li Item value formatted using the format of the x-axis.
 *                        For more information, see
 *                        \l{QValue3DAxis::labelFormat}.
 *   \row
 *     \li @yLabel    \li Item value formatted using the format of the y-axis.
 *                        For more information, see
 *                        \l{QValue3DAxis::labelFormat}.
 *   \row
 *     \li @zLabel    \li Item value formatted using the format of the z-axis.
 *                        For more information, see
 *                        \l{QValue3DAxis::labelFormat}.
 *   \row
 *     \li @seriesName \li Name of the series
 * \endtable
 *
 * For example:
 * \snippet doc_src_qtgraphs.cpp labelformat
 *
 * \sa {Qt Graphs Data Handling with 3D}
 */

/*!
 * \qmltype Surface3DSeries
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \nativetype QSurface3DSeries
 * \inherits Abstract3DSeries
 * \brief Represents a data series in a 3D surface graph.
 *
 * This type manages the series specific visual elements, as well as the series
 * data (via a data proxy).
 *
 * Surface3DSeries supports the following format tags for itemLabelFormat:
 * \table
 *   \row
 *     \li @xTitle    \li Title from x-axis
 *   \row
 *     \li @yTitle    \li Title from y-axis
 *   \row
 *     \li @zTitle    \li Title from z-axis
 *   \row
 *     \li @xLabel    \li Item value formatted using the format of the x-axis.
 *                        For more information, see
 *                        \l{QValue3DAxis::labelFormat}{labelFormat}.
 *   \row
 *     \li @yLabel    \li Item value formatted using the format of the y-axis.
 *                        For more information, see
 *                        \l{QValue3DAxis::labelFormat}{labelFormat}.
 *   \row
 *     \li @zLabel    \li Item value formatted using the format of the z-axis.
 *                        For more information, see
 *                        \l{QValue3DAxis::labelFormat}{labelFormat}.
 *   \row
 *     \li @seriesName \li Name of the series
 * \endtable
 *
 * For a more complete description, see QSurface3DSeries.
 *
 * \sa {Qt Graphs Data Handling with 3D}
 */

/*!
 * \qmlproperty SurfaceDataProxy Surface3DSeries::dataProxy
 *
 * The active data proxy. The series assumes ownership of any proxy set to
 * it and deletes any previously set proxy when a new one is added. The proxy
 * cannot be null or set to another series.
 */

/*!
 * \qmlproperty point Surface3DSeries::selectedPoint
 *
 * Sets the surface grid point in the position specified by a row and a column
 * in the data array of the series as selected.
 * Only one point can be selected at a time.
 *
 * To clear the selection from this series, assign invalidSelectionPosition as the
 * position. If this series is added to a graph, the graph can adjust the
 * selection according to user interaction or if it becomes invalid.
 *
 * Removing rows from or inserting rows into the series before the row of the
 * selected point will adjust the selection so that the same point will stay
 * selected.
 *
 * \sa GraphsItem3D::clearSelection()
 */

/*!
 * \qmlproperty point Surface3DSeries::invalidSelectionPosition
 * \readonly
 *
 * A constant property providing an invalid selection position.
 * This position is assigned to the selectedPoint property to clear the selection
 * from this series.
 *
 * \sa GraphsItem3D::clearSelection()
 */

/*!
 * \qmlproperty Shading Surface3DSeries::shading
 *
 * Sets surface flat shading to visible. It is preset to \c Surface3DSeries.Shading.Flat by default.
 * When disabled, the normals on the surface are interpolated making the edges
 * look round. When visible, the normals are kept the same on a triangle making
 * the color of the triangle solid. This makes the data more readable from the
 * model. \note Flat shaded surfaces require at least GLSL version 1.2 with
 * GL_EXT_gpu_shader4 extension. The value of the flatShadingSupported property
 * indicates whether flat shading is supported at runtime.
 */

/*!
 * \qmlproperty bool Surface3DSeries::flatShadingSupported
 *
 * Indicates whether flat shading for surfaces is supported by the current
 * system. It requires at least GLSL version 1.2 with GL_EXT_gpu_shader4
 * extension.
 *
 * \note This read-only property is set to its correct value after the first
 * render pass. Until then it is always \c true.
 */

/*!
 * \qmlproperty DrawFlag Surface3DSeries::drawMode
 *
 * Sets the drawing mode to one of
 * \l{QSurface3DSeries::DrawFlag}{Surface3DSeries.DrawFlag}{QSurface3DSeries.DrawFilledSurface}.
 * Either DrawWireframe or DrawSurface must be set
 */

/*!
 * \qmlproperty string Surface3DSeries::textureFile
 *
 * The texture file name for the surface texture. To clear the texture, an empty
 * file name is set.
 */

/*!
 * \qmlproperty color Surface3DSeries::wireframeColor
 *
 * The color used to draw the gridlines of the surface wireframe.
 */

/*
 * \qmlproperty bool Surface3DSeries::rowsSanitized
 * \since 6.11
 *  Whether to perform row sanitization.
 *
 *  When \c{true}, removes rows in the series that do not contain valid data.
 *  \note This may incur a small performance penalty.
 */

/*!
 * \qmlproperty SurfaceDataArray Surface3DSeries::dataArray
 *
 * Holds the reference of the data array.
 *
 * dataArrayChanged signal is emitted when data array is set, unless \a newDataArray
 * is identical to the previous one.
 *
 * \note Before doing anything regarding the dataArray, a series must be created for
 * the relevant proxy.
 */

/*!
    \qmlsignal Surface3DSeries::dataProxyChanged(SurfaceDataProxy proxy)

    This signal is emitted when dataProxy changes to \a proxy.
*/
/*!
    \qmlsignal Surface3DSeries::selectedPointChanged(point position)

    This signal is emitted when selectedPoint changes to \a position.
*/
/*!
    \qmlsignal Surface3DSeries::shadingChanged(const Shading shading)

    This signal is emitted when \l shading changes to \a shading.
*/
/*!
    \qmlsignal Surface3DSeries::flatShadingSupportedChanged(bool enable)

    This signal is emitted when flatShadingSupported changes to \a enable.
*/
/*!
    \qmlsignal Surface3DSeries::drawModeChanged(DrawFlag mode)

    This signal is emitted when drawMode changes to \a mode.
*/
/*!
    \qmlsignal Surface3DSeries::textureFileChanged(string filename)

    This signal is emitted when textureFile changes to \a filename.
*/
/*!
    \qmlsignal Surface3DSeries::wireframeColorChanged(color color)

    This signal is emitted when wireframeColor changes to \a color.
*/
/*!
    \qmlsignal Surface3DSeries::rowsSanitizedChanged(bool enabled)

    This signal is emitted when rowsSanitized changes to \a enabled.
*/
/*!
    \qmlsignal Surface3DSeries::dataArrayChanged(SurfaceDataArray array)

    This signal is emitted when dataArray changes to \a array.
*/

/*!
 * \enum QSurface3DSeries::DrawFlag
 *
 * The drawing mode of the surface. Values of this enumeration can be combined
 * with the OR operator.
 *
 * \value DrawWireframe
 *        Only the grid is drawn.
 * \value DrawSurface
 *        Only the surface is drawn.
 * \value DrawSurfaceAndWireframe
 *        Both the surface and grid are drawn.
 *
 * \value [since 6.10] DrawFilledSurface
 *        Draws a fill for a surface.
 *        Setting this mode also sets DrawSurface if it is not already set.
 */

/*!
 * Constructs a surface 3D series with the parent \a parent.
 */
QSurface3DSeries::QSurface3DSeries(QObject *parent)
    : QAbstract3DSeries(*(new QSurface3DSeriesPrivate()), parent)
{
    Q_D(QSurface3DSeries);
    // Default proxy
    d->setDataProxy(new QSurfaceDataProxy);
}

/*!
 * Constructs a surface 3D series with the data proxy \a dataProxy and the
 * parent \a parent.
 */
QSurface3DSeries::QSurface3DSeries(QSurfaceDataProxy *dataProxy, QObject *parent)
    : QAbstract3DSeries(*(new QSurface3DSeriesPrivate()), parent)
{
    Q_D(QSurface3DSeries);
    d->setDataProxy(dataProxy);
}

/*!
 * \internal
 */
QSurface3DSeries::QSurface3DSeries(QSurface3DSeriesPrivate &d, QObject *parent)
    : QAbstract3DSeries(d, parent)
{}

/*!
 * Deletes the surface 3D series.
 */
QSurface3DSeries::~QSurface3DSeries() {}

/*!
 * \property QSurface3DSeries::dataProxy
 *
 * \brief The active data proxy.
 *
 * The series assumes ownership of any proxy set to it and deletes any
 * previously set proxy when a new one is added. The proxy cannot be null or
 * set to another series.
 */
void QSurface3DSeries::setDataProxy(QSurfaceDataProxy *proxy)
{
    Q_D(QSurface3DSeries);
    d->setDataProxy(proxy);
}

QSurfaceDataProxy *QSurface3DSeries::dataProxy() const
{
    Q_D(const QSurface3DSeries);
    return static_cast<QSurfaceDataProxy *>(d->dataProxy());
}

/*!
 * \property QSurface3DSeries::selectedPoint
 *
 * \brief The surface grid point that is selected in the series.
 *
 * Selects a surface grid point at the position \a position in the data array of
 * the series specified by a row and a column.
 *
 * Only one point can be selected at a time.
 *
 * To clear the selection from this series, invalidSelectionPosition() is set as \a
 * position. If this series is added to a graph, the graph can adjust the
 * selection according to user interaction or if it becomes invalid.
 *
 * Removing rows from or inserting rows into the series before the row of the
 * selected point will adjust the selection so that the same point will stay
 * selected.
 *
 * \sa Q3DGraphsWidgetItem::clearSelection()
 */
void QSurface3DSeries::setSelectedPoint(QPoint position)
{
    Q_D(QSurface3DSeries);
    // Don't do this in private to avoid loops, as that is used for callback from
    // graph.
    if (d->m_graph)
        static_cast<QQuickGraphsSurface *>(d->m_graph)->setSelectedPoint(position, this, true);
    else
        d->setSelectedPoint(position);
}

QPoint QSurface3DSeries::selectedPoint() const
{
    Q_D(const QSurface3DSeries);
    return d->m_selectedPoint;
}

/*!
 * Returns the QPoint signifying an invalid selection position. This is set to
 * the selectedPoint property to clear the selection from this series.
 *
 * \sa Q3DGraphsWidgetItem::clearSelection()
 */
QPoint QSurface3DSeries::invalidSelectionPosition()
{
    return QQuickGraphsSurface::invalidSelectionPosition();
}

/*!
 * \property QSurface3DSeries::shading
 *
 * \brief Whether surface flat shading is enabled.
 *
 * Preset to \c QSurface3DSeries::Shading::Flat by default.
 *
 * When disabled, the normals on the surface are interpolated making the edges
 * look round. When visible, the normals are kept the same on a triangle making
 * the color of the triangle solid. This makes the data more readable from the
 * model. \note Flat shaded surfaces require at least GLSL version 1.2 with
 * GL_EXT_gpu_shader4 extension. The value of the flatShadingSupported property
 * indicates whether flat shading is supported at runtime.
 */
void QSurface3DSeries::setShading(const QSurface3DSeries::Shading shading)
{
    Q_D(QSurface3DSeries);
    if (d->m_shading == shading) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << shading;
        return;
    }
    d->setShading(shading);
    emit shadingChanged(shading);
}

QSurface3DSeries::Shading QSurface3DSeries::shading() const
{
    Q_D(const QSurface3DSeries);
    return d->m_shading;
}

/*!
 * \property QSurface3DSeries::flatShadingSupported
 *
 * \brief Whether surface flat shading is supported by the current system.
 *
 * Flat shading for surfaces requires at least GLSL version 1.2 with
 * GL_EXT_gpu_shader4 extension. If \c true, flat shading for surfaces is
 * supported. \note This read-only property is set to its correct value after
 * the first render pass. Until then it is always \c true.
 */
bool QSurface3DSeries::isFlatShadingSupported() const
{
    Q_D(const QSurface3DSeries);
    if (d->m_graph)
        return static_cast<QQuickGraphsSurface *>(d->m_graph)->isFlatShadingSupported();
    else
        return true;
}

/*!
 * \property QSurface3DSeries::drawMode
 *
 * The drawing mode.
 *
 * Possible values are the values of DrawFlag. Clearing all flags is not
 * allowed.
 */
void QSurface3DSeries::setDrawMode(QSurface3DSeries::DrawFlags mode)
{
    Q_D(QSurface3DSeries);
    if (d->m_drawMode == mode) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << mode;
        return;
    }
    d->setDrawMode(mode);
    emit drawModeChanged(mode);
}

QSurface3DSeries::DrawFlags QSurface3DSeries::drawMode() const
{
    Q_D(const QSurface3DSeries);
    return d->m_drawMode;
}

/*!
 * \property QSurface3DSeries::texture
 *
 * \brief The texture for the surface as a QImage.
 *
 * Setting an empty QImage clears the texture.
 */
void QSurface3DSeries::setTexture(const QImage &texture)
{
    Q_D(QSurface3DSeries);
    if (d->m_texture == texture) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << texture;
        return;
    }
    d->setTexture(texture);

    emit textureChanged(texture);
    d->m_textureFile.clear();
}

QImage QSurface3DSeries::texture() const
{
    Q_D(const QSurface3DSeries);
    return d->m_texture;
}

/*!
 * \property QSurface3DSeries::textureFile
 *
 * \brief The texture for the surface as a file.
 *
 * Setting an empty file name clears the texture.
 */
void QSurface3DSeries::setTextureFile(const QString &filename)
{
    Q_D(QSurface3DSeries);
    if (d->m_textureFile == filename) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(filename));
        return;
    }

    if (filename.isEmpty()) {
        setTexture(QImage());
    } else {
        QImage image(filename);
        if (image.isNull()) {
            qCWarning(lcProperties3D, "%s tried to set invalid image file as surface texture.",
                      qUtf8Printable(QLatin1String(__FUNCTION__)));
            return;
        }
        setTexture(image);
    }

    d->m_textureFile = filename;
    emit textureFileChanged(filename);
}

QString QSurface3DSeries::textureFile() const
{
    Q_D(const QSurface3DSeries);
    return d->m_textureFile;
}

/*!
 * \property QSurface3DSeries::wireframeColor
 *
 * \brief The color for the surface wireframe.
 */
void QSurface3DSeries::setWireframeColor(QColor color)
{
    Q_D(QSurface3DSeries);
    if (d->m_wireframeColor == color) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << color;
        return;
    }
    d->setWireframeColor(color);
    emit wireframeColorChanged(color);
}

QColor QSurface3DSeries::wireframeColor() const
{
    Q_D(const QSurface3DSeries);
    return d->m_wireframeColor;
}

/*!
 *  \property QSurface3DSeries::rowsSanitized
 *
 *  \brief Whether to perform row sanitization.
 *
 *  When \a sanitized is \c{true}, removes rows in the series that do not contain valid data.
 *  \note This may incur a small performance penalty.
 */
void QSurface3DSeries::setRowsSanitized(bool sanitized)
{
    Q_D(QSurface3DSeries);

    if (d->m_rowsSanitized == sanitized) {
        qCDebug(lcProperties3D) << __FUNCTION__ << "value is already set to " << sanitized;
        return;
    }
    d->setRowsSanitized(sanitized);
    emit rowsSanitizedChanged(sanitized);
}

bool QSurface3DSeries::rowsSanitized() const {
    Q_D(const QSurface3DSeries);
    return d->m_rowsSanitized;
}

/*!
 * \property QSurface3DSeries::dataArray
 *
 * \brief Data array for the series.
 *
 * Holds the reference of the data array.
 *
 * dataArrayChanged signal is emitted when data array is set, unless \a newDataArray
 * is identical to the previous one.
 *
 * \note Before doing anything regarding the dataArray, a series must be created for
 * the relevant proxy.
 *
 *\sa clearRow(qsizetype rowIndex)
 *
 *\sa clearArray()
 */
void QSurface3DSeries::setDataArray(const QSurfaceDataArray &newDataArray)
{
    Q_D(QSurface3DSeries);
    if (d->m_dataArray.isSharedWith(newDataArray)) {
        qCDebug(lcProperties3D, "%s newDataArray is the same than the current one",
                qUtf8Printable(QLatin1String(__FUNCTION__)));
        return;
    }
    d->setDataArray(newDataArray);
    emit dataArrayChanged(newDataArray);
}

/*!
 * Clears the existing row in the array according to given \a rowIndex.
 */
void QSurface3DSeries::clearRow(qsizetype rowIndex)
{
    Q_D(QSurface3DSeries);
    d->clearRow(rowIndex);
}

/*!
 * Clears the existing array.
 */
void QSurface3DSeries::clearArray()
{
    Q_D(QSurface3DSeries);
    d->clearArray();
}

const QSurfaceDataArray &QSurface3DSeries::dataArray() const &
{
    Q_D(const QSurface3DSeries);
    return d->m_dataArray;
}

QSurfaceDataArray QSurface3DSeries::dataArray() &&
{
    Q_D(QSurface3DSeries);
    return std::move(d->m_dataArray);
}

/*!
 * \property QSurface3DSeries::axisX
 * \since 6.11
 *
 * \brief Holds an additional X-axis for the series
 *  If an axis is given, the series will be fitted
 *  to the minimum and maximum values of the axis.
 *
 */
void QSurface3DSeries::setAxisX(QValue3DAxis *axis) {
    Q_D(QSurface3DSeries);
    if (d->m_axisX == axis) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << axis;
        return;
    }
    d->setAxisX(axis);
    emit axisXChanged(axis);
}

QValue3DAxis *QSurface3DSeries::axisX() const {
    Q_D(const QSurface3DSeries);
    return d->m_axisX;
}

void QSurface3DSeries::resetAxisX()
{
    Q_D(QSurface3DSeries);
    if (!d->m_axisX) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "axis X does not set";
        return;
    }
    d->resetAxisX();
    emit axisXChanged(nullptr);
}

/*!
 * \property QSurface3DSeries::axisY
 * \since 6.11
 *
 * \brief Holds an additional Y-axis for the series
 *  If an axis is given, the series will be fitted
 *  to the minimum and maximum values of the axis.
 */
void QSurface3DSeries::setAxisY(QValue3DAxis *axis) {
    Q_D(QSurface3DSeries);
    if (d->m_axisY == axis) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << axis;
        return;
    }
    d->setAxisY(axis);
    emit axisYChanged(axis);
}

QValue3DAxis *QSurface3DSeries::axisY() const {
    Q_D(const QSurface3DSeries);
    return d->m_axisY;
}

void QSurface3DSeries::resetAxisY()
{
    Q_D(QSurface3DSeries);
    if (!d->m_axisY) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "axis y does not set";
        return;
    }
    d->resetAxisY();
    emit axisYChanged(nullptr);
}

/*!
 * \property QSurface3DSeries::axisZ
 * \since 6.11
 *
 * \brief Holds an additional Z-axis for the series
 *  If an axis is given, the series will be fitted
 *  to the minimum and maximum values of the axis.
 */
void QSurface3DSeries::setAxisZ(QValue3DAxis *axis) {
    Q_D(QSurface3DSeries);
    if (d->m_axisZ == axis) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << axis;
        return;
    }
    d->setAxisZ(axis);
    emit axisZChanged(axis);
}

QValue3DAxis *QSurface3DSeries::axisZ() const {
    Q_D(const QSurface3DSeries);
    return d->m_axisZ;
}

void QSurface3DSeries::resetAxisZ()
{
    Q_D(QSurface3DSeries);
    if (!d->m_axisZ) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "axis z does not set";
        return;
    }
    d->resetAxisZ();
    emit axisZChanged(nullptr);
}

// QSurface3DSeriesPrivate

QSurface3DSeriesPrivate::QSurface3DSeriesPrivate()
    : QAbstract3DSeriesPrivate(QAbstract3DSeries::SeriesType::Surface)
    , m_selectedPoint(QQuickGraphsSurface::invalidSelectionPosition())
    , m_shading(QSurface3DSeries::Shading::Flat)
    , m_drawMode(QSurface3DSeries::DrawSurfaceAndWireframe)
    , m_wireframeColor(Qt::black)
    , m_rowsSanitized(false)
    , m_axisX(nullptr)
    , m_axisY(nullptr)
    , m_axisZ(nullptr)
{
    m_itemLabelFormat = QStringLiteral("@xLabel, @yLabel, @zLabel");
    m_mesh = QAbstract3DSeries::Mesh::Sphere;
}

QSurface3DSeriesPrivate::~QSurface3DSeriesPrivate()
{
    clearArray();
}

void QSurface3DSeriesPrivate::setDataProxy(QAbstractDataProxy *proxy)
{
    Q_ASSERT(proxy->type() == QAbstractDataProxy::DataType::Surface);
    Q_Q(QSurface3DSeries);

    QAbstract3DSeriesPrivate::setDataProxy(proxy);

    emit q->dataProxyChanged(static_cast<QSurfaceDataProxy *>(proxy));
}

void QSurface3DSeriesPrivate::connectGraphAndProxy(QQuickGraphsItem *newGraph)
{
    Q_Q(QSurface3DSeries);
    QSurfaceDataProxy *surfaceDataProxy = static_cast<QSurfaceDataProxy *>(m_dataProxy);

    if (m_graph && surfaceDataProxy) {
        // Disconnect old graph/old proxy
        QObject::disconnect(surfaceDataProxy, 0, m_graph, 0);
        QObject::disconnect(q, 0, m_graph, 0);
    }

    if (newGraph && surfaceDataProxy) {
        QQuickGraphsSurface *graph = static_cast<QQuickGraphsSurface *>(newGraph);

        QObject::connect(surfaceDataProxy,
                         &QSurfaceDataProxy::arrayReset,
                         graph,
                         &QQuickGraphsSurface::handleArrayReset);
        QObject::connect(surfaceDataProxy,
                         &QSurfaceDataProxy::rowsAdded,
                         graph,
                         &QQuickGraphsSurface::handleRowsAdded);
        QObject::connect(surfaceDataProxy,
                         &QSurfaceDataProxy::rowsChanged,
                         graph,
                         &QQuickGraphsSurface::handleRowsChanged);
        QObject::connect(surfaceDataProxy,
                         &QSurfaceDataProxy::rowsRemoved,
                         graph,
                         &QQuickGraphsSurface::handleRowsRemoved);
        QObject::connect(surfaceDataProxy,
                         &QSurfaceDataProxy::rowsInserted,
                         graph,
                         &QQuickGraphsSurface::handleRowsInserted);
        QObject::connect(surfaceDataProxy,
                         &QSurfaceDataProxy::itemChanged,
                         graph,
                         &QQuickGraphsSurface::handleItemChanged);
        QObject::connect(q,
                         &QSurface3DSeries::dataProxyChanged,
                         graph,
                         &QQuickGraphsSurface::handleArrayReset);
    }
}

void QSurface3DSeriesPrivate::createItemLabel()
{
    Q_Q(QSurface3DSeries);
    static const QString xTitleTag(QStringLiteral("@xTitle"));
    static const QString yTitleTag(QStringLiteral("@yTitle"));
    static const QString zTitleTag(QStringLiteral("@zTitle"));
    static const QString xLabelTag(QStringLiteral("@xLabel"));
    static const QString yLabelTag(QStringLiteral("@yLabel"));
    static const QString zLabelTag(QStringLiteral("@zLabel"));
    static const QString seriesNameTag(QStringLiteral("@seriesName"));

    if (m_selectedPoint == QSurface3DSeries::invalidSelectionPosition()) {
        m_itemLabel = QString(hiddenLabelTag);
        return;
    }

    QValue3DAxis *axisX = static_cast<QValue3DAxis *>(m_graph->axisX());
    QValue3DAxis *axisY = static_cast<QValue3DAxis *>(m_graph->axisY());
    QValue3DAxis *axisZ = static_cast<QValue3DAxis *>(m_graph->axisZ());
    QVector3D selectedPosition
        = q->dataProxy()->itemAt(QPoint(m_selectedPoint.y(), m_selectedPoint.x())).position();

    m_itemLabel = m_itemLabelFormat;

    m_itemLabel.replace(xTitleTag, axisX->title());
    m_itemLabel.replace(yTitleTag, axisY->title());
    m_itemLabel.replace(zTitleTag, axisZ->title());

    if (m_itemLabel.contains(xLabelTag)) {
        QString valueLabelText = axisX->formatter()->stringForValue(qreal(selectedPosition.x()),
                                                                    axisX->labelFormat());
        m_itemLabel.replace(xLabelTag, valueLabelText);
    }
    if (m_itemLabel.contains(yLabelTag)) {
        QString valueLabelText = axisY->formatter()->stringForValue(qreal(selectedPosition.y()),
                                                                    axisY->labelFormat());
        m_itemLabel.replace(yLabelTag, valueLabelText);
    }
    if (m_itemLabel.contains(zLabelTag)) {
        QString valueLabelText = axisZ->formatter()->stringForValue(qreal(selectedPosition.z()),
                                                                    axisZ->labelFormat());
        m_itemLabel.replace(zLabelTag, valueLabelText);
    }
    m_itemLabel.replace(seriesNameTag, m_name);
}

void QSurface3DSeriesPrivate::setSelectedPoint(QPoint position)
{
    Q_Q(QSurface3DSeries);
    if (position == m_selectedPoint) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << position;
        return;
    }
    markItemLabelDirty();
    m_selectedPoint = position;
    emit q->selectedPointChanged(m_selectedPoint);
}

void QSurface3DSeriesPrivate::setShading(const QSurface3DSeries::Shading shading)
{
    m_shading = shading;
    if (m_graph)
        m_graph->markSeriesVisualsDirty();
}

void QSurface3DSeriesPrivate::setDrawMode(QSurface3DSeries::DrawFlags mode)
{
    if (mode.testFlag(QSurface3DSeries::DrawFilledSurface)
        && !mode.testFlag(QSurface3DSeries::DrawSurface)) {
        qCWarning(lcProperties3D, "%s drawSurface was not set and is needed for DrawFilledSurface."
                 " Setting it automatically", qUtf8Printable(QLatin1String(__FUNCTION__)));
        mode |= QSurface3DSeries::DrawSurface;
    }
    if (mode.testFlag(QSurface3DSeries::DrawWireframe)
        || mode.testFlag(QSurface3DSeries::DrawSurface)) {
        m_drawMode = mode;
        if (m_graph)
            m_graph->markSeriesVisualsDirty();
    } else {
        qCWarning(lcProperties3D, "%s drawWireframe or DrawSurface must be set. Mode not changed.",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
    }
}

void QSurface3DSeriesPrivate::setTexture(const QImage &texture)
{
    Q_Q(QSurface3DSeries);
    m_texture = texture;
    if (m_graph)
        m_graph->markSeriesVisualsDirty();
    if (static_cast<QQuickGraphsSurface *>(m_graph))
        static_cast<QQuickGraphsSurface *>(m_graph)->updateSurfaceTexture(q);
}

void QSurface3DSeriesPrivate::setWireframeColor(QColor color)
{
    m_wireframeColor = color;
    if (m_graph)
        m_graph->markSeriesVisualsDirty();
}

void QSurface3DSeriesPrivate::setRowsSanitized(bool enabled)
{
    m_rowsSanitized = enabled;
    if (m_graph)
        m_graph->markDataDirty();
}

void QSurface3DSeriesPrivate::setDataArray(const QSurfaceDataArray &newDataArray)
{
    m_dataArray = newDataArray;
}

void QSurface3DSeriesPrivate::clearRow(qsizetype rowIndex)
{
    m_dataArray[rowIndex].clear();
}

void QSurface3DSeriesPrivate::clearArray()
{
    m_dataArray.clear();
}

void QSurface3DSeriesPrivate::setAxisX(QValue3DAxis *axis)
{
    m_axisX = axis;
}

void QSurface3DSeriesPrivate::resetAxisX()
{
    m_axisX = nullptr;
}

void QSurface3DSeriesPrivate::setAxisY(QValue3DAxis *axis)
{
    m_axisY = axis;
}

void QSurface3DSeriesPrivate::resetAxisY()
{
    m_axisY = nullptr;
}

void QSurface3DSeriesPrivate::setAxisZ(QValue3DAxis *axis)
{
    m_axisZ = axis;
}

void QSurface3DSeriesPrivate::resetAxisZ()
{
    m_axisZ = nullptr;
}

QT_END_NAMESPACE

#include "moc_qsurface3dseries.cpp"
