// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "graphs3d/utils/qgraphs3dlogging_p.h"
#include "private/qquick3drepeater_p.h"
#include "q3dscene.h"
#include "qabstract3daxis.h"
#include "qabstractdataproxy.h"
#include "qcategory3daxis_p.h"
#include "qgraphs3dlogging_p.h"
#include "qgraphsinputhandler_p.h"
#include "qquickgraphssurface_p.h"
#include "qquickgraphstexturedata_p.h"
#include "qsurface3dseries.h"
#include "qsurface3dseries_p.h"
#include "qsurfacedataproxy.h"
#include "qsurfacedataproxy_p.h"
#include "qvalue3daxis_p.h"
#include "utils_p.h"

#include <QtQuick3D/private/qquick3dcustommaterial_p.h>
#include <QtQuick3D/private/qquick3dprincipledmaterial_p.h>

#include <QtQuick/qquickitemgrabresult.h>
#include <qtgraphs_tracepoints_p.h>

QT_BEGIN_NAMESPACE

Q_TRACE_PREFIX(qtgraphs,
                   "QT_BEGIN_NAMESPACE" \
                   "class QQuickGraphsSurface;" \
                   "class QSurface3DSeries;" \
                   "QT_END_NAMESPACE"
               )

Q_TRACE_POINT(qtgraphs, QGraphs3DSurfaceModelUpdate_entry, void *model);
Q_TRACE_POINT(qtgraphs, QGraphs3DSurfaceModelUpdate_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DSurfacePointSelectionUpdate_entry, void *model);
Q_TRACE_POINT(qtgraphs, QGraphs3DSurfacePointSelectionUpdate_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DSurfaceDoPicking_entry, float posX, float posY);
Q_TRACE_POINT(qtgraphs, QGraphs3DSurfaceDoPicking_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DSurfaceDoRayPicking_entry, float originX, float originY,
              float originZ, float directionX, float directionY, float directionZ);
Q_TRACE_POINT(qtgraphs, QGraphs3DSurfaceDoRayPicking_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DSurfaceCreateSliceView_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DSurfaceCreateSliceView_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DSurfaceCreateOffscreenSliceView_entry, int index, int requestedIndex, int sliceType);
Q_TRACE_POINT(qtgraphs, QGraphs3DSurfaceCreateOffscreenSliceView_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DSurfaceAddModel_entry, QSurface3DSeries *series);
Q_TRACE_POINT(qtgraphs, QGraphs3DSurfaceAddModel_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DSurfaceAddSliceModel_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DSurfaceAddSliceModel_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DSurfaceAddFillModel_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DSurfaceAddFillModel_exit);

/*!
 * \qmltype Surface3D
 * \inherits GraphsItem3D
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \brief Describes the usage of the 3D surface graph.
 *
 * This type enables developers to render surface plots in 3D with Qt Quick.
 *
 * You will need to import the Qt Graphs module to use this type:
 *
 * \snippet doc_src_qmlgraphs.cpp 0
 *
 * After that you can use Surface3D in your qml files:
 *
 * \snippet doc_src_qmlgraphs.cpp 3
 *
 * See \l{Surface Graph Gallery} for more thorough usage example.
 *
 * \sa Surface3DSeries, ItemModelSurfaceDataProxy, Bars3D, Scatter3D,
 * {Qt Graphs C++ Classes for 3D}
 */

/*!
 * \qmlproperty Value3DAxis Surface3D::axisX
 * The active x-axis.
 *
 * If an axis is not given, a temporary default axis with no labels and an
 * automatically adjusting range is created.
 * This temporary axis is destroyed if another axis is explicitly set to the
 * same orientation.
 */

/*!
 * \qmlproperty Value3DAxis Surface3D::axisY
 * The active y-axis.
 *
 * If an axis is not given, a temporary default axis with no labels and an
 * automatically adjusting range is created.
 * This temporary axis is destroyed if another axis is explicitly set to the
 * same orientation.
 */

/*!
 * \qmlproperty Value3DAxis Surface3D::axisZ
 * The active z-axis.
 *
 * If an axis is not given, a temporary default axis with no labels and an
 * automatically adjusting range is created.
 * This temporary axis is destroyed if another axis is explicitly set to the
 * same orientation.
 */

/*!
 * \qmlproperty Surface3DSeries Surface3D::selectedSeries
 * \readonly
 *
 * The selected series or null. If \l {GraphsItem3D::selectionMode}{selectionMode}
 * has the \c MultiSeries flag set, this property holds the series
 * which owns the selected point.
 */

/*!
 * \qmlproperty list<Surface3DSeries> Surface3D::seriesList
 * \qmldefault
 * This property holds the series of the graph.
 * By default, this property contains an empty list.
 * To set the series, either use the addSeries() function or define them as
 * children of the graph.
 */

/*!
 * \qmlproperty bool Surface3D::flipHorizontalGrid
 *
 * In some use cases the horizontal axis grid is mostly covered by the surface,
 * so it can be more useful to display the horizontal axis grid on top of the
 * graph rather than on the bottom. A typical use case for this is showing 2D
 * spectrograms using orthoGraphic projection with a top-down viewpoint.
 *
 * If \c{false}, the horizontal axis grid and labels are drawn on the horizontal
 * background of the graph.
 * If \c{true}, the horizontal axis grid and labels are drawn on the opposite
 * side of the graph from the horizontal background.
 * Defaults to \c{false}.
 */

/*!
 * \qmlmethod void Surface3D::addSeries(Surface3DSeries series)
 * Adds the \a series to the graph.
 * \sa GraphsItem3D::hasSeries()
 */

/*!
 * \qmlmethod void Surface3D::removeSeries(Surface3DSeries series)
 * Removes the \a series from the graph.
 * \sa GraphsItem3D::hasSeries()
 */

/*!
 * \qmlmethod void Surface3D::removeSeries(Surface3DSeries series)
 * Removes the \a series from the graph.
 * \sa GraphsItem3D::hasSeries()
 */

/*!
 * \qmlmethod void Surface3D::renderSliceToImage(int index, int requestedIndex, QtGraphs3D::SliceCaptureType sliceType, QUrl filePath)
 * \since 6.10
 *
 * Exports a 2d slice from series at \a index and saves the result to an image
 * at a specified \a filePath.
 * To export all series, set \a index to -1.
 * The exported slice includes lines of row or column, which is defined by
 * \a sliceType at a given \a requestedIndex.
 */

/*!
 * \qmlsignal Surface3D::axisXChanged(ValueAxis3D axis)
 *
 * This signal is emitted when axisX changes to \a axis.
 */

/*!
 * \qmlsignal Surface3D::axisYChanged(ValueAxis3D axis)
 *
 * This signal is emitted when axisY changes to \a axis.
 */

/*!
 * \qmlsignal Surface3D::axisZChanged(ValueAxis3D axis)
 *
 * This signal is emitted when axisZ changes to \a axis.
 */

/*!
 * \qmlsignal Surface3D::selectedSeriesChanged(Surface3DSeries series)
 *
 * This signal is emitted when selectedSeries changes to \a series.
 */

/*!
 * \qmlsignal Surface3D::flipHorizontalGridChanged(bool flip)
 *
 * This signal is emitted when flipHorizontalGrid changes to \a flip.
 */

QQuickGraphsSurface::QQuickGraphsSurface(QQuickItem *parent)
    : QQuickGraphsItem(parent)
{
    m_graphType = QAbstract3DSeries::SeriesType::Surface;
    setAxisX(0);
    setAxisY(0);
    setAxisZ(0);
    setAcceptedMouseButtons(Qt::AllButtons);
    clearSelection();
}

QQuickGraphsSurface::~QQuickGraphsSurface()
{
    for (const auto &model : std::as_const(m_model))
        delete model;
    if (m_grabresult)
        delete m_grabresult;
}

void QQuickGraphsSurface::setAxisX(QValue3DAxis *axis)
{
    QQuickGraphsItem::setAxisX(axis);
}

QValue3DAxis *QQuickGraphsSurface::axisX() const
{
    return static_cast<QValue3DAxis *>(QQuickGraphsItem::axisX());
}

void QQuickGraphsSurface::setAxisY(QValue3DAxis *axis)
{
    QQuickGraphsItem::setAxisY(axis);
}

QValue3DAxis *QQuickGraphsSurface::axisY() const
{
    return static_cast<QValue3DAxis *>(QQuickGraphsItem::axisY());
}

void QQuickGraphsSurface::setAxisZ(QValue3DAxis *axis)
{
    QQuickGraphsItem::setAxisZ(axis);
}

QValue3DAxis *QQuickGraphsSurface::axisZ() const
{
    return static_cast<QValue3DAxis *>(QQuickGraphsItem::axisZ());
}

void QQuickGraphsSurface::handleShadingChanged()
{
    auto series = static_cast<QSurface3DSeries *>(sender());
    for (auto model : std::as_const(m_model)) {
        if (model->series == series) {
            updateModel(model);
            break;
        }
    }
}

void QQuickGraphsSurface::handleWireframeColorChanged()
{
    for (auto model : std::as_const(m_model)) {
        QQmlListReference gridMaterialRef(model->gridModel, "materials");
        auto gridMaterial = gridMaterialRef.at(0);
        QColor gridColor = model->series->wireframeColor();
        gridMaterial->setProperty("gridColor", gridColor);

        if (sliceView()) {
            QQmlListReference gridMaterialRef(model->sliceGridModel, "materials");
            auto gridMaterial = static_cast<QQuick3DPrincipledMaterial *>(gridMaterialRef.at(0));
            gridMaterial->setBaseColor(gridColor);
        }
    }
}

void QQuickGraphsSurface::handleMeshTypeChanged(QAbstract3DSeries::Mesh mesh)
{
    QSurface3DSeries *sender = qobject_cast<QSurface3DSeries *>(QObject::sender());
    changePointerMeshTypeForSeries(mesh, sender);
    if (sliceView())
        changeSlicePointerMeshTypeForSeries(mesh, sender);
}

void QQuickGraphsSurface::changePointerMeshTypeForSeries(QAbstract3DSeries::Mesh mesh,
                                                         QSurface3DSeries *series)
{
    changePointerForSeries(getMeshFileName(mesh, series), series);
}

void QQuickGraphsSurface::changeSlicePointerMeshTypeForSeries(QAbstract3DSeries::Mesh mesh,
                                                              QSurface3DSeries *series)
{
    changeSlicePointerForSeries(getMeshFileName(mesh, series), series);
}

void QQuickGraphsSurface::handleLightingModeChanged()
{
    auto series = static_cast<QSurface3DSeries *>(QObject::sender());
    for (auto model : std::as_const(m_model)) {
        if (model->series == series) {
            updateMaterial(model);
            break;
        }
    }
}

QString QQuickGraphsSurface::getMeshFileName(QAbstract3DSeries::Mesh mesh,
                                             QSurface3DSeries *series) const
{
    QString fileName = {};
    switch (mesh) {
    case QAbstract3DSeries::Mesh::Sphere:
        fileName = QStringLiteral("defaultMeshes/sphereMesh");
        break;
    case QAbstract3DSeries::Mesh::Bar:
    case QAbstract3DSeries::Mesh::Cube:
        fileName = QStringLiteral("defaultMeshes/barMesh");
        break;
    case QAbstract3DSeries::Mesh::Pyramid:
        fileName = QStringLiteral("defaultMeshes/pyramidMesh");
        break;
    case QAbstract3DSeries::Mesh::Cone:
        fileName = QStringLiteral("defaultMeshes/coneMesh");
        break;
    case QAbstract3DSeries::Mesh::Cylinder:
        fileName = QStringLiteral("defaultMeshes/cylinderMesh");
        break;
    case QAbstract3DSeries::Mesh::BevelBar:
    case QAbstract3DSeries::Mesh::BevelCube:
        fileName = QStringLiteral("defaultMeshes/bevelBarMesh");
        break;
    case QAbstract3DSeries::Mesh::UserDefined:
        fileName = series->userDefinedMesh();
        break;
    default:
        fileName = QStringLiteral("defaultMeshes/sphereMesh");
    }
    return fileName;
}

void QQuickGraphsSurface::handlePointerChanged(const QString &filename)
{
    QSurface3DSeries *sender = qobject_cast<QSurface3DSeries *>(QObject::sender());
    changePointerForSeries(filename, sender);
    changeSlicePointerForSeries(filename, sender);
}

void QQuickGraphsSurface::changePointerForSeries(const QString &filename, QSurface3DSeries *series)
{
    if (!filename.isEmpty()) {
        // Selection pointer
        QQuick3DNode *parent = graphNode();

        QQuick3DPrincipledMaterial *pointerMaterial = nullptr;
        QQuick3DModel *pointer = m_selectionPointers.value(series);

        if (pointer) {
            // Retrieve the already created material
            QQmlListReference materialRef(pointer, "materials");
            pointerMaterial = qobject_cast<QQuick3DPrincipledMaterial *>(materialRef.at(0));
            // Delete old model
            delete pointer;
        } else {
            // No pointer yet; create material for it
            pointerMaterial = new QQuick3DPrincipledMaterial();
            pointerMaterial->setParent(this);
            pointerMaterial->setBaseColor(theme()->singleHighlightColor());
        }
        // Create new pointer
        pointer = new QQuick3DModel();
        pointer->setParent(parent);
        pointer->setParentItem(parent);
        pointer->setSource(QUrl(filename));
        pointer->setScale(QVector3D(0.05f, 0.05f, 0.05f));
        // Insert to the map
        m_selectionPointers.insert(series, pointer);
        // Set material
        QQmlListReference materialRef(pointer, "materials");
        materialRef.append(pointerMaterial);
    }
}

void QQuickGraphsSurface::changeSlicePointerForSeries(const QString &filename,
                                                      QSurface3DSeries *series)
{
    if (!filename.isEmpty()) {
        // Slice selection pointer
        QQuick3DNode *sliceParent = sliceView()->scene();

        QQuick3DPrincipledMaterial *slicePointerMaterial = nullptr;
        QQuick3DModel *slicePointer = m_sliceSelectionPointers.value(series);

        if (slicePointer) {
            // Retrieve the already created material
            QQmlListReference sliceMaterialRef(slicePointer, "materials");
            slicePointerMaterial = qobject_cast<QQuick3DPrincipledMaterial *>(
                sliceMaterialRef.at(0));
            // Delete old model
            delete slicePointer;
        } else {
            // No pointer yet; create material for it
            slicePointerMaterial = new QQuick3DPrincipledMaterial();
            slicePointerMaterial->setParent(sliceParent);
            slicePointerMaterial->setBaseColor(theme()->singleHighlightColor());
        }
        // Create new pointer
        slicePointer = new QQuick3DModel();
        slicePointer->setParent(sliceParent);
        slicePointer->setParentItem(sliceParent);
        slicePointer->setSource(QUrl(filename));
        slicePointer->setScale(QVector3D(0.05f, 0.05f, 0.05f));
        // Insert to the map
        m_sliceSelectionPointers.insert(series, slicePointer);
        // Set material
        QQmlListReference sliceMaterialRef(slicePointer, "materials");
        sliceMaterialRef.append(slicePointerMaterial);
    }
}

void QQuickGraphsSurface::handleFlipHorizontalGridChanged(bool flip)
{
    updateLabels();
    emit flipHorizontalGridChanged(flip);
    setFlipHorizontalGridChanged(false);
}

void QQuickGraphsSurface::adjustAxisRanges()
{
    QValue3DAxis *valueAxisX = static_cast<QValue3DAxis *>(m_axisX);
    QValue3DAxis *valueAxisY = static_cast<QValue3DAxis *>(m_axisY);
    QValue3DAxis *valueAxisZ = static_cast<QValue3DAxis *>(m_axisZ);
    bool adjustX = (valueAxisX && valueAxisX->isAutoAdjustRange());
    bool adjustY = (valueAxisY && valueAxisY->isAutoAdjustRange());
    bool adjustZ = (valueAxisZ && valueAxisZ->isAutoAdjustRange());
    bool first = true;

    if (adjustX || adjustY || adjustZ) {
        float minValueX = 0.0f;
        float maxValueX = 0.0f;
        float minValueY = 0.0f;
        float maxValueY = 0.0f;
        float minValueZ = 0.0f;
        float maxValueZ = 0.0f;
        qsizetype seriesCount = m_seriesList.size();
        for (int series = 0; series < seriesCount; series++) {
            const QSurface3DSeries *surfaceSeries = static_cast<QSurface3DSeries *>(
                m_seriesList.at(series));
            const QSurfaceDataProxy *proxy = surfaceSeries->dataProxy();
            if (surfaceSeries->isVisible() && proxy) {
                QVector3D minLimits;
                QVector3D maxLimits;
                proxy->d_func()->limitValues(minLimits,
                                             maxLimits,
                                             valueAxisX,
                                             valueAxisY,
                                             valueAxisZ);
                if (adjustX) {
                    if (first) {
                        // First series initializes the values
                        minValueX = minLimits.x();
                        maxValueX = maxLimits.x();
                    } else {
                        minValueX = qMin(minValueX, minLimits.x());
                        maxValueX = qMax(maxValueX, maxLimits.x());
                    }
                }
                if (adjustY) {
                    if (first) {
                        // First series initializes the values
                        minValueY = minLimits.y();
                        maxValueY = maxLimits.y();
                    } else {
                        minValueY = qMin(minValueY, minLimits.y());
                        maxValueY = qMax(maxValueY, maxLimits.y());
                    }
                }
                if (adjustZ) {
                    if (first) {
                        // First series initializes the values
                        minValueZ = minLimits.z();
                        maxValueZ = maxLimits.z();
                    } else {
                        minValueZ = qMin(minValueZ, minLimits.z());
                        maxValueZ = qMax(maxValueZ, maxLimits.z());
                    }
                }
                first = false;
            }
        }

        static const float adjustmentRatio = 20.0f;
        static const float defaultAdjustment = 1.0f;

        if (adjustX) {
            // If all points at same coordinate, need to default to some valid range
            float adjustment = 0.0f;
            if (minValueX == maxValueX) {
                if (adjustZ) {
                    // X and Z are linked to have similar unit size, so choose the valid
                    // range based on it
                    if (minValueZ == maxValueZ)
                        adjustment = defaultAdjustment;
                    else
                        adjustment = qAbs(maxValueZ - minValueZ) / adjustmentRatio;
                } else {
                    if (valueAxisZ)
                        adjustment = qAbs(valueAxisZ->max() - valueAxisZ->min()) / adjustmentRatio;
                    else
                        adjustment = defaultAdjustment;
                }
            }
            valueAxisX->d_func()->setRange(minValueX - adjustment, maxValueX + adjustment, true);
        }
        if (adjustY) {
            // If all points at same coordinate, need to default to some valid range
            // Y-axis unit is not dependent on other axes, so simply adjust +-1.0f
            float adjustment = 0.0f;
            if (minValueY == maxValueY)
                adjustment = defaultAdjustment;
            valueAxisY->d_func()->setRange(minValueY - adjustment, maxValueY + adjustment, true);
        }
        if (adjustZ) {
            // If all points at same coordinate, need to default to some valid range
            float adjustment = 0.0f;
            if (minValueZ == maxValueZ) {
                if (adjustX) {
                    // X and Z are linked to have similar unit size, so choose the valid
                    // range based on it
                    if (minValueX == maxValueX)
                        adjustment = defaultAdjustment;
                    else
                        adjustment = qAbs(maxValueX - minValueX) / adjustmentRatio;
                } else {
                    if (valueAxisX)
                        adjustment = qAbs(valueAxisX->max() - valueAxisX->min()) / adjustmentRatio;
                    else
                        adjustment = defaultAdjustment;
                }
            }
            valueAxisZ->d_func()->setRange(minValueZ - adjustment, maxValueZ + adjustment, true);
        }
    }
}

void QQuickGraphsSurface::handleArrayReset()
{
    QSurface3DSeries *series;
    if (qobject_cast<QSurfaceDataProxy *>(sender()))
        series = static_cast<QSurfaceDataProxy *>(sender())->series();
    else
        series = static_cast<QSurface3DSeries *>(sender());

    if (series->isVisible()) {
        adjustAxisRanges();
        setDataDirty(true);
    }
    if (!m_changedSeriesList.contains(series))
        m_changedSeriesList.append(series);

    // Clear selection unless still valid
    if (m_selectedPoint != invalidSelectionPosition())
        setSelectedPoint(m_selectedPoint, m_selectedSeries, false);
    series->d_func()->markItemLabelDirty();
    emitNeedRender();
}

void QQuickGraphsSurface::handleFlatShadingSupportedChange(bool supported)
{
    // Handle renderer flat surface support indicator signal. This happens exactly
    // once per renderer.
    if (m_flatShadingSupported != supported) {
        m_flatShadingSupported = supported;
        // Emit the change for all added surfaces
        for (QAbstract3DSeries *series : std::as_const(m_seriesList)) {
            QSurface3DSeries *surfaceSeries = static_cast<QSurface3DSeries *>(series);
            emit surfaceSeries->flatShadingSupportedChanged(m_flatShadingSupported);
        }
    }
}

void QQuickGraphsSurface::handleRowsChanged(qsizetype startIndex, qsizetype count)
{
    QSurface3DSeries *series = static_cast<QSurfaceDataProxy *>(QObject::sender())->series();
    qsizetype oldChangeCount = m_changedRows.size();
    if (!oldChangeCount)
        m_changedRows.reserve(count);

    int selectedRow = m_selectedPoint.x();
    for (qsizetype i = 0; i < count; i++) {
        bool newItem = true;
        qsizetype candidate = startIndex + i;
        for (qsizetype j = 0; j < oldChangeCount; j++) {
            const ChangeRow &oldChangeItem = m_changedRows.at(j);
            if (oldChangeItem.row == candidate && series == oldChangeItem.series) {
                newItem = false;
                break;
            }
        }
        if (newItem) {
            ChangeRow newChangeItem = {series, candidate};
            m_changedRows.append(newChangeItem);
            if (series == m_selectedSeries && selectedRow == candidate)
                series->d_func()->markItemLabelDirty();
        }
    }
    if (count) {
        m_changeTracker.rowsChanged = true;
        setDataDirty(true);

        if (series->isVisible())
            adjustAxisRanges();
        emitNeedRender();
    }
}

void QQuickGraphsSurface::handleItemChanged(qsizetype rowIndex, qsizetype columnIndex)
{
    QSurfaceDataProxy *sender = static_cast<QSurfaceDataProxy *>(QObject::sender());
    QSurface3DSeries *series = sender->series();

    bool newItem = true;
    QPoint candidate((int(rowIndex)), (int(columnIndex)));
    for (ChangeItem item : std::as_const(m_changedItems)) {
        if (item.point == candidate && item.series == series) {
            newItem = false;
            break;
        }
    }
    if (newItem) {
        ChangeItem newItem = {series, candidate};
        m_changedItems.append(newItem);
        m_changeTracker.itemChanged = true;
        setDataDirty(true);

        if (series == m_selectedSeries && m_selectedPoint == candidate)
            series->d_func()->markItemLabelDirty();

        if (series->isVisible())
            adjustAxisRanges();
        emitNeedRender();
    }
}

void QQuickGraphsSurface::handleRowsAdded(qsizetype startIndex, qsizetype count)
{
    Q_UNUSED(startIndex);
    Q_UNUSED(count);
    QSurface3DSeries *series = static_cast<QSurfaceDataProxy *>(sender())->series();
    if (series->isVisible()) {
        adjustAxisRanges();
        setDataDirty(true);
    }
    if (!m_changedSeriesList.contains(series))
        m_changedSeriesList.append(series);
    emitNeedRender();
}

void QQuickGraphsSurface::handleRowsInserted(qsizetype startIndex, qsizetype count)
{
    Q_UNUSED(startIndex);
    Q_UNUSED(count);
    QSurface3DSeries *series = static_cast<QSurfaceDataProxy *>(sender())->series();
    if (series == m_selectedSeries) {
        // If rows inserted to selected series before the selection, adjust the selection
        int selectedRow = m_selectedPoint.x();
        if (startIndex <= selectedRow) {
            selectedRow += count;
            setSelectedPoint(QPoint(selectedRow, m_selectedPoint.y()), m_selectedSeries, false);
        }
    }

    if (series->isVisible()) {
        adjustAxisRanges();
        setDataDirty(true);
    }
    if (!m_changedSeriesList.contains(series))
        m_changedSeriesList.append(series);

    emitNeedRender();
}

void QQuickGraphsSurface::handleRowsRemoved(qsizetype startIndex, qsizetype count)
{
    Q_UNUSED(startIndex);
    Q_UNUSED(count);
    QSurface3DSeries *series = static_cast<QSurfaceDataProxy *>(sender())->series();
    if (series == m_selectedSeries) {
        // If rows removed from selected series before the selection, adjust the selection
        int selectedRow = m_selectedPoint.x();
        if (startIndex <= selectedRow) {
            if ((startIndex + count) > selectedRow)
                selectedRow = -1; // Selected row removed
            else
                selectedRow -= count; // Move selected row down by amount of rows removed

            setSelectedPoint(QPoint(selectedRow, m_selectedPoint.y()), m_selectedSeries, false);
        }
    }

    if (series->isVisible()) {
        adjustAxisRanges();
        setDataDirty(true);
    }
    if (!m_changedSeriesList.contains(series))
        m_changedSeriesList.append(series);

    emitNeedRender();
}

QPoint QQuickGraphsSurface::invalidSelectionPosition()
{
    static QPoint invalidSelectionPoint(-1, -1);
    return invalidSelectionPoint;
}

void QQuickGraphsSurface::setSelectedPoint(const QPoint position,
                                           QSurface3DSeries *series,
                                           bool enterSlice)
{
    // If the selection targets non-existent point, clear selection instead.
    QPoint pos = position;
    m_selectionDirty = true;

    // Series may already have been removed, so check it before setting the selection.
    if (!m_seriesList.contains(series))
        series = 0;

    const QSurfaceDataProxy *proxy = 0;
    if (series)
        proxy = series->dataProxy();

    if (!proxy)
        pos = invalidSelectionPosition();

    if (pos != invalidSelectionPosition()) {
        qsizetype maxRow = proxy->rowCount() - 1;
        qsizetype maxCol = proxy->columnCount() - 1;

        if (pos.y() < 0 || pos.y() > maxRow || pos.x() < 0 || pos.x() > maxCol)
            pos = invalidSelectionPosition();
    }

    if (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Slice)) {
        bool previousSliceActive = isSlicingActive();
        if (pos == invalidSelectionPosition() || !series->isVisible()) {
            scene()->setSlicingActive(false);
        } else {
            // If the selected point is outside data window, or there is no selected
            // point, disable slicing
            float axisMinX = m_axisX->min();
            float axisMaxX = m_axisX->max();
            float axisMinZ = m_axisZ->min();
            float axisMaxZ = m_axisZ->max();

            QSurfaceDataItem item = series->dataArray().at(pos.y()).at(pos.x());
            if (item.x() < axisMinX || item.x() > axisMaxX || item.z() < axisMinZ
                || item.z() > axisMaxZ) {
                scene()->setSlicingActive(false);
            } else if (enterSlice) {
                scene()->setSlicingActive(true);
            }

        }
        if (previousSliceActive != isSlicingActive())
            setSliceActivatedChanged(true);

        emitNeedRender();
    }

    if (pos != m_selectedPoint || series != m_selectedSeries) {
        bool seriesChanged = (series != m_selectedSeries);
        m_selectedPoint = pos;
        m_selectedSeries = series;
        m_changeTracker.selectedPointChanged = true;

        // Clear selection from other series and finally set new selection to the
        // specified series
        for (QAbstract3DSeries *otherSeries : std::as_const(m_seriesList)) {
            QSurface3DSeries *surfaceSeries = static_cast<QSurface3DSeries *>(otherSeries);
            if (surfaceSeries != m_selectedSeries)
                surfaceSeries->d_func()->setSelectedPoint(invalidSelectionPosition());
        }
        if (m_selectedSeries)
            m_selectedSeries->d_func()->setSelectedPoint(m_selectedPoint);

        if (seriesChanged)
            emit selectedSeriesChanged(m_selectedSeries);

        auto hasSeries = [series](SurfaceModel *model){ return model->series == series; };
        auto hasVertex = [pos](SurfaceVertex vertex) {
            return vertex.coord == pos;
        };

        auto it = std::find_if(m_model.constBegin(), m_model.constEnd(), hasSeries);
        if (it != m_model.constEnd()) {
            SurfaceModel *model = *it;
            auto vIt = std::find_if(model->vertices.constBegin(), model->vertices.constEnd(), hasVertex);
            if (vIt != model->vertices.constEnd()) {
                model->selectedVertex = *vIt;
                model->picked = true;
            }
        }
        emitNeedRender();
    }
}

void QQuickGraphsSurface::setSelectionMode(QtGraphs3D::SelectionFlags mode)
{
    // Currently surface only supports row and column modes when also slicing
    if ((mode.testFlag(QtGraphs3D::SelectionFlag::Row)
         || mode.testFlag(QtGraphs3D::SelectionFlag::Column))
        && !mode.testFlag(QtGraphs3D::SelectionFlag::Slice)) {
        qCWarning(lcProperties3D, "%s unsupported selection mode",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
        return;
    } else if (mode.testFlag(QtGraphs3D::SelectionFlag::Slice)
               && (mode.testFlag(QtGraphs3D::SelectionFlag::Row)
                   == mode.testFlag(QtGraphs3D::SelectionFlag::Column))) {
        qCWarning(lcProperties3D, "%s must specify one of either row or column selection mode"
                 "in conjunction with slicing mode.", qUtf8Printable(QLatin1String(__FUNCTION__)));
    } else {
        QtGraphs3D::SelectionFlags oldMode = selectionMode();

        QQuickGraphsItem::setSelectionMode(mode);

        if (mode != oldMode) {
            // Refresh selection upon mode change to ensure slicing is correctly
            // updated according to series the visibility.
            setSelectedPoint(m_selectedPoint, m_selectedSeries, true);

            // Special case: Always deactivate slicing when changing away from slice
            // automanagement, as this can't be handled in setSelectedBar.
            if (!mode.testFlag(QtGraphs3D::SelectionFlag::Slice)
                && oldMode.testFlag(QtGraphs3D::SelectionFlag::Slice)) {
                scene()->setSlicingActive(false);
            }
        }
    }
}

void QQuickGraphsSurface::handleAxisAutoAdjustRangeChangedInOrientation(
    QAbstract3DAxis::AxisOrientation orientation, bool autoAdjust)
{
    Q_UNUSED(orientation);
    Q_UNUSED(autoAdjust);

    adjustAxisRanges();
}

void QQuickGraphsSurface::handleAxisRangeChangedBySender(QObject *sender)
{
    QQuickGraphsItem::handleAxisRangeChangedBySender(sender);

    // Update selected point - may be moved offscreen
    setSelectedPoint(m_selectedPoint, m_selectedSeries, false);
}

void QQuickGraphsSurface::handleSeriesVisibilityChangedBySender(QObject *sender)
{
    QQuickGraphsItem::handleSeriesVisibilityChangedBySender(sender);

    setSeriesVisibilityDirty(true);
    // Visibility changes may require disabling slicing,
    // so just reset selection to ensure everything is still valid.
    setSelectedPoint(m_selectedPoint, m_selectedSeries, false);
}

void QQuickGraphsSurface::handleItemLabelVisibleChangedBySender(bool visible, QObject *sender)
{
    auto series = static_cast<QSurface3DSeries *>(sender);

    if (series == m_selectedSeries) {
        itemLabel()->setVisible(visible);
        if (auto label = sliceItemLabel(); label && isSlicingActive())
            label->setVisible(visible);
    }
}

void QQuickGraphsSurface::setFlipHorizontalGrid(bool flip)
{
    if (m_flipHorizontalGrid != flip) {
        m_flipHorizontalGrid = flip;
        m_changeTracker.flipHorizontalGridChanged = true;
        emit flipHorizontalGridChanged(flip);
        emitNeedRender();
    }
}

bool QQuickGraphsSurface::flipHorizontalGrid() const
{
    return m_flipHorizontalGrid;
}

bool QQuickGraphsSurface::isFlatShadingSupported()
{
    return m_flatShadingSupported;
}

QList<QSurface3DSeries *> QQuickGraphsSurface::surfaceSeriesList()
{
    QList<QSurface3DSeries *> surfaceSeriesList;
    for (QAbstract3DSeries *abstractSeries : std::as_const(m_seriesList)) {
        QSurface3DSeries *surfaceSeries = qobject_cast<QSurface3DSeries *>(abstractSeries);
        if (surfaceSeries)
            surfaceSeriesList.append(surfaceSeries);
    }

    return surfaceSeriesList;
}

void QQuickGraphsSurface::updateSurfaceTexture(QSurface3DSeries *series)
{
    m_changeTracker.surfaceTextureChanged = true;

    if (!m_changedTextures.contains(series))
        m_changedTextures.append(series);

    emitNeedRender();
}

QQmlListProperty<QSurface3DSeries> QQuickGraphsSurface::seriesList()
{
    return QQmlListProperty<QSurface3DSeries>(this,
                                              this,
                                              &QQuickGraphsSurface::appendSeriesFunc,
                                              &QQuickGraphsSurface::countSeriesFunc,
                                              &QQuickGraphsSurface::atSeriesFunc,
                                              &QQuickGraphsSurface::clearSeriesFunc);
}

void QQuickGraphsSurface::appendSeriesFunc(QQmlListProperty<QSurface3DSeries> *list,
                                           QSurface3DSeries *series)
{
    reinterpret_cast<QQuickGraphsSurface *>(list->data)->addSeries(series);
}

qsizetype QQuickGraphsSurface::countSeriesFunc(QQmlListProperty<QSurface3DSeries> *list)
{
    return reinterpret_cast<QQuickGraphsSurface *>(list->data)->surfaceSeriesList().size();
}

QSurface3DSeries *QQuickGraphsSurface::atSeriesFunc(QQmlListProperty<QSurface3DSeries> *list,
                                                    qsizetype index)
{
    return reinterpret_cast<QQuickGraphsSurface *>(list->data)->surfaceSeriesList().at(index);
}

void QQuickGraphsSurface::clearSeriesFunc(QQmlListProperty<QSurface3DSeries> *list)
{
    QQuickGraphsSurface *declSurface = reinterpret_cast<QQuickGraphsSurface *>(list->data);
    QList<QSurface3DSeries *> realList = declSurface->surfaceSeriesList();
    qsizetype count = realList.size();
    for (qsizetype i = 0; i < count; i++)
        declSurface->removeSeries(realList.at(i));
}

void QQuickGraphsSurface::addSeries(QSurface3DSeries *series)
{
    Q_ASSERT(series && series->type() == QAbstract3DSeries::SeriesType::Surface);

    QQuickGraphsItem::addSeriesInternal(series);

    QSurface3DSeries *surfaceSeries = static_cast<QSurface3DSeries *>(series);
    if (surfaceSeries->selectedPoint() != invalidSelectionPosition())
        setSelectedPoint(surfaceSeries->selectedPoint(), surfaceSeries, false);

    if (!surfaceSeries->texture().isNull())
        updateSurfaceTexture(surfaceSeries);

    if (isReady())
        addModel(series);

    if (surfaceSeries->axisX())
        handleMultiAxisChanged(surfaceSeries->axisX());
    if (surfaceSeries->axisY())
        handleMultiAxisChanged(surfaceSeries->axisY());
    if (surfaceSeries->axisZ())
        handleMultiAxisChanged(surfaceSeries->axisZ());

    QObject::connect(series,
                     &QSurface3DSeries::axisXChanged,
                     this,
                     &QQuickGraphsSurface::handleMultiAxisChanged);
    QObject::connect(series,
                     &QSurface3DSeries::axisYChanged,
                     this,
                     &QQuickGraphsSurface::handleMultiAxisChanged);
    QObject::connect(series,
                     &QSurface3DSeries::axisZChanged,
                     this,
                     &QQuickGraphsSurface::handleMultiAxisChanged);
}

void QQuickGraphsSurface::removeSeries(QSurface3DSeries *series)
{
    bool wasVisible = (series && series->d_func()->m_graph == this && series->isVisible());

    int idx = m_seriesList.indexOf(series);
    if (series->axisX())
        QQuickGraphsItem::releaseMultiAxis(QAbstract3DAxis::AxisOrientation::X, idx);
    if (series->axisY())
        QQuickGraphsItem::releaseMultiAxis(QAbstract3DAxis::AxisOrientation::Y, idx);
    if (series->axisZ())
        QQuickGraphsItem::releaseMultiAxis(QAbstract3DAxis::AxisOrientation::Z, idx);

    QQuickGraphsItem::removeSeriesInternal(series);

    if (m_selectedSeries == series)
        setSelectedPoint(invalidSelectionPosition(), 0, false);

    if (wasVisible)
        adjustAxisRanges();


    QObject::disconnect(series,
                     &QSurface3DSeries::axisXChanged,
                     this,
                     &QQuickGraphsSurface::handleMultiAxisChanged);
    QObject::disconnect(series,
                     &QSurface3DSeries::axisYChanged,
                     this,
                     &QQuickGraphsSurface::handleMultiAxisChanged);
    QObject::disconnect(series,
                     &QSurface3DSeries::axisZChanged,
                     this,
                     &QQuickGraphsSurface::handleMultiAxisChanged);

    series->setParent(this); // Reparent as removing will leave series parentless
    for (int i = 0; i < m_model.size();) {
        if (m_model[i]->series == series) {
            m_model[i]->model->deleteLater();
            m_model[i]->gridModel->deleteLater();
            m_model[i]->fillModel->deleteLater();
            m_model[i]->heights.clear();
            m_fillDirty.remove(m_model.at(i));
            if (sliceView()) {
                m_model[i]->sliceModel->deleteLater();
                m_model[i]->sliceGridModel->deleteLater();
            }
            delete m_model[i];
            m_model.removeAt(i);
        } else {
            ++i;
        }
    }
}

void QQuickGraphsSurface::clearSelection()
{
    setSelectedPoint(invalidSelectionPosition(), 0, false);
    for (auto model : std::as_const(m_model))
        model->picked = false;
}

void QQuickGraphsSurface::handleAxisXChanged(QAbstract3DAxis *axis)
{
    emit axisXChanged(static_cast<QValue3DAxis *>(axis));
}

void QQuickGraphsSurface::handleAxisYChanged(QAbstract3DAxis *axis)
{
    emit axisYChanged(static_cast<QValue3DAxis *>(axis));
}

void QQuickGraphsSurface::handleAxisZChanged(QAbstract3DAxis *axis)
{
    emit axisZChanged(static_cast<QValue3DAxis *>(axis));
}

void QQuickGraphsSurface::handleMultiAxisChanged(QAbstract3DAxis *axis)
{
    QQuickGraphsItem::handleMultiAxisChanged(axis);
}

QAbstract3DAxis *QQuickGraphsSurface::getSeriesMultiAxis(QAbstract3DSeries *series,
                   QAbstract3DAxis::AxisOrientation orientation)
{
    QSurface3DSeries *surfSeries = qobject_cast<QSurface3DSeries *>(series);

    if (!surfSeries)
        return nullptr;

    QAbstract3DAxis *axis = nullptr;
    switch (orientation)  {
    case QAbstract3DAxis::AxisOrientation::X:
        axis = surfSeries->axisX();
    break;
    case QAbstract3DAxis::AxisOrientation::Y:
        axis = surfSeries->axisY();
    break;
    case QAbstract3DAxis::AxisOrientation::Z:
        axis = surfSeries->axisZ();
    break;
    case QAbstract3DAxis::AxisOrientation::None:
    break;
    }

    if (axis)
        axis->d_func()->setOrientation(orientation);

    return axis;
}

void QQuickGraphsSurface::componentComplete()
{
    QQuickGraphsItem::componentComplete();

    auto serieslist = surfaceSeriesList();
    for (auto series : std::as_const(serieslist)) {
        addModel(series);
        changePointerMeshTypeForSeries(series->mesh(), series);
    }

    graphsInputHandler()->setGraphsItem(this);
    qCDebug(lcGraphs3D, " QQuickGraphsSurface::componentComplete");
}

void QQuickGraphsSurface::synchData()
{
    qCDebug(lcGraphs3D, "%s start syncing", qUtf8Printable(QLatin1String(__FUNCTION__)));

    if (isFlipHorizontalGridChanged()) {
        setHorizontalFlipFactor(flipHorizontalGrid() ? -1 : 1);
        handleFlipHorizontalGridChanged(flipHorizontalGrid());
    }

    QQuickGraphsItem::synchData();

    if (isSelectedPointChanged()) {
        if (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Item))
            updateSelectedPoint();
        setSelectedPointChanged(false);
    }

    if (isSurfaceTextureChanged()) {
        if (!isChangedTexturesEmpty()) {
            for (auto model : std::as_const(m_model)) {
                if (hasSeriesToChangeTexture(model->series))
                    updateMaterial(model);
            }
        }
        setSurfaceTextureChanged(false);
    }

    if (gridLineType() == QtGraphs3D::GridLineType::Shader) {
        if (!m_topGrid) {
            //add horizontal top grid
            QUrl topGridUrl = QUrl(QStringLiteral(":/defaultMeshes/barMeshFull"));
            m_topGrid = new QQuick3DModel();
            m_topGridScale = new QQuick3DNode();
            m_topGridRotation = new QQuick3DNode();

            m_topGridScale->setParent(graphNode());
            m_topGridScale->setParentItem(graphNode());

            m_topGridRotation->setParent(m_topGridScale);
            m_topGridRotation->setParentItem(m_topGridScale);

            m_topGrid->setObjectName("Top Grid");
            m_topGrid->setParent(m_topGridRotation);
            m_topGrid->setParentItem(m_topGridRotation);

            m_topGrid->setSource(topGridUrl);
            m_topGrid->setPickable(false);
        }
        auto min = qMin(scaleWithBackground().x() + backgroundScaleMargin().x(),
                        scaleWithBackground().z() + backgroundScaleMargin().z());
        m_topGridScale->setScale(QVector3D(scaleWithBackground().x() + backgroundScaleMargin().x(),
                                           min * gridOffset(),
                                           scaleWithBackground().z() + backgroundScaleMargin().z()));
        m_topGridScale->setPosition(
            QVector3D(0.0f, scaleWithBackground().y() + backgroundScaleMargin().y(), 0.0f));

        m_topGrid->setVisible(m_flipHorizontalGrid);
        QQmlListReference materialsRefF(m_topGrid, "materials");
        QQmlListReference bbRef(background(), "materials");
        QQuick3DCustomMaterial *bgMatFloor;
        if (!materialsRefF.size() && bbRef.size()) {
            bgMatFloor = static_cast<QQuick3DCustomMaterial *>(bbRef.at(0));
            materialsRefF.append(bgMatFloor);
            bgMatFloor->setProperty("gridOnTop", m_flipHorizontalGrid);
        } else if (materialsRefF.size()) {
            bgMatFloor = static_cast<QQuick3DCustomMaterial *>(materialsRefF.at(0));
            bgMatFloor->setProperty("gridOnTop", m_flipHorizontalGrid);
        }
    }

    qCDebug(lcGraphs3D, "%s end syncing", qUtf8Printable(QLatin1String(__FUNCTION__)));
}

void QQuickGraphsSurface::updateGraph()
{
    for (auto model : std::as_const(m_model)) {
        bool seriesVisible = model->series->isVisible();
        if (isSeriesVisibilityDirty()) {
            if (!seriesVisible) {
                model->model->setVisible(seriesVisible);
                model->gridModel->setVisible(seriesVisible);
                if (sliceView()) {
                    model->sliceModel->setVisible(seriesVisible);
                    model->sliceGridModel->setVisible(seriesVisible);

                    if (m_selectedSeries == model->series) {
                        clearSelection();
                        setSliceActivatedChanged(true);
                        m_selectionDirty = !seriesVisible;
                    }
                }
                continue;
            }
        }

        if (model->model->visible() != seriesVisible)
            model->model->setVisible(seriesVisible);

        model->gridModel->setVisible(
            model->series->drawMode().testFlag(QSurface3DSeries::DrawWireframe) && seriesVisible);
        if (model->series->drawMode().testFlag(QSurface3DSeries::DrawSurface))
            model->model->setLocalOpacity(1.f);
        else
            model->model->setLocalOpacity(.0f);

        if (sliceView() && sliceView()->isVisible()) {
            model->sliceGridModel->setVisible(
                model->series->drawMode().testFlag(QSurface3DSeries::DrawWireframe));
            if (model->series->drawMode().testFlag(QSurface3DSeries::DrawSurface))
                model->sliceModel->setLocalOpacity(1.f);
            else
                model->sliceModel->setLocalOpacity(.0f);
        }
        updateMaterial(model);
    }

    setSeriesVisibilityDirty(false);
    if (isDataDirty() || isSeriesVisualsDirty()) {
        if (hasChangedSeriesList()) {
            handleChangedSeries();
        } else {
            for (auto model : std::as_const(m_model)) {
                bool visible = model->series->isVisible();
                if (visible)
                    updateModel(model);
            }
        }

        if (isSliceEnabled()) {
            if (!sliceView())
                createSliceView();

            if (sliceView()->isVisible()) {
                if (!m_selectedSeries) {
                    m_selectionDirty = true;
                    setSliceActivatedChanged(true);
                }
                toggleSliceGraph();
            }
        }

        setDataDirty(false);
        setSeriesVisualsDirty(false);
    }

    if (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Item))
        updateSelectedPoint();
}

void QQuickGraphsSurface::calculateSceneScalingFactors()
{
    float scaleX, scaleY, scaleZ;
    float marginH, marginV;

    if (margin() < 0.0f) {
        marginH = .1f;
        marginV = .1f;
    } else {
        marginH = margin();
        marginV = margin();
    }

    if (isPolar()) {
        float polarMargin = calculatePolarBackgroundMargin();
        marginH = qMax(marginH, polarMargin);
    }
    float hAspectRatio;
    if (isPolar())
        hAspectRatio = 1.0f;
    else
        hAspectRatio = horizontalAspectRatio();

    QSizeF areaSize;
    if (qFuzzyIsNull(hAspectRatio)) {
        areaSize.setHeight(axisZ()->max() - axisZ()->min());
        areaSize.setWidth(axisX()->max() - axisX()->min());
    } else {
        areaSize.setHeight(1.0);
        areaSize.setWidth(hAspectRatio);
    }

    float horizontalMaxDimension;
    if (aspectRatio() > 2.0f) {
        horizontalMaxDimension = 2.0f;
        scaleY = 2.0f / aspectRatio();
    } else {
        horizontalMaxDimension = aspectRatio();
        scaleY = 1.0f;
    }

    if (isPolar())
        m_polarRadius = horizontalMaxDimension;

    float scaleFactor = qMax(areaSize.width(), areaSize.height());
    scaleX = horizontalMaxDimension * areaSize.width() / scaleFactor;
    scaleZ = horizontalMaxDimension * areaSize.height() / scaleFactor;

    setScale(QVector3D(scaleX, scaleY, scaleZ));
    setScaleWithBackground(QVector3D(scaleX, scaleY, scaleZ));
    setBackgroundScaleMargin(QVector3D(marginH, marginV, marginH));
}

void QQuickGraphsSurface::handleChangedSeries()
{
    auto changedSeries = changedSeriesList();
    for (auto series : std::as_const(changedSeries)) {
        for (auto model : std::as_const(m_model)) {
            if (model->series == series) {
                updateModel(model);
            }
        }
    }
}

inline static float getDataValue(const QSurfaceDataArray &array, bool searchRow, qsizetype index)
{
    if (searchRow)
        return array.at(0).at(index).x();
    else
        return array.at(index).at(0).z();
}

inline static int binarySearchArray(const QSurfaceDataArray &array,
                                    qsizetype maxIndex,
                                    float limitValue,
                                    bool searchRow,
                                    bool lowBound,
                                    bool ascending)
{
    qsizetype min = 0;
    qsizetype max = maxIndex;
    qsizetype mid = 0;
    qsizetype retVal;

    while (max >= min) {
        mid = (min + max) / 2;
        float arrayValue = getDataValue(array, searchRow, mid);
        if (arrayValue == limitValue)
            return int(mid);
        if (ascending) {
            if (arrayValue < limitValue)
                min = mid + 1;
            else
                max = mid - 1;
        } else {
            if (arrayValue > limitValue)
                min = mid + 1;
            else
                max = mid - 1;
        }
    }

    if (lowBound == ascending) {
        if (mid > max)
            retVal = mid;
        else
            retVal = min;
    } else {
        if (mid > max)
            retVal = max;
        else
            retVal = mid;
    }

    if (retVal < 0 || retVal > maxIndex) {
        retVal = -1;
    } else if (lowBound) {
        if (getDataValue(array, searchRow, retVal) < limitValue)
            retVal = -1;
    } else {
        if (getDataValue(array, searchRow, retVal) > limitValue)
            retVal = -1;
    }
    return int(retVal);
}

QRect QQuickGraphsSurface::calculateSampleSpace(SurfaceModel *model, const QSurfaceDataArray &array)
{
    QRect sampleSpace;

    QValue3DAxis *xAxis = model->series->axisX();
    if (!xAxis)
        xAxis = axisX();

    QValue3DAxis *zAxis = model->series->axisZ();
    if (!zAxis)
        zAxis = axisZ();

    if (array.size() > 0) {
        if (array.size() >= 1 && array.at(0).size() >= 1) {
            const qsizetype maxRow = array.size() - 1;
            const qsizetype maxColumn = array.at(0).size() - 1;

            const bool ascendingX = array.at(0).at(0).x() < array.at(0).at(maxColumn).x();
            const bool ascendingZ = array.at(0).at(0).z() < array.at(maxRow).at(0).z();

            if (array.size() > 1 && array.at(0).size() > 1) {
                // Check if Z is filled before X. If it is, or there's something else that is fishy,
                // print out a warning about incorrectly formed data.
                bool incorrectDataFormat = false;

                qreal val = array.at(0).at(0).z();
                qreal step = array.at(1).at(0).z() - array.at(0).at(0).z();
                if (maxRow > 1) {
                    if ((val + step * maxRow == array.at(maxRow).at(0).z() && !ascendingZ)
                        || (val - step * maxRow == array.at(maxRow).at(0).z() && ascendingZ)) {
                        incorrectDataFormat = true;
                    }
                }
                val = array.at(0).at(0).x();
                step = array.at(0).at(1).x() - array.at(0).at(0).x();
                if (maxColumn > 1) {
                    if ((val + step * maxColumn == array.at(0).at(maxColumn).x() && !ascendingX)
                        || (val - step * maxColumn == array.at(0).at(maxColumn).x() && ascendingX)) {
                        incorrectDataFormat = true;
                    }
                }

                if (incorrectDataFormat) {
                    qCWarning(lcProperties3D,
                              "Data might be in an incorrect format. If the graph looks wrong or "
                              "is displayed only partially, verify that rows are filled first, "
                              "and columns after.");
                }
            }

            if (model->ascendingX != ascendingX) {
                setIndexDirty(true);
                model->ascendingX = ascendingX;
            }
            if (model->ascendingZ != ascendingZ) {
                setIndexDirty(true);
                model->ascendingZ = ascendingZ;
            }

            float cutoffMargin = QQuickGraphsItem::cutoffMargin();
            float axisXMin = xAxis->min() - cutoffMargin;
            int idx = binarySearchArray(array, maxColumn, axisXMin, true, true, ascendingX);
            if (idx != -1) {
                if (ascendingX)
                    sampleSpace.setLeft(idx);
                else
                    sampleSpace.setRight(idx);
            } else {
                sampleSpace.setWidth(-1);
                return sampleSpace;
            }

            float axisXMax = xAxis->max() + cutoffMargin;
            idx = binarySearchArray(array, maxColumn, axisXMax, true, false, ascendingX);
            if (idx != -1) {
                if (ascendingX)
                    sampleSpace.setRight(idx);
                else
                    sampleSpace.setLeft(idx);
            } else {
                sampleSpace.setWidth(-1); // to indicate nothing needs to be shown
                return sampleSpace;
            }

            float axisZMin = zAxis->min() - cutoffMargin;
            idx = binarySearchArray(array, maxRow, axisZMin, false, true, ascendingZ);
            if (idx != -1) {
                if (ascendingZ)
                    sampleSpace.setTop(idx);
                else
                    sampleSpace.setBottom(idx);
            } else {
                sampleSpace.setWidth(-1); // to indicate nothing needs to be shown
                return sampleSpace;
            }

            float axisZMax = zAxis->max() + cutoffMargin;
            idx = binarySearchArray(array, maxRow, axisZMax, false, false, ascendingZ);
            if (idx != -1) {
                if (ascendingZ)
                    sampleSpace.setBottom(idx);
                else
                    sampleSpace.setTop(idx);
            } else {
                sampleSpace.setWidth(-1); // to indicate nothing needs to be shown
                return sampleSpace;
            }
        }
    }
    return sampleSpace;
}

QSurfaceDataArray QQuickGraphsSurface::removeNaNRows(const QSurfaceDataArray &array)
{
    QSurfaceDataArray sanitizedArray;

    for (QSurfaceDataRow row : array) {
        bool foundNan = false;
        for (QSurfaceDataItem value : row) {
            if (qIsNaN(value.x()) || qIsNaN(value.y()) || qIsNaN(value.z())) {
                foundNan = true;
                break;
            }
        }
        if (!foundNan)
            sanitizedArray.append(row);
    }

    return sanitizedArray;
}

void QQuickGraphsSurface::updateModel(SurfaceModel *model)
{
    QSurfaceDataArray array = model->series->dataArray();
    if (model->series->rowsSanitized())
        array = removeNaNRows(array);

    if (!array.isEmpty()) {
        Q_TRACE(QGraphs3DSurfaceModelUpdate_entry, static_cast<void *>(model));
        qsizetype rowCount = array.size();
        qsizetype columnCount = array.at(0).size();

        const qsizetype maxSize = 4096; // maximum texture size
        columnCount = qMin(maxSize, columnCount);
        rowCount = qMin(maxSize, rowCount);

        bool lineData = (rowCount == 1 || columnCount == 1);

        if (model->rowCount != rowCount) {
            model->rowCount = rowCount;
            setIndexDirty(true);
        }
        if (model->columnCount != columnCount) {
            model->columnCount = columnCount;
            setIndexDirty(true);
        }

        bool dimensionsChanged = false;
        QRect sampleSpace = calculateSampleSpace(model, array);
        if (sampleSpace != model->sampleSpace) {
            dimensionsChanged = true;
            model->sampleSpace = sampleSpace;
        }
        int rowStart = sampleSpace.top();
        int columnStart = sampleSpace.left();
        int rowLimit = sampleSpace.bottom() + 1;
        int columnLimit = sampleSpace.right() + 1;

        QPoint selC = model->selectedVertex.coord;
        selC.setX(qMin(selC.x(), int(columnCount) - 1));
        selC.setY(qMin(selC.y(), int(rowCount) - 1));
        QVector3D selP = array.at(selC.y()).at(selC.x()).position();

        QValue3DAxis *xAxis = model->series->axisX();
        if (!xAxis)
            xAxis = axisX();

        QValue3DAxis *zAxis = model->series->axisZ();
        if (!zAxis)
            zAxis = axisZ();

        bool pickOutOfRange = false;

        if (selP.x() < xAxis->min() || selP.x() > xAxis->max() || selP.z() < zAxis->min()
            || selP.z() > zAxis->max()) {
            pickOutOfRange = true;
        }

        if (m_isIndexDirty || pickOutOfRange) {
            model->selectedVertex = SurfaceVertex();
            if (sliceView() && sliceView()->isVisible() && model->series == m_selectedSeries) {
                setSlicingActive(false);
                setSliceActivatedChanged(true);
                m_selectionDirty = true;
            }
        }
        qsizetype totalSize = rowCount * columnCount * 2;
        float uvX = 1.0f / float(columnCount - 1);
        float uvY = 1.0f / float(rowCount - 1);

        bool flatShading = model->series->shading() == QSurface3DSeries::Shading::Flat;

        QVector3D boundsMin = QVector3D();
        QVector3D boundsMax = QVector3D();

        model->heights.clear();
        model->heights.reserve(totalSize);

        QQmlListReference materialRef(model->model, "materials");
        auto material = materialRef.at(0);
        QVariant heightInputAsVariant = material->property("height");
        QQuick3DShaderUtilsTextureInput *heightInput
            = heightInputAsVariant.value<QQuick3DShaderUtilsTextureInput *>();
        QQuick3DTexture *heightMap = heightInput->texture();
        QQuick3DTextureData *heightMapData = nullptr;
        if (!heightMap) {
            heightMap = new QQuick3DTexture();
            heightMap->setParent(this);
            heightMap->setHorizontalTiling(QQuick3DTexture::ClampToEdge);
            heightMap->setVerticalTiling(QQuick3DTexture::ClampToEdge);
            heightMap->setMinFilter(QQuick3DTexture::Nearest);
            heightMap->setMagFilter(QQuick3DTexture::Nearest);
            heightMapData = new QQuick3DTextureData();
            heightMapData->setSize(QSize(sampleSpace.width(), sampleSpace.height()));
            heightMapData->setFormat(QQuick3DTextureData::RGBA32F);
            heightMapData->setParent(heightMap);
            heightMapData->setParentItem(heightMap);
        } else {
            heightMapData = heightMap->textureData();
            if (dimensionsChanged)
                heightMapData->setSize(QSize(sampleSpace.width(), sampleSpace.height()));
        }

        if (heightMapData->size().width() < 1 || heightMapData->size().height() < 1) {
            heightMapData->setTextureData(QByteArray());
            heightMap->setTextureData(heightMapData);
            heightInput->setTexture(heightMap);
            model->heightTexture = heightMap;
            Q_TRACE(QGraphs3DSurfaceModelUpdate_exit);
            return;
        }

        material->setProperty("xDiff", 1.0f / float(sampleSpace.width() - 1));
        material->setProperty("yDiff", 1.0f / float(sampleSpace.height() - 1));
        material->setProperty("flatShading", flatShading);
        material->setProperty("graphHeight",
                              scaleWithBackground().y() + QQuickGraphsItem::cutoffMargin());
        material->setProperty("uvOffset", QVector2D(columnStart, rowStart));
        material->setProperty("size", QVector2D(sampleSpace.width(), sampleSpace.height()));
        material->setProperty("vertCount", QVector2D(columnCount, rowCount));
        material->setProperty("flipU", !model->ascendingX);
        material->setProperty("flipV", !model->ascendingZ);
        material->setProperty("fill", model->series->drawMode().testFlag(QSurface3DSeries::DrawFilledSurface));
        material->setProperty("lineData", lineData);
        for (int i = 0; i < m_seriesList.size(); i++) {
            if (m_seriesList.at(i) == model->series)
                material->setProperty("order", i);
        }

        model->vertices.clear();
        model->vertices.reserve(totalSize);

        for (int i = rowStart; i < rowLimit; i++) {
            const QSurfaceDataRow &row = array.at(i);
            for (int j = columnStart; j < columnLimit; j++) {
                QVector3D pos = getNormalizedVertex(row.at(j), isPolar(), false, model->series);

                float alpha = 1.0f;
                for (int axis = 0; axis < 3; axis++) {
                    if (qIsNaN(pos[axis])) {
                        pos[axis] = 0;
                        alpha = .0f;
                    }
                }
                model->heights.push_back(QVector4D(pos, alpha));
                SurfaceVertex vertex;
                vertex.position = pos;
                vertex.uv = QVector2D(j * uvX, i * uvY);
                vertex.coord = QPoint(j, i);
                model->vertices.push_back(vertex);
                if (!qIsNaN(pos.y()) && !qIsInf(pos.y())) {
                    if (boundsMin.isNull()) {
                        boundsMin = pos;
                    } else {
                        boundsMin = QVector3D(qMin(boundsMin.x(), pos.x()),
                                              qMin(boundsMin.y(), pos.y()),
                                              qMin(boundsMin.z(), pos.z()));
                    }
                }
                if (boundsMax.isNull()) {
                    boundsMax = pos;
                } else {
                    boundsMax = QVector3D(qMax(boundsMax.x(), pos.x()),
                                          qMax(boundsMax.y(), pos.y()),
                                          qMax(boundsMax.z(), pos.z()));
                }
            }
        }

        if (model->boundsMin != boundsMin || model->boundsMax != boundsMax) {
            model->boundsMin = boundsMin;
            model->boundsMax = boundsMax;
            float range = boundsMax.y() - boundsMin.y();
            material->setProperty("gradientMin", -(boundsMin.y() / range));
            material->setProperty("gradientHeight", 1.0f / range);
        }
        QByteArray heightData = QByteArray(reinterpret_cast<char *>(model->heights.data()),
                                           model->heights.size() * sizeof(QVector4D));
        heightMapData->setTextureData(heightData);
        heightMap->setTextureData(heightMapData);
        heightInput->setTexture(heightMap);
        model->heightTexture = heightMap;

        if (m_isIndexDirty) {
            QList<SurfaceVertex> vertices;
            for (int i = 0; i < rowCount; i++) {
                QSurfaceDataRow row = array.at(i);
                for (int j = 0; j < columnCount; j++) {
                    SurfaceVertex vertex;
                    QVector3D pos = getNormalizedVertex(row.at(j), isPolar(), false, model->series);

                    for (int axis = 0; axis < 3; axis++) {
                        if (qIsNaN(pos[axis]))
                            pos[axis] = 0;
                    }
                    vertex.position = pos;
                    float uStep = model->ascendingX ? j * uvX : 1 - (j * uvX);
                    float vStep = model->ascendingZ ? i * uvY : 1 - (i * uvY);

                    vertex.uv = QVector2D(uStep, vStep);
                    vertex.coord = QPoint(j, i);
                    vertices.push_back(vertex);
                }
            }
            if (lineData) {
                createLineIndices(model, rowCount);
                model->model->geometry()->setPrimitiveType(QQuick3DGeometry::PrimitiveType::LineStrip);

            } else {
                createIndices(model, columnCount, rowCount);
                model->model->geometry()->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
            }

            auto geometry = model->model->geometry();
            QByteArray vertexBuffer(reinterpret_cast<char *>(vertices.data()),
                                    vertices.size() * sizeof(SurfaceVertex));
            geometry->setVertexData(vertexBuffer);
            QByteArray indexBuffer(reinterpret_cast<char *>(model->indices.data()),
                                   model->indices.size() * sizeof(quint32));
            geometry->setIndexData(indexBuffer);
            geometry->setBounds(boundsMin, boundsMax);
            geometry->update();

            createGridlineIndices(model, 0, 0, columnCount, rowCount);
            auto gridGeometry = model->gridModel->geometry();
            gridGeometry->setVertexData(vertexBuffer);
            QByteArray gridIndexBuffer(reinterpret_cast<char *>(model->gridIndices.data()),
                                       model->gridIndices.size() * sizeof(quint32));
            gridGeometry->setIndexData(gridIndexBuffer);
            gridGeometry->setBounds(boundsMin, boundsMax);
            gridGeometry->update();
            m_fillDirty[model] = true;
            m_isIndexDirty = false;
        }
        QQmlListReference gridMaterialRef(model->gridModel, "materials");
        auto gridMaterial = gridMaterialRef.at(0);
        QVariant gridHeightInputAsVariant = gridMaterial->property("height");
        QQuick3DShaderUtilsTextureInput *gridHeightInput
            = gridHeightInputAsVariant.value<QQuick3DShaderUtilsTextureInput *>();
        gridHeightInput->setTexture(heightMap);
        QColor gridColor = model->series->wireframeColor();
        gridMaterial->setProperty("gridColor", gridColor);
        gridMaterial->setProperty("range", QVector2D(sampleSpace.width(), sampleSpace.height()));
        gridMaterial->setProperty("vertices", QVector2D(columnCount, rowCount));
        gridMaterial->setProperty("graphHeight",
                                  scaleWithBackground().y() + QQuickGraphsItem::cutoffMargin());
        gridMaterial->setProperty("fill", model->series->drawMode().testFlag(QSurface3DSeries::DrawFilledSurface));

        qCDebug(lcGraphs3D) << "surface info"
            << "\n vertices:" << model->vertices.count()
            << "\n indices:" << model->indices.count()
            << "\n dataArray dimensions:" << QVector2D(model->series->dataArray().size(), model->series->dataArray().at(0).size())
            << "\n plane size:" << QVector2D(sampleSpace.width(), sampleSpace.height())
            << "\n vertCount:" << QVector2D(columnCount, rowCount);


        if (lineData)
            updateLineFill(model);
        else
            updateFill(model);
        Q_TRACE(QGraphs3DSurfaceModelUpdate_exit);
    }
    updateMaterial(model);
    updateSelectedPoint();
}

void QQuickGraphsSurface::updateFill(SurfaceModel *model)
{
    bool fillVisible = model->series->drawMode().testFlag(QSurface3DSeries::DrawFilledSurface);
    if (m_fillDirty[model] && fillVisible) {
        QList<QList<SurfaceVertex>> sideVertsList(4);
        qsizetype rowCount = model->rowCount;
        qsizetype colCount = model->columnCount;

        float uvX = 1.0f / float(colCount - 1);
        float uvY = 1.0f / float(rowCount - 1);
        for (int i = 0; i < rowCount; i++) {
            for (int j = 0; j < colCount; j++) {
                SurfaceVertex vertex;
                QVector3D pos = QVector3D(0, 0, 0);
                vertex.position = pos;

                vertex.uv = QVector2D(j * uvX, i * uvY);
                vertex.coord = QPoint(j, i);
                float base = -scaleWithBackground().y();

                auto addVerts = [base](SurfaceVertex vertex,
                                       QList<SurfaceVertex> &sideVerts,
                                       QVector2D uvOffset,
                                       bool rev = false) {
                    // add small offset to uv to distinguish them in the shaders
                    vertex.uv += uvOffset * 0.1f;
                    SurfaceVertex botVert = vertex;
                    botVert.position.setY(base);
                    botVert.uv += uvOffset;
                    if (rev) {
                        sideVerts.push_front(botVert);
                        sideVerts.push_front(vertex);
                    } else {
                        sideVerts.push_back(vertex);
                        sideVerts.push_back(botVert);
                    }
                };

                if (i == 0) { //Bottom row
                    QList<SurfaceVertex> &sideVerts = sideVertsList[0];
                    addVerts(vertex, sideVerts, QVector2D(0.0f, -0.1f));
                } else if (j == colCount - 1) {
                    QList<SurfaceVertex> &sideVerts = sideVertsList[1];
                    addVerts(vertex, sideVerts, QVector2D(0.1f, 0.0f));
                } else if (i == rowCount - 1) {
                    QList<SurfaceVertex> &sideVerts = sideVertsList[2];
                    addVerts(vertex, sideVerts, QVector2D(0.0f, 0.1f), true);
                } else if (j == 0) {
                    QList<SurfaceVertex> &sideVerts = sideVertsList[3];
                    addVerts(vertex, sideVerts, QVector2D(-0.1f, 0.0f), true);
                }
            }
        }

        QList<SurfaceVertex> sideVerts;
        for (const auto &side : std::as_const(sideVertsList)) {
            for (auto vert : side)
                sideVerts.append(vert);
        }
        auto sideGeom = model->fillModel->geometry();
        QByteArray fillVertBuffer(reinterpret_cast<char *>(sideVerts.data()),
                                  sideVerts.size() * sizeof(SurfaceVertex));
        sideGeom->setVertexData(fillVertBuffer);
        sideGeom->setBounds(model->boundsMin, model->boundsMax);
        sideGeom->update();

        QList<quint32> indices;
        int totVerts = (rowCount * 2 + colCount * 2) - 4;
        int idxCount = 6 * (totVerts);
        indices.reserve(idxCount);

        //side 1
        for (int i = 0; i < totVerts - 1; i++) {
            quint32 start = i * 2;
            indices.push_back(start + 2);
            indices.push_back(start);
            indices.push_back(start + 1);

            indices.push_back(start + 2);
            indices.push_back(start + 1);
            indices.push_back(start + 3);
        }

        //Connecting triangles
        quint32 start = (totVerts - 1) * 2;
        indices.push_back(0);
        indices.push_back(start);
        indices.push_back(start + 1);

        indices.push_back(0);
        indices.push_back(start + 1);
        indices.push_back(1);

        QByteArray indexBuffer(reinterpret_cast<char *>(indices.data()),
                               indices.size() * sizeof(quint32));

        auto geometry = model->fillModel->geometry();
        geometry->setIndexData(indexBuffer);

        m_fillDirty[model] = false;
    }

    bool surfaceVisible = model->series->drawMode().testFlag(QSurface3DSeries::DrawSurface)
                          && model->series->isVisible();
    model->fillModel->setVisible(
        model->series->drawMode().testFlag(QSurface3DSeries::DrawFilledSurface) && surfaceVisible);
}

void QQuickGraphsSurface::updateLineFill(SurfaceModel *model)
{
    bool fillVisible = model->series->drawMode().testFlag(QSurface3DSeries::DrawFilledSurface);
    if (m_fillDirty[model] && fillVisible) {
        qsizetype rowCount = model->rowCount;
        qsizetype colCount = model->columnCount;

        bool oneRow = rowCount == 1;

        float uvX = 1.0f / qMax(float(colCount - 1), 1.0f);
        float uvY = 1.0f / qMax(float(rowCount - 1), 1.0f);
        QList<SurfaceVertex> vertices;
        for (int i = 0; i < rowCount; i++) {
            for (int j = 0; j < colCount; j++) {
                SurfaceVertex vertex;
                QVector3D pos = QVector3D(0, 0, 0);
                vertex.position = pos;

                vertex.uv = QVector2D(j * uvX, i * uvY);
                vertex.coord = QPoint(j, i);
                float base = -scaleWithBackground().y();

                QVector2D uvOffset;
                if (oneRow)
                     uvOffset = QVector2D(0.0f, -0.1f);
                else
                     uvOffset = QVector2D(-0.1f, 0.0f);
                vertex.uv += uvOffset * 0.1f;

                SurfaceVertex botVert = vertex;
                botVert.position.setY(base);
                botVert.uv += uvOffset;

                vertices.push_back(vertex);
                vertices.push_back(botVert);

            }
        }
        QList<quint32> indices;

        int totVerts = oneRow? colCount : rowCount;
        int idxCount = 6 * (totVerts);
        indices.reserve(idxCount);

        for (int i = 0; i < totVerts - 1; i++) {
            quint32 start = i * 2 ;
            indices.push_back(start + 2);
            indices.push_back(start);
            indices.push_back(start + 1);

            indices.push_back(start + 2);
            indices.push_back(start + 1);
            indices.push_back(start + 3);
        }

        QByteArray indexBuffer(reinterpret_cast<char *>(indices.data()),
                               indices.size() * sizeof(quint32));

        auto sideGeom = model->fillModel->geometry();
        QByteArray fillVertBuffer(reinterpret_cast<char *>(vertices.data()),
                                  vertices.size() * sizeof(SurfaceVertex));
        sideGeom->setVertexData(fillVertBuffer);
        sideGeom->setBounds(model->boundsMin, model->boundsMax);
        sideGeom->setIndexData(indexBuffer);
        sideGeom->update();
        m_fillDirty[model] = false;

    }

    bool surfaceVisible = model->series->drawMode().testFlag(QSurface3DSeries::DrawSurface)
        && model->series->isVisible();
    model->fillModel->setVisible(
        model->series->drawMode().testFlag(QSurface3DSeries::DrawFilledSurface) && surfaceVisible);
}

void QQuickGraphsSurface::updateMaterial(SurfaceModel *model)
{
    QQmlListReference materialRef(model->model, "materials");

    QQuick3DCustomMaterial *material = qobject_cast<QQuick3DCustomMaterial *>(materialRef.at(0));

    if (!material) {
        material = createQmlCustomMaterial(QStringLiteral(":/materials/SurfaceMaterial"));
        model->customMaterial = material;
    }

    bool textured = !(model->series->texture().isNull() && model->series->textureFile().isEmpty());
    material->setProperty("textured", textured);

    if (isSeriesVisualsDirty()) {
        switch (model->series->colorStyle()) {
        case (QGraphsTheme::ColorStyle::ObjectGradient):
            material->setProperty("colorStyle", 0);
            break;
        case (QGraphsTheme::ColorStyle::RangeGradient):
            material->setProperty("colorStyle", 1);
            break;
        case (QGraphsTheme::ColorStyle::Uniform):
            material->setProperty("colorStyle", 2);
            material->setProperty("uniformColor", model->series->baseColor());
            break;
        }

        bool flatShading = model->series->shading() == QSurface3DSeries::Shading::Flat;

        QVariant textureInputAsVariant = material->property("custex");
        QQuick3DShaderUtilsTextureInput *textureInput
            = textureInputAsVariant.value<QQuick3DShaderUtilsTextureInput *>();
        auto textureData = static_cast<QQuickGraphsTextureData *>(model->gradientTexture->textureData());
        textureData->createGradient(model->series->baseGradient());
        textureInput->setTexture(model->gradientTexture);

        QVariant heightInputAsVariant = material->property("height");
        QQuick3DShaderUtilsTextureInput *heightInput
            = heightInputAsVariant.value<QQuick3DShaderUtilsTextureInput *>();
        heightInput->setTexture(model->heightTexture);
        material->setParent(model->model);
        material->setParentItem(model->model);
        material->setCullMode(QQuick3DMaterial::NoCulling);
        material->setProperty("flatShading", flatShading);
        material->setProperty("shaded",
                              model->series->lightingMode()
                                  == QAbstract3DSeries::LightingMode::Shaded);

        // Not textured, clear transparency flag
        if (!textured)
            model->texture->textureData()->setHasTransparency(false);
    }

    if (textured) {
        QQuick3DShaderUtilsTextureInput *texInput = material->property("baseColor")
                                                        .value<QQuick3DShaderUtilsTextureInput *>();
        if (!texInput->texture()) {
            QQuick3DTexture *texture = new QQuick3DTexture();
            texture->setParent(material);
            texture->setParentItem(material);
            texInput->setTexture(texture);
        }
        if (!texInput->texture()->hasSourceData() || isSeriesVisualsDirty()) {
            if (!model->series->textureFile().isEmpty()) {
                texInput->texture()->setSource(QUrl::fromLocalFile(model->series->textureFile()));
                // Check for transparency
                model->texture->textureData()->setHasTransparency(
                    Utils::imageHasTransparency(model->series->texture()));
            } else if (!model->series->texture().isNull()) {
                QImage image = model->series->texture();
                // Check for transparency
                bool hasTransparency = Utils::imageHasTransparency(image);
                image.convertTo(QImage::Format_RGBA32FPx4);
                auto textureData = static_cast<QQuickGraphsTextureData *>(
                    model->texture->textureData());
                textureData->setHasTransparency(hasTransparency);
                textureData->setFormat(QQuick3DTextureData::RGBA32F);
                textureData->setSize(image.size());
                textureData->setTextureData(
                    QByteArray(reinterpret_cast<const char *>(image.bits()), image.sizeInBytes()));
                texInput->texture()->setTextureData(textureData);
                texInput->texture()->setVerticalTiling(QQuick3DTexture::ClampToEdge);
                texInput->texture()->setHorizontalTiling(QQuick3DTexture::ClampToEdge);
            } else {
                texInput->texture()->setSource(QUrl());
            }
        }
    }
    material->setProperty("rootScale", rootNode()->scale().y() * scaleWithBackground().y());

    bool colorHasTransparency = false;
    if (model->series->colorStyle() == QGraphsTheme::ColorStyle::Uniform)
        colorHasTransparency = model->series->baseColor().alphaF() < 1.0;
    else
        colorHasTransparency = model->gradientTexture->textureData()->hasTransparency();

    bool textureHasTransparency = model->texture->textureData()->hasTransparency();

    material->setProperty("hasTransparency", colorHasTransparency || textureHasTransparency);
    material->update();
}

QVector3D QQuickGraphsSurface::getNormalizedVertex(const QSurfaceDataItem &data,
                                                   bool polar,
                                                   bool flipXZ,
                                                   QSurface3DSeries *series)
{
    Q_UNUSED(flipXZ);

    QValue3DAxis *xAxis = series? series->axisX() : axisX();
    if (!xAxis)
        xAxis = axisX();

    QValue3DAxis *yAxis = series? series->axisY() : axisY();
    if (!yAxis)
        yAxis = axisY();

    QValue3DAxis *zAxis = series? series->axisZ() : axisZ();
    if (!zAxis)
        zAxis = axisZ();

    QValue3DAxis *axisXValue = static_cast<QValue3DAxis *>(xAxis);
    QValue3DAxis *axisYValue = static_cast<QValue3DAxis *>(yAxis);
    QValue3DAxis *axisZValue = static_cast<QValue3DAxis *>(zAxis);

    bool xReversed = axisXValue->reversed();
    bool yReversed = axisYValue->reversed();
    bool zReversed = axisZValue->reversed();

    float normalizedX = xReversed ? 1.f - axisXValue->positionAt(data.x())
                                  : axisXValue->positionAt(data.x());
    float normalizedY;
    float normalizedZ = zReversed ? 1.f - axisZValue->positionAt(data.z())
                                  : axisZValue->positionAt(data.z());

    // TODO : Need to handle, flipXZ

    float scale, translate;
    if (polar) {
        float angle = normalizedX * M_PI * 2.0f;
        float radius = normalizedZ * this->scaleWithBackground().z();
        normalizedX = radius * qSin(angle) * 1.0f;
        normalizedZ = -(radius * qCos(angle)) * 1.0f;
    } else {
        scale = translate = this->scaleWithBackground().x();
        normalizedX = normalizedX * scale * 2.0f - translate;
        scale = translate = this->scaleWithBackground().z();
        normalizedZ = normalizedZ * -scale * 2.0f + translate;
    }
    scale = translate = this->scale().y();
    float yval = (yReversed ? axisYValue->max() - data.y() : data.y());
    normalizedY = axisYValue->positionAt(yval) * scale * 2.0f - translate;
    return QVector3D(normalizedX, normalizedY, normalizedZ);
}

void QQuickGraphsSurface::toggleSliceGraph()
{
    if (m_selectionDirty)
        QQuickGraphsItem::toggleSliceGraph();

    setSelectedPointChanged(true);

    if (!sliceView()->isVisible())
        return;

    QPointF worldCoord;
    for (auto model : std::as_const(m_model)) {
        if (model->picked) {
            QPoint coords = model->selectedVertex.coord;
            worldCoord = mapCoordsToWorldSpace(model, coords);
        }
    }

    for (auto model : std::as_const(m_model)) {
        bool visible = model->series->isVisible();

        model->sliceModel->setVisible(visible);
        model->sliceGridModel->setVisible(visible);

        if (!selectionMode().testFlag(QtGraphs3D::SelectionFlag::MultiSeries) && !model->picked) {
            model->sliceModel->setVisible(false);
            model->sliceGridModel->setVisible(false);
            continue;
        } else {
            model->sliceGridModel->setVisible(
                model->series->drawMode().testFlag(QSurface3DSeries::DrawWireframe));
            if (model->series->drawMode().testFlag(QSurface3DSeries::DrawSurface))
                model->sliceModel->setLocalOpacity(1.f);
            else
                model->sliceModel->setLocalOpacity(.0f);
        }

        QList<SurfaceVertex> selectedSeries;

        QRect sampleSpace = model->sampleSpace;
        int rowStart = sampleSpace.top();
        int columnStart = sampleSpace.left();
        int rowEnd = sampleSpace.bottom() + 1;
        int columnEnd = sampleSpace.right() + 1;
        int rowCount = sampleSpace.height();
        int columnCount = sampleSpace.width();

        QPoint coord;
        if (model->picked)
            coord = model->selectedVertex.coord;
        else
            coord = mapCoordsToSampleSpace(model, worldCoord);

        int indexCount = 0;
        const QSurfaceDataArray &array = model->series->dataArray();
        const qsizetype maxRow = array.size() - 1;
        const qsizetype maxColumn = array.at(0).size() - 1;
        const bool ascendingX = array.at(0).at(0).x() < array.at(0).at(maxColumn).x();
        const bool ascendingZ = array.at(0).at(0).z() < array.at(maxRow).at(0).z();
        if (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Row) && coord.y() != -1) {
            selectedSeries.reserve(columnCount * 2);
            QList<SurfaceVertex> list;
            QSurfaceDataRow row = array.at(coord.y());
            for (int i = columnStart; i < columnEnd; i++) {
                int index = ascendingX ? i : columnEnd - i + columnStart - 1;
                QVector3D pos = getNormalizedVertex(row.at(index), false, false);
                SurfaceVertex vertex;
                vertex.position = pos;
                vertex.position.setY(vertex.position.y() - .025f);
                vertex.position.setZ(.0f);
                selectedSeries.append(vertex);
                vertex.position.setY(vertex.position.y() + .05f);
                list.append(vertex);
            }
            selectedSeries.append(list);
            indexCount = columnCount - 1;
        }

        if (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Column) && coord.x() != -1) {
            selectedSeries.reserve(rowCount * 2);
            QList<SurfaceVertex> list;
            for (int i = rowStart; i < rowEnd; i++) {
                int index = ascendingZ ? i : rowEnd - i + rowStart - 1;
                QVector3D pos = getNormalizedVertex(array.at(index).at(coord.x()), false, false);
                SurfaceVertex vertex;
                vertex.position = pos;
                vertex.position.setX(-vertex.position.z());
                vertex.position.setY(vertex.position.y() - .025f);
                vertex.position.setZ(0);
                selectedSeries.append(vertex);
                vertex.position.setY(vertex.position.y() + .05f);
                list.append(vertex);
            }
            selectedSeries.append(list);
            indexCount = rowCount - 1;

            QQmlListReference materialRef(model->sliceModel, "materials");
            auto material = materialRef.at(0);
            material->setProperty("isColumn", true);
        }

        QList<quint32> indices;
        indices.reserve(indexCount * 6);
        for (int i = 0; i < indexCount; i++) {
            indices.push_back(i + 1);
            indices.push_back(i + indexCount + 1);
            indices.push_back(i);
            indices.push_back(i + indexCount + 2);
            indices.push_back(i + indexCount + 1);
            indices.push_back(i + 1);
        }

        auto geometry = model->sliceModel->geometry();
        QByteArray vertexBuffer(reinterpret_cast<char *>(selectedSeries.data()),
                                selectedSeries.size() * sizeof(SurfaceVertex));
        geometry->setVertexData(vertexBuffer);
        QByteArray indexBuffer(reinterpret_cast<char *>(indices.data()),
                               indices.size() * sizeof(quint32));
        geometry->setIndexData(indexBuffer);
        geometry->update();

        geometry = model->sliceGridModel->geometry();
        geometry->setVertexData(vertexBuffer);

        QList<quint32> gridIndices;
        gridIndices.reserve(indexCount * 4);
        for (int i = 0; i < indexCount; i++) {
            gridIndices.push_back(i);
            gridIndices.push_back(i + indexCount + 1);

            gridIndices.push_back(i);
            gridIndices.push_back(i + 1);
        }
        QByteArray gridIndexBuffer(reinterpret_cast<char *>(gridIndices.data()),
                                   gridIndices.size() * sizeof(quint32));
        geometry->setIndexData(gridIndexBuffer);
        geometry->update();

        QQmlListReference gridMaterialRef(model->sliceGridModel, "materials");
        auto gridMaterial = static_cast<QQuick3DPrincipledMaterial *>(gridMaterialRef.at(0));
        QColor gridColor = model->series->wireframeColor();
        gridMaterial->setBaseColor(gridColor);

        updateSelectedPoint();
    }
}

QPointF QQuickGraphsSurface::mapCoordsToWorldSpace(SurfaceModel *model, QPointF coords)
{
    const QSurfaceDataArray &array = model->series->dataArray();
    QSurfaceDataItem item = array.at(coords.y()).at(coords.x());
    return QPointF(item.x(), item.z());
}

QPoint QQuickGraphsSurface::mapCoordsToSampleSpace(SurfaceModel *model, QPointF coords)
{
    const QSurfaceDataArray &array = model->series->dataArray();
    qsizetype maxRow = array.size() - 1;
    qsizetype maxCol = array.at(0).size() - 1;
    const bool ascendingX = array.at(0).at(0).x() < array.at(0).at(maxCol).x();
    const bool ascendingZ = array.at(0).at(0).z() < array.at(maxRow).at(0).z();
    qsizetype botX = ascendingX ? 0 : maxCol;
    qsizetype botZ = ascendingZ ? 0 : maxRow;
    qsizetype topX = ascendingX ? maxCol : 0;
    qsizetype topZ = ascendingZ ? maxRow : 0;

    QPoint point(-1, -1);

    QSurfaceDataItem bottomLeft = array.at(botZ).at(botX);
    QSurfaceDataItem topRight = array.at(topZ).at(topX);

    QPointF pointBL(bottomLeft.x(), bottomLeft.z());
    QPointF pointTR(topRight.x(), topRight.z());

    QPointF pointF = coords - pointBL;
    QPointF span = pointTR - pointBL;
    QPointF step = QPointF(span.x() / float(maxCol), span.y() / float(maxRow));
    QPoint sample = QPoint((pointF.x() + (step.x() / 2.0)) / step.x(),
                           (pointF.y() + (step.y() / 2.0)) / step.y());

    if (bottomLeft.x() <= coords.x() && topRight.x() >= coords.x())
        point.setX(ascendingX ? sample.x() : int(maxCol) - sample.x());

    if (bottomLeft.z() <= coords.y() && topRight.z() >= coords.y())
        point.setY(ascendingZ ? sample.y() : int(maxRow) - sample.y());
    return point;
}

void QQuickGraphsSurface::createIndices(SurfaceModel *model, qsizetype columnCount, qsizetype rowCount)
{
    qsizetype endX = columnCount - 1;
    qsizetype endY = rowCount - 1;

    qsizetype indexCount = 6 * endX * endY;
    QList<quint32> *indices = &model->indices;

    indices->clear();
    indices->reserve(indexCount);

    qsizetype rowEnd = endY * columnCount;
    for (qsizetype row = 0; row < rowEnd; row += columnCount) {
        for (qsizetype j = 0; j < endX; j++) {
            indices->push_back(int(row + j + 1));
            indices->push_back(int(row + columnCount + j));
            indices->push_back(int(row + j));

            indices->push_back(int(row + columnCount + j + 1));
            indices->push_back(int(row + columnCount + j));
            indices->push_back(int(row + j + 1));
        }
    }
}

void QQuickGraphsSurface::createLineIndices(SurfaceModel *model, qsizetype pointCount)
{
    model->indices.clear();
    model->gridIndices.reserve(pointCount);
    for (int i = 0; i < pointCount; i++)
        model->indices.push_back(i);
}

void QQuickGraphsSurface::createGridlineIndices(SurfaceModel *model, qsizetype x, qsizetype y, qsizetype endX, qsizetype endY)
{
    qsizetype columnCount = model->columnCount;
    qsizetype rowCount = model->rowCount;

    if (endX >= columnCount)
        endX = columnCount - 1;
    if (endY >= rowCount)
        endY = rowCount - 1;
    if (x > endX)
        x = endX - 1;
    if (y > endY)
        y = endY - 1;

    qsizetype nColumns = endX - x + 1;
    qsizetype nRows = endY - y + 1;

    qsizetype gridIndexCount = 2 * nColumns * (nRows - 1) + 2 * nRows * (nColumns - 1);
    model->gridIndices.clear();
    model->gridIndices.reserve(gridIndexCount);

    for (qsizetype i = y, row = columnCount * y; i <= endY; i++, row += columnCount) {
        for (qsizetype j = x; j < endX; j++) {
            model->gridIndices.push_back(int(row + j));
            model->gridIndices.push_back(int(row + j + 1));
        }
    }
    for (qsizetype i = y, row = columnCount * y; i < endY; i++, row += columnCount) {
        for (qsizetype j = x; j <= endX; j++) {
            model->gridIndices.push_back(int(row + j));
            model->gridIndices.push_back(int(row + j + columnCount));
        }
    }
}

bool QQuickGraphsSurface::doPicking(QPointF position)
{
    if (!QQuickGraphsItem::doPicking(position))
        return false;
    Q_TRACE(QGraphs3DSurfaceDoPicking_entry, position.x(), position.y());

    SurfaceModel *pickedModel = nullptr;
    const QVector3D rayOrigin = rootNode()->mapPositionFromScene(mapTo3DScene(QVector3D(position.x(), position.y(), 0)));
    const QVector3D rayEnd = rootNode()->mapPositionFromScene(mapTo3DScene(QVector3D(position.x(), position.y(), 1)));
    const QVector3D rayDir = (rayEnd - rayOrigin).normalized();
    QVector3D pickedPos = pickSurfaces(rayOrigin, rayDir, pickedModel);

    if (!selectionMode().testFlag(QtGraphs3D::SelectionFlag::None)) {
        if (!sliceView() && selectionMode().testFlag(QtGraphs3D::SelectionFlag::Slice))
            createSliceView();

        bool inBounds = qAbs(pickedPos.y()) < scaleWithBackground().y();
        if (!pickedPos.isNull() && inBounds) {

            bool inRange = qAbs(pickedPos.x()) < scaleWithBackground().x()
                           && qAbs(pickedPos.z()) < scaleWithBackground().z();

            if (!pickedPos.isNull() && inRange) {
                for (auto model : std::as_const(m_model)) {
                    if (!model->series->isVisible()) {
                        model->picked = false;
                        continue;
                    }

                    model->picked = (model == pickedModel);

                    SurfaceVertex selectedVertex;
                    qreal min = std::numeric_limits<qreal>::max();

                    for (auto vertex : std::as_const(model->vertices)) {
                        QVector3D pos = vertex.position;
                        float dist = pickedPos.distanceToPoint(pos);
                        if (dist < min) {
                            min = dist;
                            selectedVertex = vertex;
                        }
                    }
                    model->selectedVertex = selectedVertex;
                    if (!selectedVertex.position.isNull() && model->picked) {
                        setSelectedPoint(selectedVertex.coord, model->series, true);
                        qCDebug(lcInput3D) << "pick results:"
                            << "\n instance position:" << selectedVertex.position
                            << "\n picked vertices coords:" << selectedVertex.coord
                            << "\n picked vertices values:" << model->series->dataProxy()->itemAt(selectedVertex.coord).position();
                    }
                }
            }
        } else {
            clearSelection();
            for (auto model : std::as_const(m_model))
                model->picked = false;
        }
    }
    Q_TRACE(QGraphs3DSurfaceDoPicking_exit);
    return true;
}

bool QQuickGraphsSurface::doRayPicking(QVector3D origin, QVector3D direction)
{
    if (!QQuickGraphsItem::doRayPicking(origin, direction))
        return false;

    Q_TRACE(QGraphs3DSurfaceDoRayPicking_entry, origin.x(), origin.y(), origin.z(), direction.x(),
            direction.y(), direction.z());

    SurfaceModel *pickedModel = nullptr;
    QVector3D pickedPos = pickSurfaces(origin, direction, pickedModel);

    if (!selectionMode().testFlag(QtGraphs3D::SelectionFlag::None)) {
        if (!sliceView() && selectionMode().testFlag(QtGraphs3D::SelectionFlag::Slice))
            createSliceView();

        bool inBounds = qAbs(pickedPos.y()) < scaleWithBackground().y();
        if (!pickedPos.isNull() && inBounds) {

            bool inRange = qAbs(pickedPos.x()) < scaleWithBackground().x()
                           && qAbs(pickedPos.z()) < scaleWithBackground().z();

            if (!pickedPos.isNull() && inRange) {
                for (auto model : std::as_const(m_model)) {
                    if (!model->series->isVisible()) {
                        model->picked = false;
                        continue;
                    }
                    model->picked = (model == pickedModel);

                    SurfaceVertex selectedVertex;
                    qreal min = std::numeric_limits<qreal>::max();

                    for (auto vertex : std::as_const(model->vertices)) {
                        QVector3D pos = vertex.position;
                        float dist = pickedPos.distanceToPoint(pos);
                        if (dist < min) {
                            min = dist;
                            selectedVertex = vertex;
                        }
                    }
                    model->selectedVertex = selectedVertex;
                    if (!selectedVertex.position.isNull() && model->picked) {
                        setSelectedPoint(selectedVertex.coord, model->series, true);
                        qCDebug(lcInput3D) << "pick results:"
                            << "\n instance position:" << selectedVertex.position
                            << "\n picked vertices coords:" << selectedVertex.coord
                            << "\n picked vertices values:" << model->series->dataProxy()->itemAt(selectedVertex.coord).position();
                    }
                }
            }
        } else {
            clearSelection();
            for (auto model : std::as_const(m_model))
                model->picked = false;
        }
    }
    Q_TRACE(QGraphs3DSurfaceDoRayPicking_exit);
    return true;
}

QVector3D QQuickGraphsSurface::pickSurfaces(QVector3D rayOrigin,
                                            QVector3D rayDir,
                                            SurfaceModel *&pickedModel)
{
    qreal closestDistance = std::numeric_limits<qreal>::max();
    qreal closestLineDistance = std::numeric_limits<qreal>::max();

    QVector3D pickedPosition;

    QVector3D pickedLinePosition;
    SurfaceModel *pickedLineModel = nullptr;

    for (auto model : m_model) {
        qsizetype columnCount = model->sampleSpace.width();
        qsizetype rowCount = model->sampleSpace.height();
        bool lineData = (rowCount == 1 || columnCount == 1);

        if (!model->series->isVisible())
            continue;

        if (lineData) {
            QVector3D linePos;
            for (auto vertex : model->vertices) {
                QVector3D v = vertex.position - rayOrigin;
                qreal t = QVector3D::dotProduct(v, rayDir);
                QVector3D closestRayPos = rayOrigin + rayDir * t;
                qreal distance = closestRayPos.distanceToPoint(vertex.position);
                if (distance < closestLineDistance) {
                    linePos = vertex.position;
                    closestLineDistance = distance;
                }
            }
            // check distance against reasonable pick radius
            /*After testing, 0.05 gives a good error margin for
            picking, without casuing false positives
            from clicks for surrounding points */

            qreal pickRadius = 0.05f;
            if (closestLineDistance < pickRadius) {
                pickedLinePosition = linePos;
                pickedLineModel = model;
            }
            continue;
        }

        if (!intersectWithAABB(model->boundsMin, model->boundsMax, rayOrigin, rayDir))
            continue;

        qsizetype endX = columnCount - 1;
        qsizetype endY = rowCount - 1;

        qsizetype rowEnd = endY * columnCount;
        for (qsizetype row = 0; row < rowEnd; row += columnCount) {
            for (qsizetype j = 0; j < endX; j++) {
                std::array<QVector3D, 3> triangle;
                triangle[0] = model->heights.at(int(row + j + 1)).toVector3D();
                triangle[1] = model->heights.at(int(row + columnCount + j)).toVector3D();
                triangle[2] = model->heights.at(int(row + j)).toVector3D();
                QVector3D intersection = triangleIntersection(rayOrigin, rayDir, triangle);
                if (!intersection.isNull()) {
                    qreal distance = rayOrigin.distanceToPoint(intersection);
                    if (distance < closestDistance) {
                        pickedPosition = intersection;
                        pickedModel = model;
                        closestDistance = distance;
                    }
                }

                triangle[0] = model->heights.at(int(row + columnCount + j + 1)).toVector3D();
                triangle[1] = model->heights.at(int(row + columnCount + j)).toVector3D();
                triangle[2] = model->heights.at(int(row + j + 1)).toVector3D();
                intersection = triangleIntersection(rayOrigin, rayDir, triangle);
                if (!intersection.isNull()) {
                    qreal distance = rayOrigin.distanceToPoint(intersection);
                    if (distance < closestDistance) {
                        pickedPosition = intersection;
                        pickedModel = model;
                        closestDistance = distance;
                    }
                }
            }
        }
    }
    // compare line and surface positions
    if (!pickedLinePosition.isNull() && pickedLineModel) {
        qreal lineDistance = rayOrigin.distanceToPoint(pickedLinePosition);
        if (lineDistance < closestDistance) {
            pickedPosition = pickedLinePosition;
            pickedModel = pickedLineModel;
        }
    }
    return pickedPosition;
}

// Möller-Trumbore ray-triangle intersection
QVector3D QQuickGraphsSurface::triangleIntersection(QVector3D origin,
                                                    QVector3D dir,
                                                    const std::array<QVector3D, 3> &triangle)
{
    constexpr qreal epsilon = std::numeric_limits<float>::epsilon();

    const QVector3D edge1 = triangle[1] - triangle[0];
    const QVector3D edge2 = triangle[2] - triangle[0];

    // Compute the vector P as the cross product of the ray direction and edge2
    const QVector3D P = QVector3D::crossProduct(dir, edge2);

    // Compute the determinant
    const float determinant = QVector3D::dotProduct(edge1, P);

    QVector3D Q;
    qreal u;
    qreal v;

    if (determinant > epsilon) {
        // Compute the vector T from the ray origin to the first vertex of the triangle
        const QVector3D T = origin - triangle[0];

        // Calculate coordinate u and test bounds
        u = QVector3D::dotProduct(T, P);
        if (u < 0.0f || u > determinant)
            return QVector3D();

        // Compute the vector Q as the cross product of vector T and edge1
        Q = QVector3D::crossProduct(T, edge1);

        // Calculate coordinate v and test bounds
        v = QVector3D::dotProduct(dir, Q);
        if (v < 0.0f || ((u + v) > determinant))
            return QVector3D();
    } else if (determinant < -epsilon) { //Backfaces
        // Compute the vector T from the ray origin to the first vertex of the triangle
        const QVector3D T = origin - triangle[0];

        // Calculate coordinate u and test bounds
        u = QVector3D::dotProduct(T, P);
        if (u > 0.0f || u < determinant)
            return QVector3D();

        // Compute the vector Q as the cross product of vector T and edge1
        Q = QVector3D::crossProduct(T, edge1);

        // Calculate coordinate v and test bounds
        v = QVector3D::dotProduct(dir, Q);
        if (v > 0.0f || ((u + v) < determinant))
            return QVector3D();
    } else {
        // Ray is parallel to the plane of the triangle
        return QVector3D();
    }

    const float invDeterminant = 1.0f / determinant;

    // Calculate the value of t, the parameter of the intersection point along the ray
    const float t = QVector3D::dotProduct(edge2, Q) * invDeterminant;

    if (t > epsilon)
        return origin + dir * t;

    return QVector3D();
}

bool QQuickGraphsSurface::intersectWithAABB(QVector3D boundsMin,
                                            QVector3D boundsMax,
                                            QVector3D rayOrigin,
                                            QVector3D rayDir)
{
    float tmax = std::numeric_limits<float>::max();
    float tmin = std::numeric_limits<float>::min();
    float origin;

    for (int axis = 0; axis < 3; ++axis) {
        origin = rayOrigin[axis];
        const bool zeroDir = rayDir[axis] == 0;

        // Pickray is roughly parallel to the plane of the slab
        // so, if the origin is not in the range, we have no intersection
        if (zeroDir && (origin < boundsMin[axis] || origin > boundsMax[axis]))
            return false;

        if (!zeroDir) {
            float t1 = (boundsMin[axis] - origin) / rayDir[axis];
            float t2 = (boundsMax[axis] - origin) / rayDir[axis];

            tmin = qMax(tmin, qMin(t1, t2));
            tmax = qMin(tmax, qMax(t1, t2));
        }
    }

    return tmin < tmax;
}

void QQuickGraphsSurface::updateSelectedPoint()
{
    bool labelVisible = false;

    auto list = surfaceSeriesList();
    for (auto series : std::as_const(list)) {
        // If the pointer and its instancing do not exist yet (as will happen in widget case),
        // we must create them
        if (!m_selectionPointers.value(series))
            changePointerMeshTypeForSeries(series->mesh(), series);

        if (sliceView() && !m_sliceSelectionPointers.value(series))
            changeSlicePointerMeshTypeForSeries(series->mesh(), series);
    }

    for (auto i = m_selectionPointers.keyValueBegin(); i != m_selectionPointers.keyValueEnd(); i++)
        i->second->setVisible(false);

    if (sliceView() && sliceView()->isVisible()) {
        for (auto i = m_sliceSelectionPointers.keyValueBegin();
             i != m_sliceSelectionPointers.keyValueEnd();
             i++) {
            i->second->setVisible(false);
        }
    }

    QPointF worldCoord;
    for (auto model : std::as_const(m_model)) {
        if (model->picked) {
            QPoint coords = model->selectedVertex.coord;
            worldCoord = mapCoordsToWorldSpace(model, coords);
        }
    }
    for (auto model : std::as_const(m_model)) {
        Q_TRACE(QGraphs3DSurfacePointSelectionUpdate_entry,static_cast<void *>( model));

        if ((!selectionMode().testFlag(QtGraphs3D::SelectionFlag::MultiSeries) && !model->picked)
            || model->selectedVertex.position.isNull()) {
            Q_TRACE(QGraphs3DSurfacePointSelectionUpdate_exit);
            continue;
        }
        QPoint selectedCoord;
        if (model->picked)
            selectedCoord = model->selectedVertex.coord;
        else
            selectedCoord = mapCoordsToSampleSpace(model, worldCoord);
        if (selectedCoord.x() == -1 || selectedCoord.y() == -1) {
            Q_TRACE(QGraphs3DSurfacePointSelectionUpdate_exit);
            continue;
        }

        const QSurfaceDataItem &dataPos
            = model->series->dataArray().at(selectedCoord.y()).at(selectedCoord.x());
        QVector3D pos = getNormalizedVertex(dataPos, isPolar(), false, model->series);

        SurfaceVertex selectedVertex;
        selectedVertex.position = pos;
        selectedVertex.coord = model->selectedVertex.coord;
        if (model->series->isVisible() && !selectedVertex.position.isNull()
            && selectionMode().testFlag(QtGraphs3D::SelectionFlag::Item)) {
            m_selectionPointers.value(model->series)->setPosition(selectedVertex.position);
            m_selectionPointers.value(model->series)->setVisible(true);
            QVector3D slicePosition = getNormalizedVertex(dataPos, false, false, model->series);
            if (sliceView() && sliceView()->isVisible()) {
                if (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Column))
                    slicePosition.setX(-slicePosition.z());
                slicePosition.setZ(.0f);
                m_sliceSelectionPointers.value(model->series)->setPosition(slicePosition);
                m_sliceSelectionPointers.value(model->series)->setVisible(true);
            }
            if (model->picked) {
                QVector3D labelPosition = selectedVertex.position;
                QString label = model->series->itemLabel();
                setSelectedPoint(selectedVertex.coord, model->series, false);

                updateItemLabel(labelPosition);
                itemLabel()->setProperty("labelText", label);
                if (!label.compare(QString(hiddenLabelTag)))
                    itemLabel()->setVisible(false);
                labelVisible = model->series->isItemLabelVisible();
                if (sliceView() && sliceView()->isVisible())
                    updateSliceItemLabel(label, slicePosition);
            }
        }
        Q_TRACE(QGraphs3DSurfacePointSelectionUpdate_exit);
    }
    setItemSelected(m_selectedSeries != nullptr);
    itemLabel()->setVisible(labelVisible);
    if (sliceView() && sliceView()->isVisible())
        sliceItemLabel()->setVisible(labelVisible);
}

void QQuickGraphsSurface::addModel(QSurface3DSeries *series)
{
    Q_TRACE_SCOPE(QGraphs3DSurfaceAddModel, series);
    auto parent = graphNode();
    bool visible = series->isVisible();

    auto model = new QQuick3DModel();
    model->setParent(parent);
    model->setParentItem(parent);
    model->setObjectName(QStringLiteral("SurfaceModel"));
    model->setVisible(visible);
    if (selectionMode().testFlag(QtGraphs3D::SelectionFlag::None))
        model->setPickable(false);
    else
        model->setPickable(true);

    auto geometry = new QQuick3DGeometry();
    geometry->setParent(model);
    geometry->setStride(sizeof(SurfaceVertex));
    geometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
    geometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                           0,
                           QQuick3DGeometry::Attribute::F32Type);
    geometry->addAttribute(QQuick3DGeometry::Attribute::TexCoord0Semantic,
                           sizeof(QVector3D),
                           QQuick3DGeometry::Attribute::F32Type);
    geometry->addAttribute(QQuick3DGeometry::Attribute::IndexSemantic,
                           0,
                           QQuick3DGeometry::Attribute::U32Type);
    model->setGeometry(geometry);

    model->setCastsShadows(false); //Disable shadows as they render incorrectly

    QQuick3DTexture *texture = new QQuick3DTexture();
    texture->setHorizontalTiling(QQuick3DTexture::ClampToEdge);
    texture->setVerticalTiling(QQuick3DTexture::ClampToEdge);
    QQuickGraphsTextureData *textureData = new QQuickGraphsTextureData();
    textureData->setParent(texture);
    textureData->setParentItem(texture);
    texture->setTextureData(textureData);

    QQuick3DTexture *gradientTexture = new QQuick3DTexture();
    gradientTexture->setHorizontalTiling(QQuick3DTexture::ClampToEdge);
    gradientTexture->setVerticalTiling(QQuick3DTexture::ClampToEdge);
    QQuickGraphsTextureData *gradientTextureData = new QQuickGraphsTextureData();
    textureData->setParent(gradientTexture);
    textureData->setParentItem(gradientTexture);
    gradientTexture->setTextureData(gradientTextureData);

    QQmlListReference materialRef(model, "materials");

    QQuick3DCustomMaterial *customMaterial = createQmlCustomMaterial(
        QStringLiteral(":/materials/SurfaceMaterial"));

    customMaterial->setParent(model);
    customMaterial->setParentItem(model);
    customMaterial->setCullMode(QQuick3DMaterial::NoCulling);
    QVariant textureInputAsVariant = customMaterial->property("custex");
    QQuick3DShaderUtilsTextureInput *textureInput = textureInputAsVariant
                                                        .value<QQuick3DShaderUtilsTextureInput *>();
    textureInput->setTexture(texture);

    texture->setParent(customMaterial);

    materialRef.append(customMaterial);

    auto gridModel = new QQuick3DModel();
    gridModel->setParent(parent);
    gridModel->setParentItem(parent);
    gridModel->setObjectName(QStringLiteral("SurfaceModel"));
    gridModel->setVisible(visible);
    gridModel->setDepthBias(1.0f);
    auto gridGeometry = new QQuick3DGeometry();
    gridGeometry->setParent(this);
    gridGeometry->setStride(sizeof(SurfaceVertex));
    gridGeometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Lines);
    gridGeometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                               0,
                               QQuick3DGeometry::Attribute::F32Type);
    gridGeometry->addAttribute(QQuick3DGeometry::Attribute::TexCoord0Semantic,
                               sizeof(QVector3D),
                               QQuick3DGeometry::Attribute::F32Type);
    gridGeometry->addAttribute(QQuick3DGeometry::Attribute::IndexSemantic,
                               0,
                               QQuick3DGeometry::Attribute::U32Type);
    gridModel->setGeometry(gridGeometry);
    gridModel->setCastsShadows(false);
    QQmlListReference gridMaterialRef(gridModel, "materials");
    auto gridMaterial = createQmlCustomMaterial(QStringLiteral(":/materials/GridSurfaceMaterial"));
    gridMaterial->setParent(gridModel);
    gridMaterial->setParentItem(gridModel);
    gridMaterialRef.append(gridMaterial);

    SurfaceModel *surfaceModel = new SurfaceModel();
    surfaceModel->model = model;
    surfaceModel->gridModel = gridModel;
    surfaceModel->series = series;
    surfaceModel->texture = texture;
    surfaceModel->gradientTexture = gradientTexture;
    surfaceModel->customMaterial = customMaterial;

    m_model.push_back(surfaceModel);

    connect(series,
            &QSurface3DSeries::shadingChanged,
            this,
            &QQuickGraphsSurface::handleShadingChanged);
    connect(series,
            &QSurface3DSeries::wireframeColorChanged,
            this,
            &QQuickGraphsSurface::handleWireframeColorChanged);
    connect(series,
            &QSurface3DSeries::userDefinedMeshChanged,
            this,
            &QQuickGraphsSurface::handlePointerChanged);
    connect(series,
            &QSurface3DSeries::meshChanged,
            this,
            &QQuickGraphsSurface::handleMeshTypeChanged);
    if (sliceView())
        addSliceModel(surfaceModel);

    addFillModel(surfaceModel);
}

void QQuickGraphsSurface::addFillModel(SurfaceModel *model)
{
    Q_TRACE_SCOPE(QGraphs3DSurfaceAddFillModel);
    auto parent = graphNode();

    auto fillModel = new QQuick3DModel();
    fillModel->setParent(parent);
    fillModel->setParentItem(model->model);
    fillModel->setObjectName(QStringLiteral("FillModel"));
    fillModel->setVisible(true);
    fillModel->setPickable(false);

    auto geometry = new QQuick3DGeometry();
    geometry->setParent(fillModel);
    geometry->setStride(sizeof(SurfaceVertex));
    geometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
    geometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                           0,
                           QQuick3DGeometry::Attribute::F32Type);
    geometry->addAttribute(QQuick3DGeometry::Attribute::TexCoord0Semantic,
                           sizeof(QVector3D),
                           QQuick3DGeometry::Attribute::F32Type);
    geometry->addAttribute(QQuick3DGeometry::Attribute::IndexSemantic,
                           0,
                           QQuick3DGeometry::Attribute::U32Type);
    fillModel->setGeometry(geometry);

    fillModel->setCastsShadows(false); //Disable shadows as they render incorrectly

    QQmlListReference materialRef(fillModel, "materials");

    auto *customMaterial = model->customMaterial;
    materialRef.append(customMaterial);

    model->fillModel = fillModel;
    m_fillDirty[model] = false;
}

void QQuickGraphsSurface::createSliceView()
{
    setSliceOrthoProjection(true);
    QQuickGraphsItem::createSliceView();
    Q_TRACE_SCOPE(QGraphs3DSurfaceCreateSliceView);

    for (auto surfaceModel : std::as_const(m_model)) {
        addSliceModel(surfaceModel);
        changeSlicePointerMeshTypeForSeries(surfaceModel->series->mesh(), surfaceModel->series);
    }
}

QQuick3DViewport *
QQuickGraphsSurface::createOffscreenSliceView(int index, int requestedIndex,
                                              QtGraphs3D::SliceCaptureType sliceType)
{
    QQuick3DViewport *sliceView = QQuickGraphsItem::createOffscreenSliceView(sliceType);
    Q_TRACE_SCOPE(QGraphs3DSurfaceCreateOffscreenSliceView, index, requestedIndex,
                  static_cast<int>(sliceType));

    const bool isRow = (sliceType == QtGraphs3D::SliceCaptureType::RowImage);
    const bool isColumn = (sliceType == QtGraphs3D::SliceCaptureType::ColumnImage);

    int modelIndex = 0;
    for (const auto &model : std::as_const(m_model)) {
        if (index > 0 && modelIndex++ != index)
            continue;

        QRect sampleSpace = model->sampleSpace;
        int rowStart = sampleSpace.top();
        int columnStart = sampleSpace.left();
        int rowEnd = sampleSpace.bottom() + 1;
        int columnEnd = sampleSpace.right() + 1;
        int rowCount = sampleSpace.height();
        int columnCount = sampleSpace.width();

        QList<SurfaceVertex> selectedSeries;
        int indexCount = 0;
        const QSurfaceDataArray &array = model->series->dataArray();
        const qsizetype maxRow = array.size() - 1;
        const qsizetype maxColumn = array.at(0).size() - 1;
        const bool ascendingX = array.at(0).at(0).x() < array.at(0).at(maxColumn).x();
        const bool ascendingZ = array.at(0).at(0).z() < array.at(maxRow).at(0).z();

        if (requestedIndex < 0 || requestedIndex >= maxRow || requestedIndex >= maxColumn) {
            qCWarning(lcGraphsSurface3D, "The index is out of range. The render stops.");
            sliceView->setVisible(false);
            sliceView->deleteLater();
            return nullptr;
        }

        if (isRow && requestedIndex != -1) {
            selectedSeries.reserve(columnCount * 2);
            QList<SurfaceVertex> list;
            QSurfaceDataRow row = array.at(requestedIndex);
            for (int i = columnStart; i < columnEnd; i++) {
                int index = ascendingX ? i : columnEnd - i + columnStart - 1;
                QVector3D pos = getNormalizedVertex(row.at(index), false, false);
                SurfaceVertex vertex;
                vertex.position = pos;
                vertex.position.setY(vertex.position.y() - .025f);
                vertex.position.setZ(.0f);
                selectedSeries.append(vertex);
                vertex.position.setY(vertex.position.y() + .05f);
                list.append(vertex);
            }
            selectedSeries.append(list);
            indexCount = columnCount - 1;
        }

        if (isColumn && requestedIndex != -1) {
            selectedSeries.reserve(rowCount * 2);
            QList<SurfaceVertex> list;
            for (int i = rowStart; i < rowEnd; i++) {
                int index = ascendingZ ? i : rowEnd - i + rowStart - 1;
                QVector3D pos =
                        getNormalizedVertex(array.at(index).at(requestedIndex), false, false);
                SurfaceVertex vertex;
                vertex.position = pos;
                vertex.position.setX(-vertex.position.z());
                vertex.position.setY(vertex.position.y() - .025f);
                vertex.position.setZ(0);
                selectedSeries.append(vertex);
                vertex.position.setY(vertex.position.y() + .05f);
                list.append(vertex);
            }
            selectedSeries.append(list);
            indexCount = rowCount - 1;
        }

        QList<quint32> indices;
        indices.reserve(indexCount * 6);
        for (int i = 0; i < indexCount; i++) {
            indices.push_back(i + 1);
            indices.push_back(i + indexCount + 1);
            indices.push_back(i);
            indices.push_back(i + indexCount + 2);
            indices.push_back(i + indexCount + 1);
            indices.push_back(i + 1);
        }

        auto surfaceModel = new QQuick3DModel();
        surfaceModel->setParent(sliceView->scene());
        surfaceModel->setParentItem(sliceView->scene());

        auto geometry = new QQuick3DGeometry();
        geometry->setParent(surfaceModel);
        geometry->setParentItem(surfaceModel);
        geometry->setStride(sizeof(SurfaceVertex));
        geometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
        geometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                               QQuick3DGeometry::Attribute::F32Type);
        geometry->addAttribute(QQuick3DGeometry::Attribute::TexCoord0Semantic, sizeof(QVector3D),
                               QQuick3DGeometry::Attribute::F32Type);
        geometry->addAttribute(QQuick3DGeometry::Attribute::IndexSemantic, 0,
                               QQuick3DGeometry::Attribute::U32Type);
        QByteArray vertexBuffer(reinterpret_cast<char *>(selectedSeries.data()),
                                selectedSeries.size() * sizeof(SurfaceVertex));
        geometry->setVertexData(vertexBuffer);
        QByteArray indexBuffer(reinterpret_cast<char *>(indices.data()),
                               indices.size() * sizeof(quint32));
        geometry->setIndexData(indexBuffer);
        surfaceModel->setGeometry(geometry);

        QQmlListReference materialRef(surfaceModel, "materials");
        auto material = createQmlCustomMaterial(QStringLiteral(":/materials/SurfaceSliceMaterial"));
        material->setCullMode(QQuick3DMaterial::NoCulling);
        material->setProperty("isColumn", isColumn);
        QVariant textureInputAsVariant = material->property("custex");
        QQuick3DShaderUtilsTextureInput *textureInput =
                textureInputAsVariant.value<QQuick3DShaderUtilsTextureInput *>();
        QQuick3DTexture *texture = model->texture;
        textureInput->setTexture(texture);
        materialRef.append(material);

        if (model->series->drawMode().testFlag(QSurface3DSeries::DrawSurface))
            surfaceModel->setLocalOpacity(1.f);
        else
            surfaceModel->setLocalOpacity(.0f);

        if (model->series->drawMode().testFlag(QSurface3DSeries::DrawWireframe)) {
            QList<quint32> gridIndices;
            gridIndices.reserve(indexCount * 4);
            for (int i = 0; i < indexCount; i++) {
                gridIndices.push_back(i);
                gridIndices.push_back(i + indexCount + 1);

                gridIndices.push_back(i);
                gridIndices.push_back(i + 1);
            }
            QQuick3DModel *gridModel = new QQuick3DModel();
            gridModel->setParent(sliceView->scene());
            gridModel->setParentItem(sliceView->scene());
            gridModel->setDepthBias(1.0f);
            QQuick3DGeometry *gridGeometry = new QQuick3DGeometry();
            gridGeometry->setParent(gridModel);
            gridGeometry->setStride(sizeof(SurfaceVertex));
            gridGeometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Lines);
            gridGeometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                                       QQuick3DGeometry::Attribute::F32Type);
            gridGeometry->addAttribute(QQuick3DGeometry::Attribute::IndexSemantic, 0,
                                       QQuick3DGeometry::Attribute::U32Type);
            QByteArray gridIndexBuffer(reinterpret_cast<char *>(gridIndices.data()),
                                       gridIndices.size() * sizeof(quint32));
            gridGeometry->setVertexData(vertexBuffer);
            gridGeometry->setIndexData(gridIndexBuffer);
            gridGeometry->update();
            gridModel->setGeometry(gridGeometry);
            QQmlListReference gridMaterialRef(gridModel, "materials");
            QQuick3DPrincipledMaterial *gridMaterial = new QQuick3DPrincipledMaterial();
            gridMaterial->setParent(gridModel);
            gridMaterial->setLighting(QQuick3DPrincipledMaterial::NoLighting);
            gridMaterial->setParent(gridModel);
            QColor gridColor = model->series->wireframeColor();
            gridMaterial->setBaseColor(gridColor);
            gridMaterialRef.append(gridMaterial);
        }
    }

    return sliceView;
}

void QQuickGraphsSurface::renderSliceToImage(int index,
                                             int requestedIndex,
                                             QtGraphs3D::SliceCaptureType sliceType)
{
    QQuick3DViewport *sliceView = createOffscreenSliceView(index, requestedIndex, sliceType);

    if (!m_grabresult)
        m_grabresult = new QImage();

    if (sliceView) {
        QSharedPointer<QQuickItemGrabResult> grabbed = sliceView->grabToImage();
        connect(grabbed.data(), &QQuickItemGrabResult::ready, this, [&, grabbed, sliceView]() {
            sliceView->setVisible(false);
            sliceView->deleteLater();
            *m_grabresult = grabbed.data()->image();
            emit sliceImageChanged(*m_grabresult);
        });
    }
}

void QQuickGraphsSurface::renderSliceToImage(int index, int requestedIndex,
                                             QtGraphs3D::SliceCaptureType sliceType,
                                             const QUrl &filePath)
{
    QQuick3DViewport *sliceView = createOffscreenSliceView(index, requestedIndex, sliceType);

    if (!sliceView)
        return;

    if (filePath.isEmpty()) {
        qCWarning(lcGraphsSurface3D, "Save path is not defined.");
        sliceView->setVisible(false);
        sliceView->deleteLater();
        return;
    }

    QSharedPointer<QQuickItemGrabResult> grabbed = sliceView->grabToImage();
    connect(grabbed.data(), &QQuickItemGrabResult::ready, this, [grabbed, sliceView, filePath]() {
        if (!grabbed.data()->saveToFile(filePath))
            qCWarning(lcGraphsSurface3D, "Saving requested slice view to image failed");
        sliceView->setVisible(false);
        sliceView->deleteLater();
    });
}

void QQuickGraphsSurface::updateSliceItemLabel(const QString &label, QVector3D position)
{
    QQuickGraphsItem::updateSliceItemLabel(label, position);

    QFontMetrics fm(theme()->labelFont());
    float textPadding = 12.0f;
    float labelHeight = fm.height() + textPadding;
    float labelWidth = fm.horizontalAdvance(label) + textPadding;
    sliceItemLabel()->setProperty("labelWidth", labelWidth);
    sliceItemLabel()->setProperty("labelHeight", labelHeight);
    QVector3D labelPosition = position;
    labelPosition.setZ(.1f);
    labelPosition.setY(position.y() + .05f);
    sliceItemLabel()->setPosition(labelPosition);
    sliceItemLabel()->setProperty("labelText", label);
    if (!label.compare(QString(hiddenLabelTag)))
        sliceItemLabel()->setVisible(false);
}

void QQuickGraphsSurface::updateSelectionMode(QtGraphs3D::SelectionFlags mode)
{
    checkSliceEnabled();
    bool validSlice = mode.testFlag(QtGraphs3D::SelectionFlag::Slice)
                      && m_selectedPoint != invalidSelectionPosition();
    if (sliceView() && sliceView()->isVisible()) {
        if (validSlice) {
            toggleSliceGraph();
        } else {
            m_selectionDirty = true;
            setSliceActivatedChanged(true);
        }
    } else if (validSlice) {
        m_selectionDirty = true;
        setSliceActivatedChanged(true);
    }

    setSeriesVisualsDirty(true);
    itemLabel()->setVisible(false);
    if (sliceView() && sliceView()->isVisible())
        sliceItemLabel()->setVisible(false);
}

void QQuickGraphsSurface::addSliceModel(SurfaceModel *model)
{
    Q_TRACE_SCOPE(QGraphs3DSurfaceAddSliceModel);
    QQuick3DViewport *sliceParent = nullptr;

    sliceParent = sliceView();

    auto surfaceModel = new QQuick3DModel();
    surfaceModel->setParent(sliceParent->scene());
    surfaceModel->setParentItem(sliceParent->scene());
    surfaceModel->setVisible(model->series->isVisible());

    auto geometry = new QQuick3DGeometry();
    geometry->setParent(surfaceModel);
    geometry->setParentItem(surfaceModel);
    geometry->setStride(sizeof(SurfaceVertex));
    geometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
    geometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                           0,
                           QQuick3DGeometry::Attribute::F32Type);
    geometry->addAttribute(QQuick3DGeometry::Attribute::TexCoord0Semantic,
                           sizeof(QVector3D),
                           QQuick3DGeometry::Attribute::F32Type);
    geometry->addAttribute(QQuick3DGeometry::Attribute::IndexSemantic,
                           0,
                           QQuick3DGeometry::Attribute::U32Type);
    surfaceModel->setGeometry(geometry);

    QQmlListReference materialRef(surfaceModel, "materials");
    auto material = createQmlCustomMaterial(QStringLiteral(":/materials/SurfaceSliceMaterial"));
    material->setCullMode(QQuick3DMaterial::NoCulling);
    QVariant textureInputAsVariant = material->property("custex");
    QQuick3DShaderUtilsTextureInput *textureInput = textureInputAsVariant
                                                        .value<QQuick3DShaderUtilsTextureInput *>();
    QQuick3DTexture *texture = model->texture;
    textureInput->setTexture(texture);
    materialRef.append(material);

    model->sliceModel = surfaceModel;

    QQuick3DModel *gridModel = new QQuick3DModel();
    gridModel->setParent(sliceParent->scene());
    gridModel->setParentItem(sliceParent->scene());
    gridModel->setVisible(model->series->isVisible());
    gridModel->setDepthBias(1.0f);
    QQuick3DGeometry *gridGeometry = new QQuick3DGeometry();
    gridGeometry->setParent(gridModel);
    gridGeometry->setStride(sizeof(SurfaceVertex));
    gridGeometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Lines);
    gridGeometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                               0,
                               QQuick3DGeometry::Attribute::F32Type);
    gridGeometry->addAttribute(QQuick3DGeometry::Attribute::IndexSemantic,
                               0,
                               QQuick3DGeometry::Attribute::U32Type);
    gridModel->setGeometry(gridGeometry);
    QQmlListReference gridMaterialRef(gridModel, "materials");
    QQuick3DPrincipledMaterial *gridMaterial = new QQuick3DPrincipledMaterial();
    gridMaterial->setParent(gridModel);
    gridMaterial->setLighting(QQuick3DPrincipledMaterial::NoLighting);
    gridMaterial->setParent(gridModel);
    gridMaterialRef.append(gridMaterial);

    model->sliceGridModel = gridModel;
}

void QQuickGraphsSurface::updateSingleHighlightColor()
{
    auto list = surfaceSeriesList();
    for (auto series : std::as_const(list)) {
        QQmlListReference pMaterialRef(m_selectionPointers.value(series), "materials");
        auto pmat = qobject_cast<QQuick3DPrincipledMaterial *>(pMaterialRef.at(0));
        if (pmat)
            pmat->setBaseColor(theme()->singleHighlightColor());
        if (sliceView()) {
            QQmlListReference spMaterialRef(m_sliceSelectionPointers.value(series), "materials");
            auto spmat = qobject_cast<QQuick3DPrincipledMaterial *>(spMaterialRef.at(0));
            spmat->setBaseColor(theme()->singleHighlightColor());
        }
    }
}

void QQuickGraphsSurface::updateLightStrength()
{
    for (auto model : std::as_const(m_model)) {
        QQmlListReference materialRef(model->model, "materials");
        QQuick3DCustomMaterial *material = qobject_cast<QQuick3DCustomMaterial *>(materialRef.at(0));
        material->setProperty("specularBrightness", lightStrength() * 0.05);
    }
}

void QQuickGraphsSurface::handleThemeTypeChange()
{
    for (auto model : std::as_const(m_model))
        updateMaterial(model);
}

QT_END_NAMESPACE

#include "moc_qquickgraphssurface_p.cpp"
