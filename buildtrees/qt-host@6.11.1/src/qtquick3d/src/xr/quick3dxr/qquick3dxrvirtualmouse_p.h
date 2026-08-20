// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DXRVIRTUALMOUSE_P_H
#define QQUICK3DXRVIRTUALMOUSE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <QEvent>
#include <QtQml/qqml.h>
#include <QTimer>

QT_BEGIN_NAMESPACE

class QQuick3DXrView;
class QQuick3DNode;

class QQuick3DXrVirtualMouse : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool rightMouseButton READ rightMouseButton WRITE setRightMouseButton NOTIFY rightMouseButtonChanged)
    Q_PROPERTY(bool leftMouseButton READ leftMouseButton WRITE setLeftMouseButton NOTIFY leftMouseButtonChanged)
    Q_PROPERTY(bool middleMouseButton READ middleMouseButton WRITE setMiddleMouseButton NOTIFY middleMouseButtonChanged)
    Q_PROPERTY(float scrollWheelX READ scrollWheelX WRITE setScrollWheelX NOTIFY scrollWheelXChanged)
    Q_PROPERTY(float scrollWheelY READ scrollWheelY WRITE setScrollWheelY NOTIFY scrollWheelYChanged)
    Q_PROPERTY(int scrollTimerInterval READ scrollTimerInterval WRITE setScrollTimerInterval NOTIFY scrollTimerIntervalChanged)
    Q_PROPERTY(int scrollPixelDelta READ scrollPixelDelta WRITE setScrollPixelDelta NOTIFY scrollPixelDeltaChanged)
    Q_PROPERTY(QQuick3DNode* source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QQuick3DXrView* view READ view WRITE setView NOTIFY viewChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

    QML_NAMED_ELEMENT(XrVirtualMouse)
    QML_ADDED_IN_VERSION(6, 8)

public:
    explicit QQuick3DXrVirtualMouse(QObject *parent = nullptr);

    bool rightMouseButton() const;
    bool leftMouseButton() const;
    bool middleMouseButton() const;
    float scrollWheelX() const;
    float scrollWheelY() const;
    int scrollTimerInterval() const;
    int scrollPixelDelta() const;

    QQuick3DNode *source() const;
    QQuick3DXrView* view() const;

    bool enabled() const;

public Q_SLOTS:
    void setRightMouseButton(bool rightMouseButton);
    void setLeftMouseButton(bool leftMouseButton);
    void setMiddleMouseButton(bool middleMouseButton);
    void setScrollWheelX(float scrollWheelX);
    void setScrollWheelY(float scrollWheelY);
    void setScrollTimerInterval(int scrollTimerInterval);
    void setScrollPixelDelta(int scrollPixelDelta);
    void setSource(QQuick3DNode* source);
    void setView(QQuick3DXrView* view);
    void setEnabled(bool enabled);

private Q_SLOTS:
    void moveEvent();
    void generateWheelEvent();

Q_SIGNALS:
    void rightMouseButtonChanged(bool rightMouseButton);
    void leftMouseButtonChanged(bool leftMouseButton);
    void middleMouseButtonChanged(bool middleMouseButton);
    void scrollWheelXChanged(float scrollWheelX);
    void scrollWheelYChanged(float scrollWheelY);
    void scrollTimerIntervalChanged(int scrollTimerInterval);
    void scrollPixelDeltaChanged(int scrollPixelDelta);
    void sourceChanged(QQuick3DNode* source);
    void viewChanged(QQuick3DXrView* view);
    void enabledChanged(bool enabled);

private:
    void generateEvent(QEvent::Type type, Qt::MouseButton button = Qt::NoButton);

    QTimer* m_scrollTimer = nullptr;
    bool m_rightMouseButton = false;
    bool m_leftMouseButton = false;
    bool m_middleMouseButton = false;
    float m_scrollWheelX = 0;
    float m_scrollWheelY = 0;
    int m_scrollTimerInterval = 30;
    int m_scrollPixelDelta = 15;
    QQuick3DNode* m_source = nullptr;
    QQuick3DXrView* m_view = nullptr;
    bool m_enabled = true;
};

QT_END_NAMESPACE

#endif // QQUICK3DXRVIRTUALMOUSE_P_H
