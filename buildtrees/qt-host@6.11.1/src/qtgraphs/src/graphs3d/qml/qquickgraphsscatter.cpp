// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "graphs3d/utils/qgraphs3dlogging_p.h"
#include "q3dscene.h"
#include "qgraphsinputhandler_p.h"
#include "qquickgraphsscatter_p.h"
#include "qquickgraphstexturedata_p.h"
#include "qscatter3dseries.h"
#include "qscatter3dseries_p.h"
#include "qscatterdataproxy_p.h"
#include "qvalue3daxis_p.h"
#include "qgraphs3dlogging_p.h"

#include <QColor>
#include <QtQuick3D/private/qquick3dcustommaterial_p.h>
#include <QtQuick3D/private/qquick3ddirectionallight_p.h>
#include <QtQuick3D/private/qquick3dmodel_p.h>
#include <QtQuick3D/private/qquick3dperspectivecamera_p.h>
#include <QtQuick3D/private/qquick3dpointlight_p.h>
#include <QtQuick3D/private/qquick3dprincipledmaterial_p.h>
#include <QtQuick3D/private/qquick3drepeater_p.h>

#include <QtGui/qquaternion.h>

#include <qtgraphs_tracepoints_p.h>

QT_BEGIN_NAMESPACE

Q_TRACE_PREFIX(qtgraphs,
                   "QT_BEGIN_NAMESPACE" \
                   "class QQuickGraphsScatter;" \
                   "QT_END_NAMESPACE"
               )

Q_TRACE_POINT(qtgraphs, QGraphs3DScatterPointDataUpdate_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DScatterPointDataUpdate_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DScatterPointSelectionUpdate_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DScatterPointSelectionUpdate_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DScatterDoPicking_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DScatterDoPicking_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DScatterDoRayPicking_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DScatterDoRayPicking_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DScatterLegacyModelsCountUpdate_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DScatterLegacyModelsCountUpdate_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DScatterLegacyGeneratePointsForScatterModel_entry, int itemCount);
Q_TRACE_POINT(qtgraphs, QGraphs3DScatterLegacyGeneratePointsForScatterModel_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DScatterLegacyModelsPositionUpdate_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DScatterLegacyModelsPositionUpdate_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DScatterDefaultModelsPositionUpdate_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DScatterDefaultModelsPositionUpdate_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DScatterDefaultGeneratePointsForScatterModel_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DScatterDefaultGeneratePointsForScatterModel_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DScatterSplineUpdate_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DScatterSplineUpdate_exit);

static const int insertRemoveRecordReserveSize = 31;

/*!
 * \qmltype Scatter3D
 * \inherits GraphsItem3D
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \brief 3D scatter graph.
 *
 * This type enables developers to render scatter graphs in 3D with Qt Quick.
 *
 * You will need to import Qt Graphs module to use this type:
 *
 * \snippet doc_src_qmlgraphs.cpp 0
 *
 * After that you can use Scatter3D in your qml files:
 *
 * \snippet doc_src_qmlgraphs.cpp 2
 *
 * See \l{Simple Scatter Graph} for more thorough usage example.
 *
 * \sa Scatter3DSeries, Spline3DSeries, ScatterDataProxy, Bars3D, Surface3D,
 * {Qt Graphs C++ Classes for 3D}
 */

/*!
 * \qmlproperty Value3DAxis Scatter3D::axisX
 * The active x-axis.
 *
 * If an axis is not given, a temporary default axis with no labels and an
 * automatically adjusting range is created.
 * This temporary axis is destroyed if another axis is explicitly set to the
 * same orientation.
 */

/*!
 * \qmlproperty Value3DAxis Scatter3D::axisY
 * The active y-axis.
 *
 * If an axis is not given, a temporary default axis with no labels and an
 * automatically adjusting range is created.
 * This temporary axis is destroyed if another axis is explicitly set to the
 * same orientation.
 */

/*!
 * \qmlproperty Value3DAxis Scatter3D::axisZ
 * The active z-axis.
 *
 * If an axis is not given, a temporary default axis with no labels and an
 * automatically adjusting range is created.
 * This temporary axis is destroyed if another axis is explicitly set to the
 * same orientation.
 */

/*!
 * \qmlproperty Scatter3DSeries Scatter3D::selectedSeries
 * \readonly
 *
 * The selected series or null.
 */

/*!
 * \qmlproperty list<Scatter3DSeries> Scatter3D::seriesList
 * \qmldefault
 * This property holds the series of the graph.
 * By default, this property contains an empty list.
 * To set the series, either use the addSeries() method or define them as
 * children of the graph.
 */

/*!
 * \qmlmethod void Scatter3D::addSeries(Scatter3DSeries series)
 * Adds the \a series to the graph. A graph can contain multiple series, but has
 * only one set of axes. If the newly added series has specified a selected
 * item, it will be highlighted and any existing selection will be cleared. Only
 * one added series can have an active selection.
 * \sa GraphsItem3D::hasSeries()
 */

/*!
 * \qmlmethod void Scatter3D::removeSeries(Scatter3DSeries series)
 * Remove the \a series from the graph.
 * \sa GraphsItem3D::hasSeries()
 */

/*!
 * \qmlsignal Scatter3D::axisXChanged(ValueAxis3D axis)
 *
 * This signal is emitted when axisX changes to \a axis.
 */

/*!
 * \qmlsignal Scatter3D::axisYChanged(ValueAxis3D axis)
 *
 * This signal is emitted when axisY changes to \a axis.
 */

/*!
 * \qmlsignal Scatter3D::axisZChanged(ValueAxis3D axis)
 *
 * This signal is emitted when axisZ changes to \a axis.
 */

/*!
 * \qmlsignal Scatter3D::selectedSeriesChanged(Scatter3DSeries series)
 *
 * This signal is emitted when selectedSeries changes to \a series.
 */

QQuickGraphsScatter::QQuickGraphsScatter(QQuickItem *parent)
    : QQuickGraphsItem(parent)
{
    m_graphType = QAbstract3DSeries::SeriesType::Scatter;
    setAxisX(0);
    setAxisY(0);
    setAxisZ(0);
    setAcceptedMouseButtons(Qt::AllButtons);
    setFlag(ItemHasContents);
    clearSelection();
}

QQuickGraphsScatter::~QQuickGraphsScatter()
{
    for (auto &graphModel : m_scatterGraphs) {
        delete graphModel;
    }
}

void QQuickGraphsScatter::setAxisX(QValue3DAxis *axis)
{
    QQuickGraphsItem::setAxisX(axis);
}

QValue3DAxis *QQuickGraphsScatter::axisX() const
{
    return static_cast<QValue3DAxis *>(QQuickGraphsItem::axisX());
}

void QQuickGraphsScatter::setAxisY(QValue3DAxis *axis)
{
    QQuickGraphsItem::setAxisY(axis);
}

QValue3DAxis *QQuickGraphsScatter::axisY() const
{
    return static_cast<QValue3DAxis *>(QQuickGraphsItem::axisY());
}

void QQuickGraphsScatter::setAxisZ(QValue3DAxis *axis)
{
    QQuickGraphsItem::setAxisZ(axis);
}

QValue3DAxis *QQuickGraphsScatter::axisZ() const
{
    return static_cast<QValue3DAxis *>(QQuickGraphsItem::axisZ());
}

void QQuickGraphsScatter::disconnectSeries(QScatter3DSeries *series)
{
    QObject::disconnect(series, 0, this, 0);
}

void QQuickGraphsScatter::generatePointsForScatterModel(ScatterModel *graphModel)
{
    QList<QQuick3DModel *> itemList;
    if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy) {
        qsizetype itemCount = graphModel->series->dataProxy()->itemCount();
        Q_TRACE_SCOPE(QGraphs3DScatterLegacyGeneratePointsForScatterModel, static_cast<int>(itemCount));
        if (graphModel->series->dataProxy()->itemCount() > 0)
            itemList.resize(itemCount);

        for (int i = 0; i < itemCount; i++) {
            QQuick3DModel *item = createDataItem(graphModel->series);
            item->setPickable(true);
            item->setParent(graphModel->series);
            itemList[i] = item;
        }
        graphModel->dataItems = itemList;
        markDataDirty();
    } else if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
        Q_TRACE_SCOPE(QGraphs3DScatterDefaultGeneratePointsForScatterModel);

        graphModel->instancingRootItem = createDataItem(graphModel->series);
        graphModel->instancingRootItem->setParent(graphModel->series);
        graphModel->instancingRootItem->setInstancing(graphModel->instancing);
        if (selectionMode() != QtGraphs3D::SelectionFlag::None) {
            graphModel->selectionIndicator = createDataItem(graphModel->series);
            graphModel->instancingRootItem->setPickable(true);
        }
    }
    markSeriesVisualsDirty();
}

qsizetype QQuickGraphsScatter::getItemIndex(QQuick3DModel *item)
{
    Q_UNUSED(item);
    if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy)
        return 0;

    return -1;
}

void QQuickGraphsScatter::clearSelection()
{
    clearSelectionModel();
}

void QQuickGraphsScatter::updateScatterGraphItemPositions(ScatterModel *graphModel)
{
    float itemSize = graphModel->series->itemSize() / m_itemScaler;
    QQuaternion meshRotation = graphModel->series->meshRotation();
    QScatterDataProxy *dataProxy = graphModel->series->dataProxy();
    QList<QQuick3DModel *> itemList = graphModel->dataItems;

    QValue3DAxis *valueAxisX = graphModel->series->axisX();
    QValue3DAxis *valueAxisY = graphModel->series->axisY();
    QValue3DAxis *valueAxisZ = graphModel->series->axisZ();

    if (!valueAxisX)
        valueAxisX = static_cast<QValue3DAxis *>(axisX());
    if (!valueAxisY)
        valueAxisY = static_cast<QValue3DAxis *>(axisY());
    if (!valueAxisZ)
        valueAxisZ = static_cast<QValue3DAxis *>(axisZ());

    bool xReversed = valueAxisX->reversed();
    bool yReversed = valueAxisY->reversed();
    bool zReversed = valueAxisZ->reversed();

    if (itemSize == 0.0f)
        itemSize = m_pointScale;

    if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy) {
        if (dataProxy->itemCount() != itemList.size()) {
            qCWarning(lcProperties3D, "%ls Item count differs from itemList count",
                     qUtf16Printable(QString::fromUtf8(__func__)));
        }
        Q_TRACE_SCOPE(QGraphs3DScatterLegacyModelsPositionUpdate);
        for (int i = 0; i < dataProxy->itemCount(); ++i) {
            const QScatterDataItem item = dataProxy->itemAt(i);
            QQuick3DModel *dataPoint = itemList.at(i);

            QVector3D dotPos = item.position();
            if (isDotPositionInAxisRange(dotPos, graphModel->series)) {
                dataPoint->setVisible(true);
                QQuaternion dotRot = item.rotation();
                float dotPosX = xReversed ? 1.0f - valueAxisX->positionAt(dotPos.x())
                                          : valueAxisX->positionAt(dotPos.x());
                float dotPosY = yReversed ? 1.0f - valueAxisY->positionAt(dotPos.y())
                                          : valueAxisY->positionAt(dotPos.y());
                float dotPosZ = zReversed ? 1.0f - valueAxisZ->positionAt(dotPos.z())
                                          : valueAxisZ->positionAt(dotPos.z());

                float posX = dotPosX * scale().x() + translate().x();
                float posY = dotPosY * scale().y() + translate().y();
                float posZ = dotPosZ * scale().z() + translate().z();
                dataPoint->setPosition(QVector3D(posX, posY, posZ));
                QQuaternion totalRotation;

                if (graphModel->series->mesh() != QAbstract3DSeries::Mesh::Point)
                    totalRotation = dotRot * meshRotation;
                else
                    totalRotation = cameraTarget()->rotation();

                dataPoint->setRotation(totalRotation);
                dataPoint->setScale(QVector3D(itemSize, itemSize, itemSize)
                                    * dataProxy->scaleAt(i));
            } else {
                dataPoint->setVisible(false);
            }
        }
    } else if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
        qsizetype count = dataProxy->itemCount();
        QList<DataItemHolder> positions;

        Q_TRACE_SCOPE(QGraphs3DScatterDefaultModelsPositionUpdate);
        for (int i = 0; i < count; i++) {
            const QScatterDataItem &item = dataProxy->itemAt(i);
            QVector3D dotPos = item.position();

            if (isDotPositionInAxisRange(dotPos, graphModel->series)) {
                float dotPosX = xReversed ? 1.0f - valueAxisX->positionAt(dotPos.x())
                                          : valueAxisX->positionAt(dotPos.x());
                float dotPosY = yReversed ? 1.0f - valueAxisY->positionAt(dotPos.y())
                                          : valueAxisY->positionAt(dotPos.y());
                float dotPosZ = zReversed ? 1.0f - valueAxisZ->positionAt(dotPos.z())
                                          : valueAxisZ->positionAt(dotPos.z());

                float posX = dotPosX * scale().x() + translate().x();
                float posY = dotPosY * scale().y() + translate().y();
                float posZ = dotPosZ * scale().z() + translate().z();

                QQuaternion totalRotation;

                if (graphModel->series->mesh() != QAbstract3DSeries::Mesh::Point)
                    totalRotation = item.rotation() * meshRotation;
                else
                    totalRotation = cameraTarget()->rotation();

                DataItemHolder dih;
                if (isPolar()) {
                    float x;
                    float z;
                    calculatePolarXZ(axisX()->positionAt(dotPos.x()),
                                     axisZ()->positionAt(dotPos.z()),
                                     x,
                                     z);
                    dih.position = {x, posY, z};
                } else {
                    dih.position = {posX, posY, posZ};
                }
                dih.rotation = totalRotation;
                dih.scale = QVector3D(itemSize, itemSize, itemSize) * dataProxy->scaleAt(i);

                positions.push_back(dih);
            } else {
                DataItemHolder dih;
                dih.hide = true;
                positions.push_back(dih);
            }
        }
        graphModel->instancing->setDataArray(positions);

        if (selectedItemInSeries(graphModel->series)) {
            if (isDotPositionInAxisRange(dataProxy->itemAt(m_selectedItem).position(), graphModel->series)) {
                QQuaternion totalRotation;

                if (graphModel->series->mesh() != QAbstract3DSeries::Mesh::Point) {
                    totalRotation = graphModel->instancing->dataArray().at(m_selectedItem).rotation
                                    * meshRotation;
                } else {
                    totalRotation = cameraTarget()->rotation();
                }
                graphModel->selectionIndicator->setRotation(totalRotation);
                graphModel->instancing->hideDataItem(m_selectedItem);
            } else {
                clearSelectionModel();
            }
        }
    }
}

void QQuickGraphsScatter::updateScatterGraphItemVisuals(ScatterModel *graphModel)
{
    bool useGradient = graphModel->series->d_func()->isUsingGradient();
    bool usePoint = graphModel->series->mesh() == QAbstract3DSeries::Mesh::Point;
    qsizetype itemCount = graphModel->series->dataProxy()->itemCount();

    if (useGradient) {
        if (!graphModel->seriesTexture) {
            graphModel->seriesTexture = createTexture();
            graphModel->seriesTexture->setParent(graphModel->series);
        }

        QLinearGradient gradient = graphModel->series->baseGradient();
        auto textureData = static_cast<QQuickGraphsTextureData *>(
            graphModel->seriesTexture->textureData());
        textureData->createGradient(gradient);

        if (!graphModel->highlightTexture) {
            graphModel->highlightTexture = createTexture();
            graphModel->highlightTexture->setParent(graphModel->series);
        }

        QLinearGradient highlightGradient = graphModel->series->singleHighlightGradient();
        auto highlightTextureData = static_cast<QQuickGraphsTextureData *>(
            graphModel->highlightTexture->textureData());
        highlightTextureData->createGradient(highlightGradient);
    } else {
        if (graphModel->seriesTexture) {
            graphModel->seriesTexture->deleteLater();
            graphModel->seriesTexture = nullptr;
        }

        if (graphModel->highlightTexture) {
            graphModel->highlightTexture->deleteLater();
            graphModel->highlightTexture = nullptr;
        }
    }

    bool rangeGradient = (useGradient
                          && graphModel->series->d_func()->m_colorStyle
                                 == QGraphsTheme::ColorStyle::RangeGradient)
                             ? true
                             : false;

    const bool shaded = graphModel->series->lightingMode()
        == QAbstract3DSeries::LightingMode::Shaded;
    if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy) {
        // Release resources that might not have been deleted even though deleteLater had been set
        if (m_customView)
            m_customView->window()->releaseResources();
        else
            window()->releaseResources();

        if (itemCount != graphModel->dataItems.size())
            qCWarning(lcGraphsScatter3D, "%ls Item count differs from itemList count",
                     qUtf16Printable(QString::fromUtf8(__func__)));

        bool transparentTexture = false;
        if (graphModel->seriesTexture) {
            auto textureData = static_cast<QQuickGraphsTextureData *>(
                graphModel->seriesTexture->textureData());
            transparentTexture = textureData->hasTransparency();
        }
        const bool transparency = (graphModel->series->baseColor().alphaF() < 1.0)
                                  || transparentTexture;

        updateMaterialReference(graphModel);
        QQmlListReference baseRef(graphModel->baseRef, "materials");
        auto baseMat = static_cast<QQuick3DCustomMaterial *>(baseRef.at(0));
        for (const auto &obj : std::as_const(graphModel->dataItems)) {
            QQmlListReference matRef(obj, "materials");
            matRef.clear();
            matRef.append(baseMat);
        }
        if (m_selectedItem != invalidSelectionIndex() && graphModel->series == selectedSeries()) {
            QQmlListReference selRef(graphModel->selectionRef, "materials");
            auto selMat = static_cast<QQuick3DCustomMaterial *>(selRef.at(0));
            QQuick3DModel *selectedItem = graphModel->dataItems.at(m_selectedItem);
            QQmlListReference matRef(selectedItem, "materials");
            matRef.clear();
            matRef.append(selMat);
        }
        updateItemMaterial(graphModel->baseRef,
                           useGradient,
                           rangeGradient,
                           usePoint,
                           QStringLiteral(":/materials/ScatterMaterial"));

        updateItemMaterial(graphModel->selectionRef,
                           useGradient,
                           rangeGradient,
                           usePoint,
                           QStringLiteral(":/materials/ScatterMaterial"));
        updateMaterialProperties(graphModel->baseRef,
                                 graphModel->seriesTexture,
                                 graphModel->series->baseColor(),
                                 transparency,
                                 shaded);

        updateMaterialProperties(graphModel->selectionRef,
                                 graphModel->highlightTexture,
                                 graphModel->series->singleHighlightColor(),
                                 shaded);

    } else if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
        graphModel->instancingRootItem->setVisible(true);
        graphModel->instancing->setRangeGradient(rangeGradient);
        if (!rangeGradient) {
            bool transparentTexture = false;
            if (graphModel->seriesTexture) {
                auto textureData = static_cast<QQuickGraphsTextureData *>(
                    graphModel->seriesTexture->textureData());
                transparentTexture = textureData->hasTransparency();
            }
            const bool transparency = (graphModel->series->baseColor().alphaF() < 1.0)
                                      || transparentTexture;
            if (transparency)
                graphModel->instancing->setTransparency(true);
            else
                graphModel->instancing->setTransparency(false);

            updateItemMaterial(graphModel->instancingRootItem,
                               useGradient,
                               rangeGradient,
                               usePoint,
                               QStringLiteral(":/materials/ScatterMaterialInstancing"));
            updateMaterialProperties(graphModel->instancingRootItem,
                                     graphModel->seriesTexture,
                                     graphModel->series->baseColor(),
                                     transparency,
                                     shaded);
        } else {
            auto textureData = static_cast<QQuickGraphsTextureData *>(
                graphModel->seriesTexture->textureData());
            if (textureData->hasTransparency())
                graphModel->instancing->setTransparency(true);
            else
                graphModel->instancing->setTransparency(false);

            updateItemMaterial(graphModel->instancingRootItem,
                               useGradient,
                               rangeGradient,
                               usePoint,
                               QStringLiteral(":/materials/ScatterMaterialInstancing"));
            updateInstancedMaterialProperties(graphModel,
                                              false,
                                              graphModel->seriesTexture,
                                              graphModel->highlightTexture,
                                              textureData->hasTransparency());

            const float scaleY = scaleWithBackground().y();
            float rangeGradientYScaler = m_rangeGradientYHelper / scaleY;

            QList<float> customData;
            customData.resize(itemCount);

            QList<DataItemHolder> instancingData = graphModel->instancing->dataArray();
            for (int i = 0; i < instancingData.size(); i++) {
                auto dih = instancingData.at(i);
                float value = (dih.position.y() + scaleY) * rangeGradientYScaler;
                customData[i] = value;
            }
            graphModel->instancing->setCustomData(customData);
        }

        if (selectedItemInSeries(graphModel->series)) {
            // Selection indicator
            if (!rangeGradient) {
                updateItemMaterial(graphModel->selectionIndicator,
                                   useGradient,
                                   rangeGradient,
                                   usePoint,
                                   QStringLiteral(":/materials/ScatterMaterial"));
                updateMaterialProperties(graphModel->selectionIndicator,
                                         graphModel->highlightTexture,
                                         graphModel->series->singleHighlightColor(),
                                         shaded);
                graphModel->selectionIndicator->setCastsShadows(!usePoint);
            } else {
                // Rangegradient
                updateItemMaterial(graphModel->selectionIndicator,
                                   useGradient,
                                   rangeGradient,
                                   usePoint,
                                   QStringLiteral(":/materials/ScatterMaterial"));
                updateInstancedMaterialProperties(graphModel,
                                                  true,
                                                  nullptr,
                                                  graphModel->highlightTexture);
                graphModel->selectionIndicator->setCastsShadows(!usePoint);
            }

            const DataItemHolder &dih = graphModel->instancing->dataArray().at(m_selectedItem);

            graphModel->selectionIndicator->setPosition(dih.position);
            graphModel->selectionIndicator->setRotation(dih.rotation);
            graphModel->selectionIndicator->setScale(dih.scale);
            graphModel->selectionIndicator->setVisible(true);
            itemLabel()->setVisible(graphModel->series->isItemLabelVisible());
            graphModel->instancing->hideDataItem(m_selectedItem);
            updateItemLabel(graphModel->selectionIndicator->position());
            graphModel->instancing->markDataDirty();
        } else if ((m_selectedItem == -1 || m_selectedItemSeries != graphModel->series)
                   && graphModel->selectionIndicator) {
            graphModel->selectionIndicator->setVisible(false);
        }
    }
}

void QQuickGraphsScatter::updateMaterialReference(ScatterModel *model)
{
    if (model->baseRef == nullptr) {
        model->baseRef = createDataItem(model->series);
        model->baseRef->setParent(model->series);
        model->baseRef->setVisible(false);
    }
    if (model->selectionRef == nullptr) {
        model->selectionRef = createDataItem(model->series);
        model->selectionRef->setParent(model->series);
        model->selectionRef->setVisible(false);
    }

    QQmlListReference baseRef(model->baseRef, "materials");
    QQmlListReference selectionRef(model->selectionRef, "materials");

    const QString &materialName = QStringLiteral(":/materials/ScatterMaterial");
    if (!baseRef.size()) {
        auto mat = createQmlCustomMaterial(materialName);
        mat->setObjectName(materialName);
        mat->setParent(model->baseRef);
        baseRef.append(mat);
    }
    if (!selectionRef.size()) {
        auto mat = createQmlCustomMaterial(materialName);
        mat->setObjectName(materialName + QStringLiteral("_Selection"));
        mat->setParent(model->selectionRef);
        selectionRef.append(mat);
    }
}
void QQuickGraphsScatter::updateItemMaterial(QQuick3DModel *item,
                                             bool useGradient,
                                             bool rangeGradient,
                                             bool usePoint,
                                             const QString &materialName)
{
    QQmlListReference materialsRef(item, "materials");
    bool needNewMat = false;
    if (!materialsRef.size()) {
        needNewMat = true;
    } else if (materialsRef.at(0)->objectName().contains(QStringLiteral("Instancing"))
               != materialName.contains(QStringLiteral("Instancing"))) {
        needNewMat = true;
    }

    if (needNewMat) {
        materialsRef.clear();
        auto newMaterial = createQmlCustomMaterial(materialName);
        newMaterial->setObjectName(materialName);
        newMaterial->setParent(item);
        materialsRef.append(newMaterial);
    }

    auto material = qobject_cast<QQuick3DCustomMaterial *>(materialsRef.at(0));
    if (!useGradient)
        material->setProperty("colorStyle", 0);
    else if (!rangeGradient)
        material->setProperty("colorStyle", 1);
    else
        material->setProperty("colorStyle", 2);

    material->setProperty("usePoint", usePoint);
    material->setProperty("rootScale", rootNode()->scale().y());
}

void QQuickGraphsScatter::updateInstancedMaterialProperties(ScatterModel *graphModel,
                                                            const bool isHighlight,
                                                            QQuick3DTexture *seriesTexture,
                                                            QQuick3DTexture *highlightTexture,
                                                            const bool transparency)
{
    QQuick3DModel *model = nullptr;
    if (isHighlight)
        model = graphModel->selectionIndicator;
    else
        model = graphModel->instancingRootItem;

    QQmlListReference materialsRef(model, "materials");

    auto customMaterial = static_cast<QQuick3DCustomMaterial *>(materialsRef.at(0));
    customMaterial->setProperty("transparency", transparency);
    customMaterial->setProperty("shaded",
                                graphModel->series->lightingMode()
                                    == QAbstract3DSeries::LightingMode::Shaded);

    QVariant textureInputAsVariant = customMaterial->property("custex");
    QQuick3DShaderUtilsTextureInput *textureInput = textureInputAsVariant
                                                        .value<QQuick3DShaderUtilsTextureInput *>();

    if (isHighlight) {
        textureInput->setTexture(highlightTexture);

        if (selectedItemInSeries(graphModel->series))
            m_selectedGradientPos = graphModel->instancing->customData().at(m_selectedItem);

        customMaterial->setProperty("gradientPos", m_selectedGradientPos);
    } else {
        textureInput->setTexture(seriesTexture);
    }
}

void QQuickGraphsScatter::updateMaterialProperties(QQuick3DModel *item,
                                                   QQuick3DTexture *texture,
                                                   QColor color,
                                                   const bool transparency,
                                                   const bool shaded)
{
    QQmlListReference materialsRef(item, "materials");
    auto customMaterial = static_cast<QQuick3DCustomMaterial *>(materialsRef.at(0));
    customMaterial->setProperty("transparency", transparency);
    customMaterial->setProperty("rootScale", rootNode()->scale().y());
    customMaterial->setProperty("shaded", shaded);

    int style = customMaterial->property("colorStyle").value<int>();
    if (style == 0) {
        customMaterial->setProperty("uColor", color);
    } else {
        QVariant textureInputAsVariant = customMaterial->property("custex");
        QQuick3DShaderUtilsTextureInput *textureInput
            = textureInputAsVariant.value<QQuick3DShaderUtilsTextureInput *>();

        textureInput->setTexture(texture);

        const float scaleY = scaleWithBackground().y();
        float rangeGradientYScaler = m_rangeGradientYHelper / scaleY;
        float value = (item->y() + scaleY) * rangeGradientYScaler;
        customMaterial->setProperty("gradientPos", value);
    }
}

QQuick3DTexture *QQuickGraphsScatter::createTexture()
{
    QQuick3DTexture *texture = new QQuick3DTexture();
    texture->setParent(this);
    texture->setRotationUV(-90.0f);
    texture->setHorizontalTiling(QQuick3DTexture::ClampToEdge);
    texture->setVerticalTiling(QQuick3DTexture::ClampToEdge);
    QQuickGraphsTextureData *textureData = new QQuickGraphsTextureData();
    textureData->setParent(texture);
    textureData->setParentItem(texture);
    texture->setTextureData(textureData);

    return texture;
}

QQuick3DNode *QQuickGraphsScatter::createSeriesRoot()
{
    auto model = new QQuick3DNode();

    model->setParentItem(graphNode());
    return model;
}

QQuick3DModel *QQuickGraphsScatter::createDataItem(QAbstract3DSeries *series)
{
    auto model = new QQuick3DModel();
    model->setParent(this);
    model->setParentItem(graphNode());
    QString fileName = getMeshFileName(series);
    if (fileName.isEmpty())
        fileName = series->userDefinedMesh();

    model->setSource(QUrl(fileName));
    return model;
}

void QQuickGraphsScatter::removeDataItems(ScatterModel *graphModel,
                                          QtGraphs3D::OptimizationHint optimizationHint)
{
    if (optimizationHint == QtGraphs3D::OptimizationHint::Default) {
        delete graphModel->instancing;
        graphModel->instancing = nullptr;
        deleteDataItem(graphModel->instancingRootItem);
        deleteDataItem(graphModel->selectionIndicator);
        deleteDataItem(graphModel->baseRef);
        deleteDataItem(graphModel->selectionRef);
        deleteDataItem(graphModel->splineModel);

        graphModel->instancingRootItem = nullptr;
        graphModel->selectionIndicator = nullptr;
        graphModel->baseRef = nullptr;
        graphModel->selectionRef = nullptr;
        graphModel->splineModel = nullptr;
    } else {
        QList<QQuick3DModel *> &items = graphModel->dataItems;
        removeDataItems(items, items.count());
    }
}

void QQuickGraphsScatter::removeDataItems(QList<QQuick3DModel *> &items, qsizetype count)
{
    for (int i = 0; i < count; ++i) {
        QQuick3DModel *item = items.takeLast();
        QQmlListReference materialsRef(item, "materials");
        if (materialsRef.size()) {
            QObject *material = materialsRef.at(0);
            delete material;
        }
        item->deleteLater();
    }
}

QList<QScatter3DSeries *> QQuickGraphsScatter::scatterSeriesList()
{
    QList<QScatter3DSeries *> scatterSeriesList;
    for (QAbstract3DSeries *abstractSeries : std::as_const(m_seriesList)) {
        QScatter3DSeries *scatterSeries = qobject_cast<QScatter3DSeries *>(abstractSeries);
        if (scatterSeries)
            scatterSeriesList.append(scatterSeries);
    }

    return scatterSeriesList;
}

void QQuickGraphsScatter::recreateDataItems()
{
    if (!isComponentComplete())
        return;
    QList<QScatter3DSeries *> seriesList = scatterSeriesList();
    for (auto series : std::as_const(seriesList)) {
        for (const auto &model : std::as_const(m_scatterGraphs)) {
            if (model->series == series)
                removeDataItems(model, optimizationHint());
        }
    }
    markDataDirty();
}

void QQuickGraphsScatter::recreateDataItems(const QList<ScatterModel *> &graphs)
{
    if (!isComponentComplete())
        return;
    QList<QScatter3DSeries *> seriesList = scatterSeriesList();
    for (auto series : std::as_const(seriesList)) {
        for (const auto &model : graphs) {
            if (model->series == series)
                removeDataItems(model, optimizationHint());
        }
    }
    markDataDirty();
}

void QQuickGraphsScatter::addPointsToScatterModel(ScatterModel *graphModel, qsizetype count)
{
    for (int i = 0; i < count; i++) {
        QQuick3DModel *item = createDataItem(graphModel->series);
        item->setPickable(true);
        item->setParent(graphModel->series);
        graphModel->dataItems.push_back(item);
    }
    setSeriesVisualsDirty();
}

qsizetype QQuickGraphsScatter::sizeDifference(qsizetype size1, qsizetype size2)
{
    return size2 - size1;
}

QVector3D QQuickGraphsScatter::selectedItemPosition()
{
    QVector3D position;
    if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy)
        position = {0.0f, 0.0f, 0.0f};
    else if (optimizationHint() == QtGraphs3D::OptimizationHint::Default)
        position = {0.0f, 0.0f, 0.0f};

    return position;
}

void QQuickGraphsScatter::fixMeshFileName(QString &fileName, QAbstract3DSeries *series)
{
    auto meshType = series->mesh();
    // Should it be smooth?
    if (series->isMeshSmooth() && meshType != QAbstract3DSeries::Mesh::Point
        && meshType != QAbstract3DSeries::Mesh::UserDefined) {
        fileName += QStringLiteral("Smooth");
    }

    // Should it be filled?
    if (meshType != QAbstract3DSeries::Mesh::Sphere && meshType != QAbstract3DSeries::Mesh::Arrow
        && meshType != QAbstract3DSeries::Mesh::Minimal
        && meshType != QAbstract3DSeries::Mesh::Point
        && meshType != QAbstract3DSeries::Mesh::UserDefined) {
        fileName.append(QStringLiteral("Full"));
    }
}

QString QQuickGraphsScatter::getMeshFileName(QAbstract3DSeries *series)
{
    QString fileName = {};
    switch (series->mesh()) {
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
    case QAbstract3DSeries::Mesh::Minimal:
        fileName = QStringLiteral("defaultMeshes/minimalMesh");
        break;
    case QAbstract3DSeries::Mesh::Arrow:
        fileName = QStringLiteral("defaultMeshes/arrowMesh");
        break;
    case QAbstract3DSeries::Mesh::Point:
        fileName = shadowQuality() == QtGraphs3D::ShadowQuality::None
                       ? QStringLiteral("defaultMeshes/planeMesh")
                       : QStringLiteral("defaultMeshes/octagonMesh");
        break;
    case QAbstract3DSeries::Mesh::UserDefined:
        break;
    default:
        fileName = QStringLiteral("defaultMeshes/sphereMesh");
    }

    fixMeshFileName(fileName, series);

    return fileName;
}

void QQuickGraphsScatter::deleteDataItem(QQuick3DModel *item)
{
    if (item) {
        QQmlListReference materialsRef(item, "materials");
        if (materialsRef.size()) {
            QObject *material = materialsRef.at(0);
            delete material;
        }
        item->deleteLater();
        item = nullptr;
    }
}

void QQuickGraphsScatter::handleSeriesChanged(QList<QAbstract3DSeries *> changedSeries)
{
    Q_UNUSED(changedSeries)
    // TODO: generate items and remove old items
}

bool QQuickGraphsScatter::selectedItemInSeries(const QScatter3DSeries *series)
{
    return (m_selectedItem != -1 && m_selectedItemSeries == series);
}

bool QQuickGraphsScatter::isDotPositionInAxisRange(QVector3D dotPos, QScatter3DSeries *series)
{
    float cutoffMargin = QQuickGraphsItem::cutoffMargin();
    QValue3DAxis *xAxis = series->axisX();
    QValue3DAxis *yAxis = series->axisY();
    QValue3DAxis *zAxis = series->axisZ();

    if (!xAxis)
        xAxis = static_cast<QValue3DAxis *>(axisX());
    if (!yAxis)
        yAxis = static_cast<QValue3DAxis *>(axisY());
    if (!zAxis)
        zAxis = static_cast<QValue3DAxis *>(axisZ());

    float axisMinX = xAxis->min() - cutoffMargin;
    float axisMinY = yAxis->min() - cutoffMargin;
    float axisMinZ = zAxis->min() - cutoffMargin;

    float axisMaxX = xAxis->max() + cutoffMargin;
    float axisMaxY = yAxis->max() + cutoffMargin;
    float axisMaxZ = zAxis->max() + cutoffMargin;

    return ((dotPos.x() >= axisMinX && dotPos.x() <= axisMaxX)
            && (dotPos.y() >= axisMinY && dotPos.y() <= axisMaxY)
            && (dotPos.z() >= axisMinZ && dotPos.z() <= axisMaxZ));
}

QScatter3DSeries *QQuickGraphsScatter::selectedSeries() const
{
    return m_selectedItemSeries;
}

void QQuickGraphsScatter::setSelectedItem(qsizetype index, QScatter3DSeries *series)
{
    const QScatterDataProxy *proxy = 0;

    // Series may already have been removed, so check it before setting the selection.
    if (!m_seriesList.contains(series))
        series = nullptr;

    if (series)
        proxy = series->dataProxy();

    if (!proxy || index < 0 || index >= proxy->itemCount())
        index = invalidSelectionIndex();

    if (index != m_selectedItem || series != m_selectedItemSeries) {
        bool seriesChanged = (series != m_selectedItemSeries);

        // Clear hidden point from the previous selected series
        if (seriesChanged) {
            for (auto model : std::as_const(m_scatterGraphs)) {
                if (model->series && model->instancing && model->series == m_selectedItemSeries)
                    model->instancing->unhidePreviousDataItem();
            }
        }

        m_selectedItem = index;
        m_selectedItemSeries = series;
        m_changeTracker.selectedItemChanged = true;

        // Clear selection from other series and finally set new selection to the
        // specified series
        for (QAbstract3DSeries *otherSeries : std::as_const(m_seriesList)) {
            QScatter3DSeries *scatterSeries = static_cast<QScatter3DSeries *>(otherSeries);
            if (scatterSeries != m_selectedItemSeries)
                scatterSeries->d_func()->setSelectedItem(invalidSelectionIndex());
        }
        if (m_selectedItemSeries)
            m_selectedItemSeries->d_func()->setSelectedItem(m_selectedItem);

        if (seriesChanged)
            emit selectedSeriesChanged(m_selectedItemSeries);

        emitNeedRender();
    }

    if (index != invalidSelectionIndex())
        itemLabel()->setVisible(series->isItemLabelVisible());
}

void QQuickGraphsScatter::setSelectionMode(QtGraphs3D::SelectionFlags mode)
{
    // We only support single item selection mode and no selection mode
    if (mode != QtGraphs3D::SelectionFlag::Item && mode != QtGraphs3D::SelectionFlag::None) {
        qCWarning(lcProperties3D, "%s unsupported selection mode - only none and item selection modes "
                 "are supported", qUtf8Printable(QLatin1String(__FUNCTION__)));
        return;
    }

    QQuickGraphsItem::setSelectionMode(mode);
}

void QQuickGraphsScatter::handleAxisAutoAdjustRangeChangedInOrientation(
    QAbstract3DAxis::AxisOrientation orientation, bool autoAdjust)
{
    Q_UNUSED(orientation);
    Q_UNUSED(autoAdjust);
    adjustAxisRanges();
}

void QQuickGraphsScatter::handleAxisRangeChangedBySender(QObject *sender)
{
    QQuickGraphsItem::handleAxisRangeChangedBySender(sender);

    m_isDataDirty = true;

    // Update selected index - may be moved offscreen
    setSelectedItem(m_selectedItem, m_selectedItemSeries);
}

void QQuickGraphsScatter::handleLightingModeChanged() {
    auto series = static_cast<QScatter3DSeries *>(QObject::sender());
    for (auto model : std::as_const(m_scatterGraphs)) {
        if (model->series == series) {
            updateScatterGraphItemVisuals(model);
            break;
        }
    }
}

QQmlListProperty<QScatter3DSeries> QQuickGraphsScatter::seriesList()
{
    return QQmlListProperty<QScatter3DSeries>(this,
                                              this,
                                              &QQuickGraphsScatter::appendSeriesFunc,
                                              &QQuickGraphsScatter::countSeriesFunc,
                                              &QQuickGraphsScatter::atSeriesFunc,
                                              &QQuickGraphsScatter::clearSeriesFunc);
}

void QQuickGraphsScatter::appendSeriesFunc(QQmlListProperty<QScatter3DSeries> *list,
                                           QScatter3DSeries *series)
{
    reinterpret_cast<QQuickGraphsScatter *>(list->data)->addSeries(series);
}

qsizetype QQuickGraphsScatter::countSeriesFunc(QQmlListProperty<QScatter3DSeries> *list)
{
    return reinterpret_cast<QQuickGraphsScatter *>(list->data)->scatterSeriesList().size();
}

QScatter3DSeries *QQuickGraphsScatter::atSeriesFunc(QQmlListProperty<QScatter3DSeries> *list,
                                                    qsizetype index)
{
    return reinterpret_cast<QQuickGraphsScatter *>(list->data)->scatterSeriesList().at(index);
}

void QQuickGraphsScatter::clearSeriesFunc(QQmlListProperty<QScatter3DSeries> *list)
{
    QQuickGraphsScatter *declScatter = reinterpret_cast<QQuickGraphsScatter *>(list->data);
    QList<QScatter3DSeries *> realList = declScatter->scatterSeriesList();
    qsizetype count = realList.size();
    for (int i = 0; i < count; i++)
        declScatter->removeSeries(realList.at(i));
}

void QQuickGraphsScatter::addSeries(QScatter3DSeries *series)
{
    Q_ASSERT(series && series->type() == QAbstract3DSeries::SeriesType::Scatter);

    QQuickGraphsItem::addSeriesInternal(series);

    QScatter3DSeries *scatterSeries = static_cast<QScatter3DSeries *>(series);
    if (scatterSeries->selectedItem() != invalidSelectionIndex())
        setSelectedItem(scatterSeries->selectedItem(), scatterSeries);

    auto graphModel = new ScatterModel;
    graphModel->series = series;
    graphModel->seriesTexture = nullptr;
    graphModel->highlightTexture = nullptr;
    m_scatterGraphs.push_back(graphModel);

    connectSeries(series);

    if (series->selectedItem() != invalidSelectionIndex())
        setSelectedItem(series->selectedItem(), series);

    if (scatterSeries->axisX())
        handleMultiAxisChanged(scatterSeries->axisX());
    if (scatterSeries->axisY())
        handleMultiAxisChanged(scatterSeries->axisY());
    if (scatterSeries->axisZ())
        handleMultiAxisChanged(scatterSeries->axisZ());

}

void QQuickGraphsScatter::removeSeries(QScatter3DSeries *series)
{
    bool wasVisible = (series && series->d_func()->m_graph == this && series->isVisible());

    QQuickGraphsItem::removeSeriesInternal(series);

    if (m_selectedItemSeries == series)
        setSelectedItem(invalidSelectionIndex(), 0);

    if (wasVisible)
        adjustAxisRanges();

    series->setParent(this); // Reparent as removing will leave series parentless

    // Find scattergraph model
    for (QList<ScatterModel *>::Iterator it = m_scatterGraphs.begin();
         it != m_scatterGraphs.end();) {
        if ((*it)->series == series) {
            removeDataItems(*it, optimizationHint());

            if ((*it)->seriesTexture)
                delete (*it)->seriesTexture;
            if ((*it)->highlightTexture)
                delete (*it)->highlightTexture;

            delete *it;
            it = m_scatterGraphs.erase(it);
        } else {
            ++it;
        }
    }

    QObject::disconnect(series,
                     &QScatter3DSeries::axisXChanged,
                     this,
                     &QQuickGraphsScatter::handleMultiAxisChanged);
    QObject::disconnect(series,
                     &QScatter3DSeries::axisYChanged,
                     this,
                     &QQuickGraphsScatter::handleMultiAxisChanged);
    QObject::disconnect(series,
                     &QScatter3DSeries::axisZChanged,
                     this,
                     &QQuickGraphsScatter::handleMultiAxisChanged);

    disconnectSeries(series);
}

void QQuickGraphsScatter::handleAxisXChanged(QAbstract3DAxis *axis)
{
    emit axisXChanged(static_cast<QValue3DAxis *>(axis));
}

void QQuickGraphsScatter::handleAxisYChanged(QAbstract3DAxis *axis)
{
    emit axisYChanged(static_cast<QValue3DAxis *>(axis));
}

void QQuickGraphsScatter::handleAxisZChanged(QAbstract3DAxis *axis)
{
    emit axisZChanged(static_cast<QValue3DAxis *>(axis));
}

void QQuickGraphsScatter::handleMultiAxisChanged(QAbstract3DAxis *axis)
{
    QQuickGraphsItem::handleMultiAxisChanged(axis);
}

QAbstract3DAxis *QQuickGraphsScatter::getSeriesMultiAxis(QAbstract3DSeries *series,
                   QAbstract3DAxis::AxisOrientation orientation)
{
    QScatter3DSeries *scatterSeries = qobject_cast<QScatter3DSeries *>(series);

    if (!scatterSeries)
        return nullptr;

    QAbstract3DAxis *axis = nullptr;
    switch (orientation)  {
    case QAbstract3DAxis::AxisOrientation::X:
        axis = scatterSeries->axisX();
    break;
    case QAbstract3DAxis::AxisOrientation::Y:
        axis = scatterSeries->axisY();
    break;
    case QAbstract3DAxis::AxisOrientation::Z:
        axis = scatterSeries->axisZ();
    break;
    case QAbstract3DAxis::AxisOrientation::None:
    break;
    }

    if (axis)
        axis->d_func()->setOrientation(orientation);

    return axis;
}

void QQuickGraphsScatter::handleSeriesMeshChanged()
{
    recreateDataItems();
}

void QQuickGraphsScatter::handleMeshSmoothChanged(bool enable)
{
    Q_UNUSED(enable);
    QScatter3DSeries *series = qobject_cast<QScatter3DSeries *>(sender());
    for (auto &model : std::as_const(m_scatterGraphs)) {
        if (model->series == series)
            removeDataItems(model, optimizationHint());
    }
    markDataDirty();
}

void QQuickGraphsScatter::handleArrayReset()
{
    QScatter3DSeries *series;
    if (qobject_cast<QScatterDataProxy *>(sender()))
        series = static_cast<QScatterDataProxy *>(sender())->series();
    else
        series = static_cast<QScatter3DSeries *>(sender());

    if (series->isVisible()) {
        adjustAxisRanges();
        m_isDataDirty = true;
    }
    if (!m_changedSeriesList.contains(series))
        m_changedSeriesList.append(series);
    setSelectedItem(m_selectedItem, m_selectedItemSeries);
    series->d_func()->markItemLabelDirty();
    emitNeedRender();
}

void QQuickGraphsScatter::handleItemsAdded(qsizetype startIndex, qsizetype count)
{
    Q_UNUSED(startIndex);
    Q_UNUSED(count);
    QScatter3DSeries *series = static_cast<QScatterDataProxy *>(sender())->series();
    if (series->isVisible()) {
        adjustAxisRanges();
        m_isDataDirty = true;
    }
    if (!m_changedSeriesList.contains(series))
        m_changedSeriesList.append(series);
    emitNeedRender();
}

void QQuickGraphsScatter::handleItemsChanged(qsizetype startIndex, qsizetype count)
{
    QScatter3DSeries *series = static_cast<QScatterDataProxy *>(sender())->series();
    qsizetype oldChangeCount = m_changedItems.size();
    if (!oldChangeCount)
        m_changedItems.reserve(count);

    for (int i = 0; i < count; i++) {
        bool newItem = true;
        qsizetype candidate = startIndex + i;
        for (qsizetype j = 0; j < oldChangeCount; j++) {
            const ChangeItem &oldChangeItem = m_changedItems.at(j);
            if (oldChangeItem.index == candidate && series == oldChangeItem.series) {
                newItem = false;
                break;
            }
        }
        if (newItem) {
            ChangeItem newChangeItem = {series, candidate};
            m_changedItems.append(newChangeItem);
            if (series == m_selectedItemSeries && m_selectedItem == candidate)
                series->d_func()->markItemLabelDirty();
        }
    }

    if (count) {
        m_changeTracker.itemChanged = true;
        if (series->isVisible())
            adjustAxisRanges();
        emitNeedRender();
    }
}

void QQuickGraphsScatter::handleItemsRemoved(qsizetype startIndex, qsizetype count)
{
    Q_UNUSED(startIndex);
    Q_UNUSED(count);
    QScatter3DSeries *series = static_cast<QScatterDataProxy *>(sender())->series();
    if (series == m_selectedItemSeries) {
        // If items removed from selected series before the selection, adjust the selection
        qsizetype selectedItem = m_selectedItem;
        if (startIndex <= selectedItem) {
            if ((startIndex + count) > selectedItem)
                selectedItem = -1; // Selected item removed
            else
                selectedItem -= count; // Move selected item down by amount of item removed

            setSelectedItem(selectedItem, m_selectedItemSeries);
        }
    }

    if (series->isVisible()) {
        adjustAxisRanges();
        m_isDataDirty = true;
    }
    if (!m_changedSeriesList.contains(series))
        m_changedSeriesList.append(series);

    if (m_recordInsertsAndRemoves) {
        InsertRemoveRecord record(false, startIndex, count, series);
        m_insertRemoveRecords.append(record);
    }

    emitNeedRender();
}

void QQuickGraphsScatter::adjustAxisRanges()
{
    QValue3DAxis *valueAxisX = static_cast<QValue3DAxis *>(m_axisX);
    QValue3DAxis *valueAxisY = static_cast<QValue3DAxis *>(m_axisY);
    QValue3DAxis *valueAxisZ = static_cast<QValue3DAxis *>(m_axisZ);
    bool adjustX = (valueAxisX && valueAxisX->isAutoAdjustRange());
    bool adjustY = (valueAxisY && valueAxisY->isAutoAdjustRange());
    bool adjustZ = (valueAxisZ && valueAxisZ->isAutoAdjustRange());

    if (adjustX || adjustY || adjustZ) {
        float minValueX = 0.0f;
        float maxValueX = 0.0f;
        float minValueY = 0.0f;
        float maxValueY = 0.0f;
        float minValueZ = 0.0f;
        float maxValueZ = 0.0f;
        qsizetype seriesCount = m_seriesList.size();
        for (int series = 0; series < seriesCount; series++) {
            const QScatter3DSeries *scatterSeries = static_cast<QScatter3DSeries *>(
                m_seriesList.at(series));
            const QScatterDataProxy *proxy = scatterSeries->dataProxy();
            if (scatterSeries->isVisible() && proxy) {
                QVector3D minLimits;
                QVector3D maxLimits;
                proxy->d_func()->limitValues(minLimits,
                                             maxLimits,
                                             valueAxisX,
                                             valueAxisY,
                                             valueAxisZ);
                if (adjustX) {
                    if (!series) {
                        // First series initializes the values
                        minValueX = minLimits.x();
                        maxValueX = maxLimits.x();
                    } else {
                        minValueX = qMin(minValueX, minLimits.x());
                        maxValueX = qMax(maxValueX, maxLimits.x());
                    }
                }
                if (adjustY) {
                    if (!series) {
                        // First series initializes the values
                        minValueY = minLimits.y();
                        maxValueY = maxLimits.y();
                    } else {
                        minValueY = qMin(minValueY, minLimits.y());
                        maxValueY = qMax(maxValueY, maxLimits.y());
                    }
                }
                if (adjustZ) {
                    if (!series) {
                        // First series initializes the values
                        minValueZ = minLimits.z();
                        maxValueZ = maxLimits.z();
                    } else {
                        minValueZ = qMin(minValueZ, minLimits.z());
                        maxValueZ = qMax(maxValueZ, maxLimits.z());
                    }
                }
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

void QQuickGraphsScatter::handleItemsInserted(qsizetype startIndex, qsizetype count)
{
    Q_UNUSED(startIndex);
    Q_UNUSED(count);
    QScatter3DSeries *series = static_cast<QScatterDataProxy *>(sender())->series();
    if (series == m_selectedItemSeries) {
        // If items inserted to selected series before the selection, adjust the selection
        qsizetype selectedItem = m_selectedItem;
        if (startIndex <= selectedItem) {
            selectedItem += count;
            setSelectedItem(selectedItem, m_selectedItemSeries);
        }
    }

    if (series->isVisible()) {
        adjustAxisRanges();
        m_isDataDirty = true;
    }
    if (!m_changedSeriesList.contains(series))
        m_changedSeriesList.append(series);

    if (m_recordInsertsAndRemoves) {
        InsertRemoveRecord record(true, startIndex, count, series);
        m_insertRemoveRecords.append(record);
    }

    emitNeedRender();
}

bool QQuickGraphsScatter::doPicking(QPointF position)
{
    if (!QQuickGraphsItem::doPicking(position))
        return false;
    if (selectionMode() == QtGraphs3D::SelectionFlag::Item) {
        Q_TRACE_SCOPE(QGraphs3DScatterDoPicking);
        QList<QQuick3DPickResult> results;
        if (m_customView)
            results = m_customView->pickAll(position.x(), position.y());
        else
            results = pickAll(position.x(), position.y());
        if (!results.empty()) {
            for (const auto &result : std::as_const(results)) {
                if (const auto &hitItem = result.objectHit()) {
                    if (hitItem == backgroundBB() || hitItem == background()) {
                        m_clickedType = QtGraphs3D::ElementType::None;
                        clearSelectionModel();
                        continue;
                    }

                    if (m_clickedType != QtGraphs3D::ElementType::AxisXLabel
                        && m_clickedType != QtGraphs3D::ElementType::AxisYLabel
                        && m_clickedType != QtGraphs3D::ElementType::AxisZLabel) {
                        if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy) {
                            setSelected(hitItem);
                            handleSelectedElementChange(QtGraphs3D::ElementType::Series);
                            break;
                        } else if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
                            setSelected(hitItem, result.instanceIndex());
                            handleSelectedElementChange(QtGraphs3D::ElementType::Series);
                            const auto& hitSeries = static_cast<QScatter3DSeries *>(hitItem->parent());
                            qCDebug(lcInput3D) << "pick results:"
                                << "\n hit item position:" << position
                                << "\n hit item instance index:" << result.instanceIndex()
                                << "\n hit item values:" << hitSeries->dataProxy()->itemAt(result.instanceIndex()).position();
                            break;
                        }
                    } else {
                        clearSelectionModel();
                    }
                }
            }
        } else {
            clearSelectionModel();
            handleSelectedElementChange(QtGraphs3D::ElementType::None);
        }
    }

    return true;
}

bool QQuickGraphsScatter::doRayPicking(QVector3D origin, QVector3D direction)
{
    if (!QQuickGraphsItem::doRayPicking(origin, direction))
        return false;

    if (selectionMode() == QtGraphs3D::SelectionFlag::Item) {
        Q_TRACE_SCOPE(QGraphs3DScatterDoRayPicking);
        QList<QQuick3DPickResult> results;
        if (m_customView)
            results = m_customView->rayPickAll(origin, direction);
        else
            results = rayPickAll(origin, direction);
        if (!results.empty()) {
            for (const auto &result : std::as_const(results)) {
                if (const auto &hit = result.objectHit()) {
                    if (hit == backgroundBB() || hit == background()) {
                        clearSelectionModel();
                        continue;
                    }
                    if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy) {
                        setSelected(hit);
                        break;
                    } else if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
                        setSelected(hit, result.instanceIndex());
                        const auto& hitSeries = static_cast<QScatter3DSeries *>(hit->parent());
                        qCDebug(lcInput3D) << "pick results:"
                            << "\n hit item instance index:" << result.instanceIndex()
                            << "\n hit item values:" << hitSeries->dataProxy()->itemAt(result.instanceIndex()).position();
                        break;
                    }
                }
            }
        } else {
            clearSelectionModel();
        }
    }
    return true;
}

void QQuickGraphsScatter::updateShadowQuality(QtGraphs3D::ShadowQuality quality)
{
    // Were shadows visible before?
    bool prevShadowsVisible = light()->castsShadow();
    QQuickGraphsItem::updateShadowQuality(quality);
    setSeriesVisualsDirty();

    if (prevShadowsVisible != light()->castsShadow()) {
        // Need to change mesh for series using point type
        QList<ScatterModel *> graphs;
        for (const auto &graph : std::as_const(m_scatterGraphs)) {
            if (graph->series->mesh() == QAbstract3DSeries::Mesh::Point)
                graphs.append(graph);
        }
        recreateDataItems(graphs);
    }
}

void QQuickGraphsScatter::updateLightStrength()
{
    for (auto graphModel : std::as_const(m_scatterGraphs)) {
        for (const auto &obj : std::as_const(graphModel->dataItems)) {
            QQmlListReference materialsRef(obj, "materials");
            auto material = qobject_cast<QQuick3DCustomMaterial *>(materialsRef.at(0));
            material->setProperty("specularBrightness", lightStrength() * 0.05);
        }
    }
}

void QQuickGraphsScatter::startRecordingRemovesAndInserts()
{
    m_recordInsertsAndRemoves = false;

    if (m_scene->selectionQueryPosition() != m_scene->invalidSelectionPoint()) {
        m_recordInsertsAndRemoves = true;
        if (m_insertRemoveRecords.size()) {
            m_insertRemoveRecords.clear();
            // Reserve some space for remove/insert records to avoid unnecessary reallocations.
            m_insertRemoveRecords.reserve(insertRemoveRecordReserveSize);
        }
    }
}

void QQuickGraphsScatter::componentComplete()
{
    QQuickGraphsItem::componentComplete();
    QObject::connect(cameraTarget(),
                     &QQuick3DNode::rotationChanged,
                     this,
                     &QQuickGraphsScatter::cameraRotationChanged);

    graphsInputHandler()->setGraphsItem(this);
    qCDebug(lcGraphs3D) << "QQuickGraphsScatter::componentComplete";
}

void QQuickGraphsScatter::connectSeries(QScatter3DSeries *series)
{
    QObject::connect(series,
                     &QScatter3DSeries::meshChanged,
                     this,
                     &QQuickGraphsScatter::handleSeriesMeshChanged);
    QObject::connect(series,
                     &QScatter3DSeries::meshSmoothChanged,
                     this,
                     &QQuickGraphsScatter::handleMeshSmoothChanged);
    QObject::connect(series,
                     &QScatter3DSeries::itemSizeChanged,
                     this,
                     &QQuickGraphsScatter::markDataDirty);
    QObject::connect(series,
                     &QScatter3DSeries::axisXChanged,
                     this,
                     &QQuickGraphsScatter::handleMultiAxisChanged);
    QObject::connect(series,
                     &QScatter3DSeries::axisYChanged,
                     this,
                     &QQuickGraphsScatter::handleMultiAxisChanged);
    QObject::connect(series,
                     &QScatter3DSeries::axisZChanged,
                     this,
                     &QQuickGraphsScatter::handleMultiAxisChanged);
}

void QQuickGraphsScatter::calculateSceneScalingFactors()
{
    float marginV = 0.0f;
    float marginH = 0.0f;
    if (margin() < 0.0f) {
        if (m_maxItemSize > m_defaultMaxSize)
            marginH = m_maxItemSize / m_itemScaler;
        else
            marginH = m_defaultMaxSize;
        marginV = marginH;
    } else {
        marginH = margin();
        marginV = margin();
    }
    if (isPolar()) {
        float polarMargin = calculatePolarBackgroundMargin();
        marginH = qMax(marginH, polarMargin);
    }

    float tHorizontalAspectRatio;
    if (isPolar())
        tHorizontalAspectRatio = 1.0f;
    else
        tHorizontalAspectRatio = horizontalAspectRatio();

    QSizeF areaSize;
    if (qFuzzyIsNull(tHorizontalAspectRatio)) {
        areaSize.setHeight(axisZ()->max() - axisZ()->min());
        areaSize.setWidth(axisX()->max() - axisX()->min());
    } else {
        areaSize.setHeight(1.0f);
        areaSize.setWidth(tHorizontalAspectRatio);
    }

    float horizontalMaxDimension;
    float scaleY = 0.0f;
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
    float scaleX = horizontalMaxDimension * areaSize.width() / scaleFactor;
    float scaleZ = horizontalMaxDimension * areaSize.height() / scaleFactor;

    setBackgroundScaleMargin({marginH, marginV, marginH});

    setScaleWithBackground({scaleX, scaleY, scaleZ});
    setScale({scaleX * 2.0f, scaleY * 2.0f, scaleZ * -2.0f});
    setTranslate({-scaleX, -scaleY, scaleZ});
}

float QQuickGraphsScatter::calculatePointScaleSize()
{
    QList<QScatter3DSeries *> series = scatterSeriesList();
    int totalDataSize = 0;
    for (const auto &scatterSeries : std::as_const(series)) {
        if (scatterSeries->isVisible())
            totalDataSize += scatterSeries->dataArray().size();
    }

    return qBound(m_defaultMinSize, 2.0f / float(qSqrt(qreal(totalDataSize))), m_defaultMaxSize);
}

void QQuickGraphsScatter::updatePointScaleSize()
{
    m_pointScale = calculatePointScaleSize();
}

void QQuickGraphsScatter::calculatePolarXZ(const float posX,
                                           const float posZ,
                                           float &x,
                                           float &z) const
{
    const qreal angle = posX * (M_PI * 2.0f);
    const qreal radius = posZ;

    x = static_cast<float>(radius * qSin(angle)) * m_polarRadius;
    z = -static_cast<float>(radius * qCos(angle)) * m_polarRadius;
}

void QQuickGraphsScatter::updateSpline(ScatterModel *model)
{
    if (auto series = qobject_cast<QSpline3DSeries *>(model->series)) {
        Q_TRACE_SCOPE(QGraphs3DScatterSplineUpdate);
        if (!series->isSplineVisible()) {
            if (model->splineModel)
                model->splineModel->setVisible(false);
            return;
        } else {
            if (!model->splineModel)
                createSplineModel(model);

            QQmlListReference materialRef(model->splineModel, "materials");
            QQuick3DCustomMaterial *material = qobject_cast<QQuick3DCustomMaterial *>(
                materialRef.at(0));

            QVariant splineInputAsVariant = material->property("controlPoints");
            QQuick3DShaderUtilsTextureInput *splineInput
                = splineInputAsVariant.value<QQuick3DShaderUtilsTextureInput *>();
            QQuick3DTexture *splineTexture = splineInput->texture();
            QQuick3DTextureData *splineData = splineTexture->textureData();

            bool loop = series->isSplineLooping();
            material->setProperty("tension", series->splineTension());
            material->setProperty("knotting", series->splineKnotting());
            material->setProperty("loop", loop);
            material->setProperty("color", series->splineColor());

            const QScatterDataArray &array = series->dataArray();
            qsizetype pointCount = array.size();
            if (isDataDirty() && array.size() != 0) {
                QList<QVector4D> splinePoints;
                QList<SplineVertex> vertices;
                splinePoints.reserve(pointCount + 2);
                splineData->setSize(QSize(pointCount + 2, 1));

                QValue3DAxis *valueAxisX = series->axisX();
                QValue3DAxis *valueAxisY = series->axisY();
                QValue3DAxis *valueAxisZ = series->axisZ();
                if (!valueAxisX)
                    valueAxisX = static_cast<QValue3DAxis *>(axisX());
                if (!valueAxisY)
                    valueAxisY = static_cast<QValue3DAxis *>(axisY());
                if (!valueAxisZ)
                    valueAxisZ = static_cast<QValue3DAxis *>(axisZ());

                auto normalizedPos = [this, valueAxisX, valueAxisY, valueAxisZ](QVector3D pos) {
                    float posX = valueAxisX->positionAt(pos.x())
                                     * scale().x()
                                 + translate().x();
                    float posY = valueAxisY->positionAt(pos.y())
                                     * scale().y()
                                 + translate().y();
                    float posZ = valueAxisZ->positionAt(pos.z())
                                     * scale().z()
                                 + translate().z();
                    return QVector3D(posX, posY, posZ);
                };

                QVector3D first = normalizedPos(array.at(0).position());
                QVector3D second = normalizedPos(array.at(1).position());
                QVector3D pStart = first + (first - second) * 0.1f;
                QVector3D last = normalizedPos(array.at(pointCount - 1).position());
                QVector3D secondLast = normalizedPos(array.at(pointCount - 2).position());
                QVector3D pEnd = last + (last - secondLast) * 0.1f;

                if (loop)
                    splinePoints.append(QVector4D(last, 1));
                else
                    splinePoints.append(QVector4D(pStart, 1));

                const qsizetype resolution = series->splineResolution();
                vertices.reserve(resolution * pointCount);
                for (int i = 0; i < pointCount; i++) {
                    splinePoints.push_back(QVector4D(normalizedPos(array.at(i).position()), 1));
                    for (int j = 0; j < resolution; j++) {
                        SplineVertex vertex;
                        vertex.position = QVector3D(float(j) / float(resolution), float(i), 0);
                        vertex.uv = QVector2D(float(j) / float(resolution - 1),
                                              float(i) / float(pointCount + 2));
                        vertices.push_back(vertex);
                    }
                }
                if (loop)
                    splinePoints.append(QVector4D(first, 1));
                else
                    splinePoints.append(QVector4D(pEnd, 1));

                QByteArray pointData = QByteArray(reinterpret_cast<char *>(splinePoints.data()),
                                                  splinePoints.size() * sizeof(QVector4D));

                splineData->setTextureData(pointData);
                material->setProperty("points", splinePoints.size());
                QQuick3DGeometry *splineGeometry = model->splineModel->geometry();
                QByteArray vertexBuffer(reinterpret_cast<char *>(vertices.data()),
                                        vertices.size() * sizeof(SplineVertex));
                splineGeometry->setVertexData(vertexBuffer);
                splineGeometry->update();
                splineTexture->setTextureData(splineData);
                splineInput->setTexture(splineTexture);
            }
            model->splineModel->setVisible(true);
        }
    }
}

void QQuickGraphsScatter::createSplineModel(ScatterModel *model)
{
    QQuick3DModel *splineModel = new QQuick3DModel();
    splineModel->setParent(model->series);
    splineModel->setParentItem(graphNode());
    splineModel->setObjectName(QStringLiteral("SplineModel"));
    splineModel->setVisible(true);
    splineModel->setPickable(false);
    auto geometry = new QQuick3DGeometry();
    geometry->setParent(splineModel);
    geometry->setStride(sizeof(SplineVertex)); //pos + uv
    geometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::LineStrip);
    geometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                           0,
                           QQuick3DGeometry::Attribute::F32Type);
    geometry->addAttribute(QQuick3DGeometry::Attribute::TexCoord0Semantic,
                           sizeof(QVector3D),
                           QQuick3DGeometry::Attribute::F32Type);
    splineModel->setGeometry(geometry);

    QQuick3DTexture *splineTex = new QQuick3DTexture();
    splineTex->setHorizontalTiling(QQuick3DTexture::ClampToEdge);
    splineTex->setVerticalTiling(QQuick3DTexture::ClampToEdge);
    splineTex->setMinFilter(QQuick3DTexture::Nearest);
    splineTex->setMagFilter(QQuick3DTexture::Nearest);
    QQuick3DTextureData *splineData = new QQuick3DTextureData;
    splineData->setSize(QSize(0, 1));
    splineData->setFormat(QQuick3DTextureData::RGBA32F);
    splineData->setParent(splineTex);
    splineData->setParentItem(splineTex);
    splineTex->setTextureData(splineData);

    QQmlListReference materialRef(splineModel, "materials");
    QQuick3DCustomMaterial *material = createQmlCustomMaterial(
        QStringLiteral(":/materials/SplineMaterial"));
    material->setParent(splineModel);
    material->setParentItem(splineModel);
    material->setObjectName("splineMaterial");
    QVariant textureInputAsVariant = material->property("controlPoints");
    QQuick3DShaderUtilsTextureInput *textureInput = textureInputAsVariant
                                                        .value<QQuick3DShaderUtilsTextureInput *>();
    textureInput->setTexture(splineTex);
    splineTex->setParent(material);
    materialRef.append(material);

    model->splineModel = splineModel;

    if (auto series = qobject_cast<QSpline3DSeries *>(model->series)) {
        connect(series,
                &QSpline3DSeries::splineTensionChanged,
                this,
                &QQuickGraphsScatter::handleSplineChanged);
        connect(series,
                &QSpline3DSeries::splineKnottingChanged,
                this,
                &QQuickGraphsScatter::handleSplineChanged);
        connect(series,
                &QSpline3DSeries::splineLoopingChanged,
                this,
                &QQuickGraphsScatter::handleSplineChanged);
        connect(series,
                &QSpline3DSeries::splineColorChanged,
                this,
                &QQuickGraphsScatter::handleSplineChanged);
        connect(series,
                &QSpline3DSeries::splineResolutionChanged,
                this,
                &QQuickGraphsScatter::handleSplineChanged);
    }
}

void QQuickGraphsScatter::handleSplineChanged()
{
    m_isDataDirty = true;
}

QQuick3DModel *QQuickGraphsScatter::selected() const
{
    return m_selected;
}

void QQuickGraphsScatter::setSelected(QQuick3DModel *newSelected)
{
    if (newSelected != m_selected) {
        m_previousSelected = m_selected;
        m_selected = newSelected;

        auto series = static_cast<QScatter3DSeries *>(m_selected->parent());

        // Find scattermodel
        ScatterModel *graphModel = nullptr;

        for (const auto &model : std::as_const(m_scatterGraphs)) {
            if (model->series == series) {
                graphModel = model;
                break;
            }
        }

        if (graphModel) {
            qsizetype index = graphModel->dataItems.indexOf(m_selected);
            setSelectedItem(index, series);
            setSeriesVisualsDirty();
            setSelectedItemChanged(true);
        }
    }
}

void QQuickGraphsScatter::setSelected(QQuick3DModel *root, qsizetype index)
{
    auto series = static_cast<QScatter3DSeries *>(root->parent());
    if (index != m_selectedItem || series != m_selectedItemSeries) {
        setSeriesVisualsDirty();
        setSelectedItem(index, series);
        setSelectedItemChanged(true);
    }
}

void QQuickGraphsScatter::clearSelectionModel()
{
    if (optimizationHint() == QtGraphs3D::OptimizationHint::Default)
        clearAllSelectionInstanced();

    setSelectedItem(invalidSelectionIndex(), nullptr);

    if (itemLabel())
        itemLabel()->setVisible(false);

    setSeriesVisualsDirty();
    m_selected = nullptr;
    m_previousSelected = nullptr;
}

void QQuickGraphsScatter::clearAllSelectionInstanced()
{
    for (const auto &graph : std::as_const(m_scatterGraphs)) {
        if (graph->instancing)
            graph->instancing->resetVisibilty();
    }
}

void QQuickGraphsScatter::handleItemLabelVisibleChangedBySender(bool visible, QObject *sender)
{
    auto series = qobject_cast<QScatter3DSeries *>(sender);
    if (series && series == m_selectedItemSeries)
        itemLabel()->setVisible(visible);
}

void QQuickGraphsScatter::optimizationChanged(QtGraphs3D::OptimizationHint toOptimization)
{
    if (toOptimization == QtGraphs3D::OptimizationHint::Default) {
        for (const auto &graph : std::as_const(m_scatterGraphs))
            removeDataItems(graph, QtGraphs3D::OptimizationHint::Legacy);
    } else {
        for (const auto &graph : std::as_const(m_scatterGraphs))
            removeDataItems(graph, QtGraphs3D::OptimizationHint::Default);
    }
    setSeriesVisualsDirty();
}

void QQuickGraphsScatter::updateGraph()
{
    updatePointScaleSize();
    if (m_optimizationChanged) {
        optimizationChanged(optimizationHint());
        m_optimizationChanged = false;
    }

    for (auto graphModel : std::as_const(m_scatterGraphs)) {
        bool seriesVisible = graphModel->series->isVisible();
        if (isDataDirty()) {
            if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy && seriesVisible) {
                if (graphModel->dataItems.count() != graphModel->series->dataProxy()->itemCount()) {
                    Q_TRACE(QGraphs3DScatterLegacyModelsCountUpdate_entry);
                    qsizetype sizeDiff = sizeDifference(graphModel->dataItems.count(),
                                                        graphModel->series->dataProxy()->itemCount());

                    if (sizeDiff > 0)
                        addPointsToScatterModel(graphModel, sizeDiff);
                    else
                        removeDataItems(graphModel->dataItems, qAbs(sizeDiff));
                    Q_TRACE(QGraphs3DScatterLegacyModelsCountUpdate_exit);
                }
            } else if (optimizationHint() == QtGraphs3D::OptimizationHint::Default
                       && seriesVisible) {
                if (graphModel->instancing == nullptr) {
                    graphModel->instancing = new ScatterInstancing;
                    graphModel->instancing->setParent(graphModel->series);
                    qCDebug(lcGraphs3D) << "scatter info: "
                        << "\n series:" << graphModel->series
                        << "\n dataArray size:" << graphModel->series->dataArray().size()
                        << "\n basecolor:" << graphModel->series->baseColor()
                        << "\n customData:" << graphModel->instancing->customData()
                        << "\n mesh:" << graphModel->series->mesh();
                }
                if (graphModel->instancingRootItem == nullptr) {
                    graphModel->instancingRootItem = createDataItem(graphModel->series);
                    graphModel->instancingRootItem->setParent(graphModel->series);
                    graphModel->instancingRootItem->setInstancing(graphModel->instancing);
                    if (selectionMode() != QtGraphs3D::SelectionFlag::None) {
                        graphModel->instancingRootItem->setPickable(true);
                        graphModel->selectionIndicator = createDataItem(graphModel->series);
                        graphModel->selectionIndicator->setVisible(false);
                    }
                }
            }
        }

        if (seriesVisible && (isDataDirty() || isSeriesVisualsDirty())) {
            updateScatterGraphItemPositions(graphModel);
            updateSpline(graphModel);
        }

        if (seriesVisible
            && (isSeriesVisualsDirty()
                || (graphModel->instancing && graphModel->instancing->isDirty()))) {
            updateScatterGraphItemVisuals(graphModel);
        }

        const bool validSelection = (m_selectedItemSeries == graphModel->series
                                     && m_selectedItem != invalidSelectionIndex())
                                    && selectedItemInRange(graphModel);

        if (validSelection) {
            QVector3D selectionPosition = {0.0f, 0.0f, 0.0f};
            if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy) {
                QQuick3DModel *selectedModel = graphModel->dataItems.at(m_selectedItem);

                selectionPosition = selectedModel->position();
            } else {
                selectionPosition = graphModel->instancing->dataArray().at(m_selectedItem).position;
            }

            updateItemLabel(selectionPosition);
            QString label = m_selectedItemSeries->itemLabel();
            itemLabel()->setProperty("labelText", label);
            if (!label.compare(QString(hiddenLabelTag)))
                itemLabel()->setVisible(false);
        }
    }

    if (m_selectedItem == invalidSelectionIndex())
        itemLabel()->setVisible(false);

    setItemSelected(m_selectedItem != invalidSelectionIndex());
}

void QQuickGraphsScatter::synchData()
{
    qCDebug(lcGraphs3D, "%s start syncing", qUtf8Printable(QLatin1String(__FUNCTION__)));
    QList<QScatter3DSeries *> seriesList = scatterSeriesList();

    Q_TRACE(QGraphs3DScatterPointDataUpdate_entry);
    float maxItemSize = 0.0f;
    for (const auto &series : std::as_const(seriesList)) {
        if (series->isVisible()) {
            float itemSize = series->itemSize();
            if (itemSize > maxItemSize)
                maxItemSize = itemSize;
        }
    }

    m_maxItemSize = maxItemSize;

    updatePointScaleSize();
    Q_TRACE(QGraphs3DScatterPointDataUpdate_exit);
    QQuickGraphsItem::synchData();

    m_pointScale = calculatePointScaleSize();

    Q_TRACE(QGraphs3DScatterPointSelectionUpdate_entry);

    if (hasSelectedItemChanged()) {
        if (m_selectedItem != invalidSelectionIndex()) {
            QString itemLabelText = m_selectedItemSeries->itemLabel();
            itemLabel()->setProperty("labelText", itemLabelText);
            if (!itemLabelText.compare(QString(hiddenLabelTag)))
                itemLabel()->setVisible(false);
        }
        setSelectedItemChanged(false);
    }
    Q_TRACE(QGraphs3DScatterPointSelectionUpdate_exit);
    qCDebug(lcGraphs3D, "%s end syncing", qUtf8Printable(QLatin1String(__FUNCTION__)));
}

void QQuickGraphsScatter::cameraRotationChanged()
{
    m_isDataDirty = true;
}

void QQuickGraphsScatter::handleOptimizationHintChange(QtGraphs3D::OptimizationHint hint)
{
    Q_UNUSED(hint)
    m_optimizationChanged = true;
}

void QQuickGraphsScatter::handleSeriesVisibilityChangedBySender(QObject *sender)
{
    QQuickGraphsItem::handleSeriesVisibilityChangedBySender(sender);
    auto series = static_cast<QScatter3DSeries *>(sender);
    if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
        ScatterModel *graphModel = findGraphModel(series);
        if (graphModel && graphModel->series == series) {
            if (graphModel->instancingRootItem)
                graphModel->instancingRootItem->setVisible(series->isVisible());
            if (series == m_selectedItemSeries) {
                itemLabel()->setVisible(series->isVisible());
                graphModel->selectionIndicator->setVisible(series->isVisible());
            }
        }
    }
}

bool QQuickGraphsScatter::selectedItemInRange(const ScatterModel *graphModel)
{
    qsizetype itemCount;
    if (optimizationHint() == QtGraphs3D::OptimizationHint::Default)
        itemCount = graphModel->instancing->dataArray().count();
    else
        itemCount = graphModel->dataItems.count();

    return m_selectedItem >= 0 && m_selectedItem < itemCount;
}

QQuickGraphsScatter::ScatterModel *QQuickGraphsScatter::findGraphModel(QScatter3DSeries *series)
{
    for (const auto &graphModel : std::as_const(m_scatterGraphs)) {
        if (graphModel->series == series)
            return graphModel;
    }
    return nullptr;
}

QT_END_NAMESPACE

#include "moc_qquickgraphsscatter_p.cpp"
