// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qquicktableviewdelegate_p.h"
#include "qquicktableviewdelegate_p_p.h"

#include <QtQuickTemplates2/private/qquickitemdelegate_p_p.h>
#include <QtQuick/private/qquicktaphandler_p.h>
#include <QtQuick/private/qquicktableview_p.h>

#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype TableViewDelegate
    \inherits ItemDelegate
    \inqmlmodule QtQuick.Controls
    \since 6.9
    \ingroup qtquickcontrols-delegates
    \brief A delegate that can be assigned to a TableView.

    \image qtquickcontrols-tableviewdelegate.png
           {Table view displaying addresses using a table view delegate}

    A TableViewDelegate is a delegate that can be assigned to the
    \l {TableView::delegate} {delegate property} of a \l TableView.
    It renders each cell of the table in the view using the application style.

    \code
    TableView {
        anchors.fill: parent
        delegate: TableViewDelegate {}
        // model: yourModel
    }
    \endcode

    TableViewDelegate inherits \l ItemDelegate, which means that it's composed of
    two items: a \l[QML]{Control::}{background} and a \l [QML]{Control::}{contentItem}.

    The position of the contentItem is controlled with \l [QML]{Control::}{padding}.

    \section2 Interacting with pointers
    TableViewDelegate inherits \l ItemDelegate. This means that it will emit signals such as
    \l {AbstractButton::clicked()}{clicked} when the user clicks on the delegate.
    You can connect to this signal to implement application-specific functionality.

    However, the ItemDelegate API does not give you information about the position of the
    click, or which modifiers are being held. If this is needed, a better approach would
    be to use pointer handlers, for example:

    \code
    TableView {
        id: tableView
        delegate: TableViewDelegate {
            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: someContextMenu.open()
            }

            TapHandler {
                acceptedModifiers: Qt.ControlModifier
                onTapped: tableView.doSomethingToCell(row, column)
            }
        }
    }
    \endcode

    \note If you want to disable the default behavior that occurs when the
    user clicks on the delegate (like changing the current index), you can set
    \l {TableView::pointerNavigationEnabled}{pointerNavigationEnabled} to \c false.

    \section2 Editing cells in the table
    TableViewDelegate has a default \l {TableView::editDelegate}{edit delegate} assigned.
    If \l TableView has \l {TableView::editTriggers}{edit triggers} set, and
    the \l {TableView::model}{model} has support for \l {Editing cells} {editing model items},
    then the user can activate any of the edit triggers to edit the text of
    the \l {TableViewDelegate::current}{current} table cell.

    The default edit delegate will use the \c {Qt.EditRole} to read and write data to the
    \l {TableView::model}{model}. If you need to use another role, or otherwise have needs
    outside what the default edit delegate offers, you can always assign your own delegate
    to \l {TableView::editDelegate}{TableView.editDelegate}.

    \sa {Customizing TableViewDelegate}, {TableView}
*/

/*!
    \qmlproperty TableView QtQuick.Controls::TableViewDelegate::tableView

    This property points to the \l TableView that contains the delegate item.
*/

/*!
    \qmlproperty bool QtQuick.Controls::TableViewDelegate::current

    This property holds whether or not the delegate represents the
    \l {QItemSelectionModel::currentIndex()}{current index}
    in the \l {TableView::selectionModel}{selection model}.
*/

/*!
    \qmlproperty bool QtQuick.Controls::TableViewDelegate::selected

    This property holds whether or not the delegate represents a
    \l {QItemSelectionModel::selection()}{selected index}
    in the \l {TableView::selectionModel}{selection model}.
*/

/*!
    \qmlproperty bool QtQuick.Controls::TableViewDelegate::editing

    This property holds whether or not the delegate is being \l {Editing cells}{edited}.
*/

using namespace Qt::Literals::StringLiterals;

QQuickTableViewDelegate::QQuickTableViewDelegate(QQuickItem *parent)
    : QQuickItemDelegate(*(new QQuickTableViewDelegatePrivate), parent)
{
    Q_D(QQuickTableViewDelegate);

    auto tapHandler = new QQuickTapHandler(this);
    tapHandler->setAcceptedModifiers(Qt::NoModifier);

    // Since we override mousePressEvent to avoid QQuickAbstractButton from blocking
    // pointer handlers, we inform the button about its pressed state from the tap
    // handler instead. This will ensure that we emit button signals like
    // pressed, clicked, and doubleClicked.
    connect(tapHandler, &QQuickTapHandler::pressedChanged, this, [this, d, tapHandler] {
        auto view = tableView();
        if (!view || !view->pointerNavigationEnabled())
            return;

        const QQuickHandlerPoint p = tapHandler->point();
        if (tapHandler->isPressed())
            d->handlePress(p.position(), 0);
        else if (tapHandler->tapCount() > 0)
            d->handleRelease(p.position(), 0);
        else
            d->handleUngrab();

        if (tapHandler->tapCount() > 1 && !tapHandler->isPressed())
            emit doubleClicked();
    }, Qt::DirectConnection);
}

QQuickTableViewDelegate::QQuickTableViewDelegate(QQuickTableViewDelegatePrivate &dd, QQuickItem *parent)
    : QQuickItemDelegate(dd, parent)
{
}

void QQuickTableViewDelegate::mousePressEvent(QMouseEvent *event)
{
    Q_D(QQuickTableViewDelegate);

    const auto view = d->m_tableView;
    if (view && view->pointerNavigationEnabled()) {
        // Ignore mouse events so that we don't block our own pointer handlers, or
        // pointer handlers in e.g TreeView, TableView, or SelectionRectangle. Instead
        // we call out to the needed mouse handling functions in QQuickAbstractButton directly
        // from our pointer handlers, to ensure that we continue to work as a button.
        event->ignore();
        return;
    }

    QQuickItemDelegate::mousePressEvent(event);
}

QPalette QQuickTableViewDelegatePrivate::defaultPalette() const
{
    return QQuickTheme::palette(QQuickTheme::ItemView);
}

QFont QQuickTableViewDelegate::defaultFont() const
{
    return QQuickTheme::font(QQuickTheme::ItemView);
}

bool QQuickTableViewDelegate::current() const
{
    return d_func()->current;
}

void QQuickTableViewDelegate::setCurrent(bool current)
{
    Q_D(QQuickTableViewDelegate);
    if (d->current == current)
        return;

    d->current = current;
    emit currentChanged();
}

bool QQuickTableViewDelegate::selected() const
{
    return d_func()->selected;
}

void QQuickTableViewDelegate::setSelected(bool selected)
{
    Q_D(QQuickTableViewDelegate);
    if (d->selected == selected)
        return;

    d->selected = selected;
    emit selectedChanged();
}

bool QQuickTableViewDelegate::editing() const
{
    return d_func()->editing;
}

void QQuickTableViewDelegate::setEditing(bool editing)
{
    Q_D(QQuickTableViewDelegate);
    if (d->editing == editing)
        return;

    d->editing = editing;
    emit editingChanged();
}

QQuickTableView *QQuickTableViewDelegate::tableView() const
{
    return d_func()->m_tableView;
}

void QQuickTableViewDelegate::setTableView(QQuickTableView *tableView)
{
    Q_D(QQuickTableViewDelegate);
    if (d->m_tableView == tableView)
        return;

    d->m_tableView = tableView;
    emit tableViewChanged();
}

QT_END_NAMESPACE

#include "moc_qquicktableviewdelegate_p.cpp"
