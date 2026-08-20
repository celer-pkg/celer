// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef HOVERWATCHER_H
#define HOVERWATCHER_H
#include <QObject>
#include <QEvent>
#include <QScroller>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

class HoverWatcher : public QObject
{
    Q_OBJECT

private:
    explicit HoverWatcher(QWidget *watched);
    static QMap<QWidget *, HoverWatcher *> m_hoverWatchers;

public:
    ~HoverWatcher();

    enum HoverAction {
        Entered,
        MousePress,
        MouseRelease,
        Left,
        Ignore
    };
    Q_ENUM(HoverAction);

    bool eventFilter(QObject *obj, QEvent *event) override;

    Qt::CursorShape cursorShape(HoverAction type) const;
    Qt::MouseButtons mouseButtons() const { return m_mouseButtons; }

    static HoverWatcher *watcher(QWidget *watched);
    static const HoverWatcher *watcher(const QWidget *watched);
    static bool hasWatcher(QWidget *widget);
    static void dismiss(QWidget *watched);

public slots:
    void setCursorShape(HoverAction type, Qt::CursorShape shape);
    void unSetCursorShape(HoverAction type);
    void setMouseButtons(Qt::MouseButtons buttons);
    void setMouseButton(Qt::MouseButton button, bool enable);

signals:
    void entered();
    void mousePressed();
    void mouseReleased();
    void left();
    void hoverAction(HoverAction action);

private slots:
    void handleScrollerStateChange(QScroller::State state);

private:
    QWidget *m_watched;
    std::array<std::optional<Qt::CursorShape>, HoverAction::Ignore> m_cursorShapes;
    Qt::MouseButtons m_mouseButtons = Qt::MouseButton::LeftButton;
    void handleAction(HoverAction action);
    void setApplicationCursor(HoverAction action) const;
    bool hasShape(HoverAction action) const;
};

#endif // HOVERWATCHER_H
