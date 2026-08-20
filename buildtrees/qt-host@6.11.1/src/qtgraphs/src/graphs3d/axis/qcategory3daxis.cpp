// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcategory3daxis_p.h"
#include "qquickgraphsitem_p.h"
#include "qgraphs3dlogging_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \class QCategory3DAxis
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief The QCategory3DAxis class manipulates an axis of a graph.
 *
 * QCategory3DAxis provides an axis that can be given labels. The axis is
 * divided into equal-sized categories based on the data window size defined by
 * setting the axis range.
 *
 * Grid lines are drawn between categories, if visible. Labels are drawn to
 * positions of categories if provided.
 */

/*!
 * \qmltype Category3DAxis
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \nativetype QCategory3DAxis
 * \inherits Abstract3DAxis
 * \brief Manipulates an axis of a graph.
 *
 * This type provides an axis that can be given labels.
 */

/*!
 * \qmlproperty list Category3DAxis::labels
 *
 * The labels for the axis are applied to categories. If there are fewer labels
 * than categories, the remaining ones do not have a label. If category labels
 * are not defined explicitly, labels are generated from the data row (or column)
 * labels of the primary series of the graph.
 */

/*!
 * \qmlsignal Category3DAxis::rowLabelsChanged()
 *
 * This signal is emitted when row \l labels change.
 */

/*!
 * \qmlsignal Category3DAxis::columnLabelsChanged()
 *
 * This signal is emitted when column \l labels change.
 */

/*!
 * Constructs a category 3D axis with the parent \a parent.
 */
QCategory3DAxis::QCategory3DAxis(QObject *parent)
    : QAbstract3DAxis(*(new QCategory3DAxisPrivate()), parent)
{}

/*!
 * Destroys the category 3D axis.
 */
QCategory3DAxis::~QCategory3DAxis() {}

/*!
 * \property QCategory3DAxis::labels
 *
 * \brief The labels for the axis applied to categories.
 *
 * If there are fewer labels than categories, the
 * remaining ones do not have a label. If category labels are not defined
 * explicitly, labels are generated from the data row (or column) labels of the
 * primary series of the graph.
 */
QStringList QCategory3DAxis::labels() const
{
    return QAbstract3DAxis::labels();
}

void QCategory3DAxis::setLabels(const QStringList &labels)
{
    Q_D(QCategory3DAxis);
    d->m_labelsExplicitlySet = !labels.isEmpty();
    bool labelsFromData = false;

    // Get labels from data proxy if axis is attached to a bar graph and an active
    // axis there
    if (labels.isEmpty()) {
        QQuickGraphsItem *graph = qobject_cast<QQuickGraphsItem *>(parent());
        // TODO: QCategory3DAxis is only used with QBars3D, so is it still necessary to check if
        // the item is bars or something else?
        if (graph) {
            if (graph->axisX() == this) {
                emit rowLabelsChanged();
                labelsFromData = true;
            } else if (graph->axisZ() == this) {
                emit columnLabelsChanged();
                labelsFromData = true;
            }
        }
    }

    if (!labelsFromData && d->m_labels != labels) {
        d->m_labels = labels;
        emit QAbstract3DAxis::labelsChanged();
    }
}

/*!
 * \internal
 */
QCategory3DAxisPrivate::QCategory3DAxisPrivate()
    : QAbstract3DAxisPrivate(QAbstract3DAxis::AxisType::Category)
    , m_labelsExplicitlySet(false)
{}

QCategory3DAxisPrivate::~QCategory3DAxisPrivate() {}

/*!
 * \internal
 * The graph uses this function to set labels from the data proxy as category
 * labels.
 * If the labels have been set explicitly by the user, data proxy labels
 * are not used.
 */
void QCategory3DAxisPrivate::setDataLabels(const QStringList &labels)
{
    Q_Q(QCategory3DAxis);
    if (m_labelsExplicitlySet || m_labels == labels) {
        qCDebug(lcAProperties3D) << __FUNCTION__
            << "value is already set to:" << labels;
        return;
    }

    m_labels = labels;
    emit q->QAbstract3DAxis::labelsChanged();
}

bool QCategory3DAxisPrivate::allowZero()
{
    return true;
}

bool QCategory3DAxisPrivate::allowNegatives()
{
    return false;
}

bool QCategory3DAxisPrivate::allowMinMaxSame()
{
    return true;
}

QT_END_NAMESPACE

#include "moc_qcategory3daxis.cpp"
