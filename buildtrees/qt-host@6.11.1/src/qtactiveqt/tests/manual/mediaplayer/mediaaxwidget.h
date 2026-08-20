// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef MEDIAAXWIDGET_H
#define MEDIAAXWIDGET_H

#include <QtAxContainer/QAxWidget>
#include <qt_windows.h>

// Overrides the translateKeyEvent() function to pass keystrokes
// to the Windows Media Player ActiveX control.
class MediaAxWidget : public QAxWidget
{
public:
    MediaAxWidget(QWidget *parent = nullptr, Qt::WindowFlags f = {})
        : QAxWidget(parent, f)
    {
    }

protected:
    bool translateKeyEvent(int message, int keycode) const override
    {
        if (message >= WM_KEYFIRST && message <= WM_KEYLAST)
            return true;
        return QAxWidget::translateKeyEvent(message, keycode);
    }
};

#endif // MEDIAAXWIDGET_H
