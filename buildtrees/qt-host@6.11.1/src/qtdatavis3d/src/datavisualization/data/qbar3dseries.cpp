// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qbar3dseries_p.h"
#include "bars3dcontroller_p.h"

#include <QtGui/qquaternion.h>

#include <QtCore/qmath.h>

QT_BEGIN_NAMESPACE

/*!
 * \class QBar3DSeries
 * \inmodule QtDataVisualization
 * \brief The QBar3DSeries class represents a data series in a 3D bar graph.
 * \since QtDataVisualization 1.0
 *
 * This class manages the series specific visual elements, as well as the series
 * data (via a data proxy).
 *
 * If no data proxy is set explicitly for the series, the series creates a default
 * proxy. Setting another proxy will destroy the existing proxy and all data added to it.
 *
 * QBar3DSeries supports the following format tags for QAbstract3DSeries::setItemLabelFormat():
 * \table
 *   \row
 *     \li @rowTitle      \li Title from row axis
 *   \row
 *     \li @colTitle      \li Title from column axis
 *   \row
 *     \li @valueTitle    \li Title from value axis
 *   \row
 *     \li @rowIdx        \li Visible row index. Localized using the graph locale.
 *   \row
 *     \li @colIdx        \li Visible column index. Localized using the graph locale.
 *   \row
 *     \li @rowLabel      \li Label from row axis
 *   \row
 *     \li @colLabel      \li Label from column axis
 *   \row
 *     \li @valueLabel    \li Item value formatted using the format of the value
 *                            axis attached to the graph. For more information,
 *                            see \l{QValue3DAxis::labelFormat}.
 *   \row
 *     \li @seriesName    \li Name of the series
 *   \row
 *     \li %<format spec> \li Item value in the specified format. Formatted
 *                            using the same rules as \l{QValue3DAxis::labelFormat}.
 * \endtable
 *
 * For example:
 * \snippet doc_src_qtdatavisualization.cpp 1
 *
 * \sa {Qt Data Visualization Data Handling}, QAbstract3DGraph::locale
 */

/*!
 * \qmltype Bar3DSeries
 * \inqmlmodule QtDataVisualization
 * \since QtDataVisualization 1.0
 * \ingroup datavisualization_qml
 * \nativetype QBar3DSeries
 * \inherits Abstract3DSeries
 * \brief Represents a data series in a 3D bar graph.
 *
 * This type manages the series specific visual elements, as well as the series
 * data (via a data proxy).
 *
 * For a more complete description, see QBar3DSeries.
 *
 * \sa {Qt Data Visualization Data Handling}
 */

/*!
 * \qmlproperty BarDataProxy Bar3DSeries::dataProxy
 *
 * The active data proxy. The series assumes ownership of any proxy set to
 * it and deletes any previously set proxy when a new one is added. The proxy cannot be null or
 * set to another series.
 */

/*!
 * \qmlproperty point Bar3DSeries::selectedBar
 *
 * The bar in the series that is selected.
 *
 * The position of the selected bar is specified as a row and column in the
 * data array of the series.
 *
 * Only one bar can be selected at a time.
 *
 * To clear selection from this series, set invalidSelectionPosition as the position.
 *
 * If this series is added to a graph, the graph can adjust the selection according to user
 * interaction or if it becomes invalid. Selecting a bar on another added series will also
 * clear the selection.
 *
 * Removing rows from or inserting rows to the series before the row of the selected bar
 * will adjust the selection so that the same bar will stay selected.
 *
 * \sa {AbstractGraph3D::clearSelection()}{AbstractGraph3D.clearSelection()}
 */

/*!
 * \qmlproperty point Bar3DSeries::invalidSelectionPosition
 * A constant property providing an invalid position for selection. This
 * position is set to the selectedBar property to clear the selection from this
 * series.
 *
 * \sa {AbstractGraph3D::clearSelection()}{AbstractGraph3D.clearSelection()}
 */

/*!
 * \qmlproperty real Bar3DSeries::meshAngle
 *
 * A convenience property for defining the series rotation angle in degrees.
 *
 * \note When reading this property, it is calculated from the
 * \l{Abstract3DSeries::meshRotation}{Abstract3DSeries.meshRotation} value
 * using floating point precision and always returns a value from zero to 360 degrees.
 *
 * \sa {Abstract3DSeries::meshRotation}{Abstract3DSeries.meshRotation}
 */

/*!
 * \qmlproperty list<ThemeColor> Bar3DSeries::rowColors
 * \since 6.3
 * This property can be used to draw the rows of the series in different colors.
 * The \l{Theme3D::colorStyle}{Theme3D.colorStyle} must be set to
 * \c ColorStyleUniform to use this property.
 * \note If the property is set and the theme is changed,
 * the rowColors list is not cleared automatically.
 *
 * \sa Q3DTheme::ColorStyleUniform
 */

/*!
 * Constructsa bar 3D series with the parent \a parent.
 */
QBar3DSeries::QBar3DSeries(QObject *parent) :
    QAbstract3DSeries(new QBar3DSeriesPrivate(this), parent)
{
    // Default proxy
    dptr()->setDataProxy(new QBarDataProxy);
    dptr()->connectSignals();
}

/*!
 * Constructs a bar 3D series with the data proxy \a dataProxy and the parent
 * \a parent.
 */
QBar3DSeries::QBar3DSeries(QBarDataProxy *dataProxy, QObject *parent) :
    QAbstract3DSeries(new QBar3DSeriesPrivate(this), parent)
{
    dptr()->setDataProxy(dataProxy);
    dptr()->connectSignals();
}

/*!
 * Deletes a bar 3D series.
 */
QBar3DSeries::~QBar3DSeries()
{
}

/*!
 * \property QBar3DSeries::dataProxy
 *
 * \brief The active data proxy.
 *
 * The series assumes ownership of any proxy set to it and deletes any
 * previously set proxy when a new one is added. The proxy cannot be null or
 * set to another series.
 */
void QBar3DSeries::setDataProxy(QBarDataProxy *proxy)
{
    d_ptr->setDataProxy(proxy);
}

QBarDataProxy *QBar3DSeries::dataProxy() const
{
    return static_cast<QBarDataProxy *>(d_ptr->dataProxy());
}

/*!
 * \property QBar3DSeries::selectedBar
 *
 * \brief The bar in the series that is selected.
 *
 */

/*!
 * Selects the bar at the \a position position, specified as a row and column in
 * the data array of the series.
 *
 * Only one bar can be selected at a time.
 *
 * To clear selection from this series, invalidSelectionPosition() is set as
 * \a position.
 *
 * If this series is added to a graph, the graph can adjust the selection according to user
 * interaction or if it becomes invalid. Selecting a bar on another added series will also
 * clear the selection.
 *
 * Removing rows from or inserting rows to the series before the row of the selected bar
 * will adjust the selection so that the same bar will stay selected.
 *
 * \sa QAbstract3DGraph::clearSelection()
 */
void QBar3DSeries::setSelectedBar(const QPoint &position)
{
    // Don't do this in private to avoid loops, as that is used for callback from controller.
    if (d_ptr->m_controller)
        static_cast<Bars3DController *>(d_ptr->m_controller)->setSelectedBar(position, this, true);
    else
        dptr()->setSelectedBar(position);
}

QPoint QBar3DSeries::selectedBar() const
{
    return dptrc()->m_selectedBar;
}

/*!
 * Returns an invalid position for selection. This position is set to the
 * selectedBar property to clear the selection from this series.
 *
 * \sa QAbstract3DGraph::clearSelection()
 */
QPoint QBar3DSeries::invalidSelectionPosition()
{
    return Bars3DController::invalidSelectionPosition();
}

static inline float quaternionAngle(const QQuaternion &rotation)
{
    return qRadiansToDegrees(qAcos(rotation.scalar())) * 2.f;
}

/*!
  \property QBar3DSeries::meshAngle

  \brief The series rotation angle in degrees.

  Setting this property is equivalent to the following call:

  \code
  setMeshRotation(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, angle))
  \endcode

  \note When reading this property, it is calculated from the
        QAbstract3DSeries::meshRotation value using floating point precision
        and always returns a value from zero to 360 degrees.

  \sa QAbstract3DSeries::meshRotation
 */
void QBar3DSeries::setMeshAngle(float angle)
{
    setMeshRotation(QQuaternion::fromAxisAndAngle(upVector, angle));
}

float QBar3DSeries::meshAngle() const
{
    QQuaternion rotation = meshRotation();

    if (rotation.isIdentity() || rotation.x() != 0.0f || rotation.z() != 0.0f)
        return 0.0f;
    else
        return quaternionAngle(rotation);
}

/*!
 * \property QBar3DSeries::rowColors
 * \since 6.3
 *
 * \brief The list of row colors in the series.
 *
 * This property can be used to color
 * the rows of the series in different colors.
 * The Q3DTheme::ColorStyle must be set to
 * Q3DTheme::ColorStyleUniform to use this property.
 *
 * \sa Q3DTheme::ColorStyleUniform
 */
void QBar3DSeries::setRowColors(const QList<QColor> &colors)
{
    dptr()->setRowColors(colors);
}
QList<QColor> QBar3DSeries::rowColors() const
{
    return dptrc()->m_rowColors;
}
/*!
 * \internal
 */
QBar3DSeriesPrivate *QBar3DSeries::dptr()
{
    return static_cast<QBar3DSeriesPrivate *>(d_ptr.data());
}

/*!
 * \internal
 */
const QBar3DSeriesPrivate *QBar3DSeries::dptrc() const
{
    return static_cast<const QBar3DSeriesPrivate *>(d_ptr.data());
}

// QBar3DSeriesPrivate

QBar3DSeriesPrivate::QBar3DSeriesPrivate(QBar3DSeries *q)
    : QAbstract3DSeriesPrivate(q, QAbstract3DSeries::SeriesTypeBar),
      m_selectedBar(Bars3DController::invalidSelectionPosition())
{
    m_itemLabelFormat = QStringLiteral("@valueLabel");
    m_mesh = QAbstract3DSeries::MeshBevelBar;
}

QBar3DSeriesPrivate::~QBar3DSeriesPrivate()
{
}

QBar3DSeries *QBar3DSeriesPrivate::qptr()
{
    return static_cast<QBar3DSeries *>(q_ptr);
}

void QBar3DSeriesPrivate::setDataProxy(QAbstractDataProxy *proxy)
{
    Q_ASSERT(proxy->type() == QAbstractDataProxy::DataTypeBar);

    QAbstract3DSeriesPrivate::setDataProxy(proxy);

    emit qptr()->dataProxyChanged(static_cast<QBarDataProxy *>(proxy));
}

void QBar3DSeriesPrivate::connectControllerAndProxy(Abstract3DController *newController)
{
    QBarDataProxy *barDataProxy = static_cast<QBarDataProxy *>(m_dataProxy);

    if (m_controller && barDataProxy) {
        // Disconnect old controller/old proxy
        QObject::disconnect(barDataProxy, 0, m_controller, 0);
        QObject::disconnect(q_ptr, 0, m_controller, 0);
    }

    if (newController && barDataProxy) {
        Bars3DController *controller = static_cast<Bars3DController *>(newController);
        QObject::connect(barDataProxy, &QBarDataProxy::arrayReset, controller,
                         &Bars3DController::handleArrayReset);
        QObject::connect(barDataProxy, &QBarDataProxy::rowsAdded, controller,
                         &Bars3DController::handleRowsAdded);
        QObject::connect(barDataProxy, &QBarDataProxy::rowsChanged, controller,
                         &Bars3DController::handleRowsChanged);
        QObject::connect(barDataProxy, &QBarDataProxy::rowsRemoved, controller,
                         &Bars3DController::handleRowsRemoved);
        QObject::connect(barDataProxy, &QBarDataProxy::rowsInserted, controller,
                         &Bars3DController::handleRowsInserted);
        QObject::connect(barDataProxy, &QBarDataProxy::itemChanged, controller,
                         &Bars3DController::handleItemChanged);
        QObject::connect(barDataProxy, &QBarDataProxy::rowLabelsChanged, controller,
                         &Bars3DController::handleDataRowLabelsChanged);
        QObject::connect(barDataProxy, &QBarDataProxy::columnLabelsChanged, controller,
                         &Bars3DController::handleDataColumnLabelsChanged);
        QObject::connect(qptr(), &QBar3DSeries::dataProxyChanged, controller,
                         &Bars3DController::handleArrayReset);
        QObject::connect(qptr(), &QBar3DSeries::rowColorsChanged, controller,
                         &Bars3DController::handleRowColorsChanged);
    }
}

void QBar3DSeriesPrivate::createItemLabel()
{
    static const QString rowIndexTag(QStringLiteral("@rowIdx"));
    static const QString rowLabelTag(QStringLiteral("@rowLabel"));
    static const QString rowTitleTag(QStringLiteral("@rowTitle"));
    static const QString colIndexTag(QStringLiteral("@colIdx"));
    static const QString colLabelTag(QStringLiteral("@colLabel"));
    static const QString colTitleTag(QStringLiteral("@colTitle"));
    static const QString valueTitleTag(QStringLiteral("@valueTitle"));
    static const QString valueLabelTag(QStringLiteral("@valueLabel"));
    static const QString seriesNameTag(QStringLiteral("@seriesName"));

    if (m_selectedBar == QBar3DSeries::invalidSelectionPosition()) {
        m_itemLabel = QString();
        return;
    }

    QLocale locale(QLocale::c());
    if (m_controller)
        locale = m_controller->locale();

    QCategory3DAxis *categoryAxisZ = static_cast<QCategory3DAxis *>(m_controller->axisZ());
    QCategory3DAxis *categoryAxisX = static_cast<QCategory3DAxis *>(m_controller->axisX());
    QValue3DAxis *valueAxis = static_cast<QValue3DAxis *>(m_controller->axisY());
    qreal selectedBarValue = qreal(qptr()->dataProxy()->itemAt(m_selectedBar)->value());

    // Custom format expects printf format specifier. There is no tag for it.
    m_itemLabel = valueAxis->formatter()->stringForValue(selectedBarValue, m_itemLabelFormat);

    int selBarPosRow = m_selectedBar.x();
    int selBarPosCol = m_selectedBar.y();
    m_itemLabel.replace(rowIndexTag, locale.toString(selBarPosRow));
    if (categoryAxisZ->labels().size() > selBarPosRow)
        m_itemLabel.replace(rowLabelTag, categoryAxisZ->labels().at(selBarPosRow));
    else
        m_itemLabel.replace(rowLabelTag, QString());
    m_itemLabel.replace(rowTitleTag, categoryAxisZ->title());
    m_itemLabel.replace(colIndexTag, locale.toString(selBarPosCol));
    if (categoryAxisX->labels().size() > selBarPosCol)
        m_itemLabel.replace(colLabelTag, categoryAxisX->labels().at(selBarPosCol));
    else
        m_itemLabel.replace(colLabelTag, QString());
    m_itemLabel.replace(colTitleTag, categoryAxisX->title());
    m_itemLabel.replace(valueTitleTag, valueAxis->title());

    if (m_itemLabel.contains(valueLabelTag)) {
        QString valueLabelText = valueAxis->formatter()->stringForValue(selectedBarValue,
                                                                        valueAxis->labelFormat());
        m_itemLabel.replace(valueLabelTag, valueLabelText);
    }

    m_itemLabel.replace(seriesNameTag, m_name);
}

void QBar3DSeriesPrivate::handleMeshRotationChanged(const QQuaternion &rotation)
{
    emit qptr()->meshAngleChanged(quaternionAngle(rotation));
}

void QBar3DSeriesPrivate::setSelectedBar(const QPoint &position)
{
    if (position != m_selectedBar) {
        markItemLabelDirty();
        m_selectedBar = position;
        emit qptr()->selectedBarChanged(m_selectedBar);
    }
}

void QBar3DSeriesPrivate::connectSignals()
{
    QObject::connect(q_ptr, &QAbstract3DSeries::meshRotationChanged, this,
                     &QBar3DSeriesPrivate::handleMeshRotationChanged);
}

void QBar3DSeriesPrivate::setRowColors(const QList<QColor> &colors)
{
    if (m_rowColors != colors) {
        m_rowColors = colors;
        emit qptr()->rowColorsChanged(m_rowColors);
    }
}

QT_END_NAMESPACE
