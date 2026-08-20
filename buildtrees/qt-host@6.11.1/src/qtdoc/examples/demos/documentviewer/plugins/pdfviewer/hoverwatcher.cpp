// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include "hoverwatcher.h"
#include <QGuiApplication>
#include <QWidget>
#include <QMouseEvent>

HoverWatcher::HoverWatcher(QWidget *watched)
    : QObject(watched), m_watched(watched)
{
    Q_ASSERT(watched);
    m_cursorShapes[Entered].emplace(Qt::OpenHandCursor);
    m_cursorShapes[MousePress].emplace(Qt::ClosedHandCursor);
    m_cursorShapes[MouseRelease].emplace(Qt::OpenHandCursor);
    // no default for Left => restore override cursor
    m_watched->installEventFilter(this);
}

HoverWatcher::~HoverWatcher()
{
    m_watched->removeEventFilter(this);
}

typedef QHash<QWidget *, HoverWatcher*> WatchMap;
Q_GLOBAL_STATIC(WatchMap, qt_allHoverWatchers)

HoverWatcher *HoverWatcher::watcher(QWidget *watched)
{
    if (qt_allHoverWatchers()->contains(watched))
        return qt_allHoverWatchers()->value(watched);

    HoverWatcher *watcher = new HoverWatcher(watched);
    qt_allHoverWatchers()->insert(watched, watcher);
    return watcher;
}

/*!
   \overload Const version of watcher
 */
const HoverWatcher *HoverWatcher::watcher(const QWidget *watched)
{
    return watcher(const_cast<QWidget *>(watched));
}

void HoverWatcher::dismiss(QWidget *watched)
{
    if (!hasWatcher(watched))
        return;

    delete qt_allHoverWatchers()->take(watched);
}

bool HoverWatcher::hasWatcher(QWidget *widget)
{
    return qt_allHoverWatchers()->contains(widget);
}

static constexpr HoverWatcher::HoverAction toHoverAction(QEvent::Type et)
{
    switch (et) {
    case QEvent::Type::Enter:
        return HoverWatcher::HoverAction::Entered;
    case QEvent::Type::Leave:
        return HoverWatcher::HoverAction::Left;
    case QEvent::Type::MouseButtonPress:
        return HoverWatcher::HoverAction::MousePress;
    case QEvent::Type::MouseButtonRelease:
        return HoverWatcher::HoverAction::MouseRelease;
    default:
        return HoverWatcher::HoverAction::Ignore;
    }
}

void HoverWatcher::handleAction (HoverWatcher::HoverAction action)
{
    const Qt::CursorShape newShape = cursorShape(action);
    if (QGuiApplication::overrideCursor()
            && (QGuiApplication::overrideCursor()->shape() == newShape
            || action == HoverAction::Ignore)) {
        return;
    }

    QGuiApplication::setOverrideCursor(cursorShape(action));
    emit hoverAction(action);

    switch (action) {
    case HoverAction::Entered:
        emit entered();
        break;
    case HoverAction::Left:
        emit left();
        break;
    case HoverAction::MousePress:
        emit mousePressed();
        break;
    case HoverAction::MouseRelease: {
        emit mouseReleased();
    }
        break;
    case HoverAction::Ignore:
        break;
    }
}

bool HoverWatcher::hasShape(HoverAction action) const
{
    return action != HoverAction::Ignore && m_cursorShapes[action].has_value();
}

void HoverWatcher::setApplicationCursor(HoverAction action) const
{
    if (!hasShape(action)) {
        QGuiApplication::restoreOverrideCursor();
        return;
    }

    QGuiApplication::setOverrideCursor(cursorShape(action));
}

bool HoverWatcher::eventFilter(QObject *obj, QEvent *event)
{
    Q_ASSERT(obj == m_watched); // don't install event filters elsewhere

    // Ignore irrelevant events
    const auto action = toHoverAction(event->type());
    if (action == HoverAction::Ignore)
        return false;

    // React to a QScroller having been installed or removed
    // A Scroller sends a fake mouse release to QPoint (-1, -1)
    // => needs to be ignored and end of scrolling processed instead
    static bool hasScroller = false;
    if (QScroller::hasScroller(m_watched) != hasScroller) {
        hasScroller = QScroller::hasScroller(m_watched);
        static QMetaObject::Connection con;
        if (hasScroller) {
            con = connect(QScroller::scroller(m_watched), &QScroller::stateChanged,
                          this, &HoverWatcher::handleScrollerStateChange);
        } else {
            disconnect(con);
        }
    }

    // Ignore fake mouse release event sent by scroller
    if (action == HoverAction::MouseRelease && hasScroller) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (me->pos().x() < -9000000 )
            return false;
    }

    // Ignore unpermitted mouse buttons
    if (action == HoverAction::MousePress) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (!m_mouseButtons.testFlag(me->button()))
            return false;
    }

    handleAction(action);
    return false;
}

Qt::CursorShape HoverWatcher::cursorShape(HoverAction type) const
{
    const Qt::CursorShape fallback = Qt::ArrowCursor;
    if (type == HoverAction::Ignore)
        return fallback;

    return m_cursorShapes[type].value_or(fallback);
}

void HoverWatcher::setCursorShape(HoverAction type, Qt::CursorShape shape)
{
    if (type == HoverAction::Ignore)
        return;
    m_cursorShapes[type].emplace(shape);
}

void HoverWatcher::unSetCursorShape(HoverAction type)
{
    if (type == HoverAction::Ignore)
        return;
    m_cursorShapes[type].reset();
}

void HoverWatcher::setMouseButtons(Qt::MouseButtons buttons)
{
    m_mouseButtons = buttons;
}

void HoverWatcher::setMouseButton(Qt::MouseButton button, bool enable)
{
    m_mouseButtons.setFlag(button, enable);;
}

/*!
   \brief This slot handles a QScroller state change, in case the watched
    widget uses a scroller. It translates \param state into the appropriate
    action.
 */
void HoverWatcher::handleScrollerStateChange(QScroller::State state)
{
    switch (state) {
    case QScroller::State::Pressed:
    case QScroller::State::Dragging:
    case QScroller::State::Scrolling:
        handleAction(HoverAction::MousePress);
        break;
    case QScroller::State::Inactive:
        handleAction(HoverAction::MouseRelease);
        break;
    }
}
