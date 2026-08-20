// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qabstract3dseries_p.h"
#include "qabstractdataproxy_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \class QAbstractDataProxy
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief The QAbstractDataProxy class is a base class for all 3D graph proxies.
 *
 * The following graphs type specific inherited classes are used instead
 * of the base class: QBarDataProxy, QScatterDataProxy, and QSurfaceDataProxy.
 *
 * For more information, see \l{Qt Graphs Data Handling with 3D}.
 */

/*!
 * \qmltype AbstractDataProxy
 * \qmlabstract
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \nativetype QAbstractDataProxy
 * \brief Base type for all 3D graph data proxies.
 *
 * This abstract class serves as a base class for the following subtypes:
 * BarDataProxy, ScatterDataProxy, SurfaceDataProxy.
 *
 * For more information, see \l {Qt Graphs Data Handling with 3D}.
 */

/*!
 * \qmlproperty AbstractDataProxy.DataType AbstractDataProxy::type
 * The type of the proxy. One of the QAbstractDataProxy::DataType values.
 */

/*!
 * \enum QAbstractDataProxy::DataType
 *
 * This enum type specifies the data type of the proxy.
 *
 * \value None
 *        No data type.
 * \value Bar
 *        Data type for Q3DBarsWidgetItem.
 * \value Scatter
 *        Data type for Q3DScatterWidgetItem.
 * \value Surface
 *        Data type for Q3DSurfaceWidgetItem.
 */

/*!
 * \internal
 */
QAbstractDataProxy::QAbstractDataProxy(QAbstractDataProxyPrivate &d, QObject *parent)
    : QObject(d, parent)
{}

/*!
 * Deletes the abstract data proxy.
 */
QAbstractDataProxy::~QAbstractDataProxy() {}

/*!
 * \property QAbstractDataProxy::type
 *
 * \brief The data type of the proxy.
 */
QAbstractDataProxy::DataType QAbstractDataProxy::type() const
{
    Q_D(const QAbstractDataProxy);
    return d->m_type;
}

// QAbstractDataProxyPrivate

QAbstractDataProxyPrivate::QAbstractDataProxyPrivate(QAbstractDataProxy::DataType type)
    : m_type(type)
    , m_series(0)
{}

QAbstractDataProxyPrivate::~QAbstractDataProxyPrivate() {}

void QAbstractDataProxyPrivate::setSeries(QAbstract3DSeries *series)
{
    Q_Q(QAbstractDataProxy);
    q->setParent(series);
    m_series = series;
}

QT_END_NAMESPACE

#include "moc_qabstractdataproxy.cpp"
