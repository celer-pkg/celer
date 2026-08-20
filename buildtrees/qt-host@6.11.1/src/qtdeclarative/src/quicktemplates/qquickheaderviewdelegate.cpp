// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qquickheaderviewdelegate_p.h"

#include <QtQuickTemplates2/private/qquickheaderview_p.h>
#include <QtQuickTemplates2/private/qquickheaderview_p_p.h>
#include <QtQuickTemplates2/private/qquicktableviewdelegate_p_p.h>

/*!
    \qmltype HorizontalHeaderViewDelegate
    \inherits TableViewDelegate
    \inqmlmodule QtQuick.Controls
    \since 6.10
    \ingroup qtquickcontrols-delegates

    \image qtquickcontrols-headerviewdelegate.png
           {Header view delegate in table column header}

    The HorizontalHeaderViewDelegate serves as the default delegate
    automatically assigned to the \l HorizontalHeaderView's
    \l {TableView::delegate} {delegate property}.
    This delegate handles rendering every header cell using the
    application's predefined style specifications.

    HorizontalHeaderViewDelegate inherits TableViewDelegate, which means
    that it's composed of two items:
    a \l[QML]{Control::}{background} and
    a \l [QML]{Control::}{contentItem}.

    \sa {Customizing HeaderViewDelegate}, {HorizontalHeaderView}
*/

/*!
    \qmltype VerticalHeaderViewDelegate
    \inherits TableViewDelegate
    \inqmlmodule QtQuick.Controls
    \since 6.10
    \ingroup qtquickcontrols-delegates

    \image qtquickcontrols-headerviewdelegate.png
           {Header view delegate in table column header}

     The VerticalHeaderViewDelegate serves as the default delegate
     automatically assigned to the \l VerticalHeaderView's
     \l {TableView::delegate} {delegate property}.
     This delegate handles rendering every header cell using the
     application's predefined style specifications.

     VerticalHeaderViewDelegate inherits TableViewDelegate, which means
     that it's composed of two items:
     a \l[QML]{Control::}{background} and
     a \l [QML]{Control::}{contentItem}.

     \sa {Customizing HeaderViewDelegate}, {VerticalHeaderView}
*/

/*!
    \qmlproperty HeaderView QtQuick.Controls::HorizontalHeaderViewDelegate::headerView

    This property points to the \l HorizontalHeaderView that contains the delegate item.
*/

/*!
    \qmlproperty HeaderView QtQuick.Controls::VerticalHeaderViewDelegate::headerView

    This property points to the \l VerticalHeaderView that contains the delegate item.
*/

/*!
    \qmlproperty Qt::Orientations QtQuick.Controls::HorizontalHeaderViewDelegate::orientation

    This property has the same value of the headerView's orientation.
*/

/*!
    \qmlproperty Qt::Orientations QtQuick.Controls::VerticalHeaderViewDelegate::orientation

    This property has the same value of the headerView's orientation.
*/


QT_BEGIN_NAMESPACE

class QQuickHeaderViewDelegatePrivate : public QQuickTableViewDelegatePrivate
{
public:
    Q_DECLARE_PUBLIC(QQuickHeaderViewDelegate)

public:
    QPointer<QQuickHeaderViewBase> headerView;
    QVariant model;
};

QQuickHeaderViewDelegate::QQuickHeaderViewDelegate(QQuickItem *parent)
    : QQuickTableViewDelegate(*(new QQuickHeaderViewDelegatePrivate), parent)
{
}

QQuickHeaderViewBase *QQuickHeaderViewDelegate::headerView() const
{
    return d_func()->headerView;
}

Qt::Orientation QQuickHeaderViewDelegate::orientation() const
{
    return headerView()
            ? QQuickHeaderViewBasePrivate::get(headerView())->orientation()
            : Qt::Horizontal;
}

void QQuickHeaderViewDelegate::setHeaderView(QQuickHeaderViewBase *headerView)
{
    Q_D(QQuickHeaderViewDelegate);
    if (d->headerView == headerView)
        return;

    const Qt::Orientation oldOrientation = orientation();

    d->headerView = headerView;
    emit headerViewChanged();

    if (oldOrientation != orientation())
        emit orientationChanged();
}

QVariant QQuickHeaderViewDelegate::model() const
{
    return d_func()->model;
}

void QQuickHeaderViewDelegate::setModel(const QVariant &model)
{
    Q_D(QQuickHeaderViewDelegate);
    if (d->model == model)
        return;
    d->model = model;
    emit modelChanged();
}

QT_END_NAMESPACE

#include "moc_qquickheaderviewdelegate_p.cpp"
