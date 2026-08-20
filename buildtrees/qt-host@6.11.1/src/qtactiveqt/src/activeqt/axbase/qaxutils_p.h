// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#ifndef QAXUTILS_P_H
#define QAXUTILS_P_H

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

#include <QtCore/qt_windows.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qpair.h>
#include <QtCore/qrect.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QWidget;
class QPixmap;
class QRect;
class QRegion;
class QWindow;

HWND hwndForWidget(QWidget *widget);
HRGN qaxHrgnFromQRegion(const QRegion &region, const QWindow *window);

using QDpi = QPair<qreal, qreal>;

extern SIZEL qaxMapPixToLogHiMetrics(const QSize &s, const QDpi &d, const QWindow *w);
extern QSize qaxMapLogHiMetricsToPix(const SIZEL &s, const QDpi &d, const QWindow *w);

void qaxClearCachedSystemLogicalDpi(); // Call from WM_DISPLAYCHANGE

static inline RECT qaxQRect2Rect(const QRect &r)
{
    RECT result = { r.x(), r.y(), r.x() + r.width(), r.y() + r.height() };
    return result;
}

static inline QSize qaxSizeOfRect(const RECT &rect)
{
    return QSize(rect.right -rect.left, rect.bottom - rect.top);
}

static inline QRect qaxRect2QRect(const RECT &rect)
{
    return QRect(QPoint(rect.left, rect.top), qaxSizeOfRect(rect));
}

static inline RECT qaxContentRect(const QSize &size)  // Size with topleft = 0,0
{
    RECT result = { 0, 0, size.width(), size.height() };
    return result;
}

static inline wchar_t *qaxQString2MutableOleChars(QString &s) // must be passed an lvalue
{
    // using utf16() to force NUL-termination:
    return const_cast<wchar_t *>(reinterpret_cast<const wchar_t *>(s.utf16()));
}

#ifdef QT_WIDGETS_LIB
SIZEL qaxMapPixToLogHiMetrics(const QSize &s, const QWidget *widget);
QSize qaxMapLogHiMetricsToPix(const SIZEL &s, const QWidget *widget);

QPoint qaxFromNativePosition(const QWidget *w, const QPoint &nativePos);
QPoint qaxNativeWidgetPosition(const QWidget *w);
QSize qaxToNativeSize(const QWidget *w, const QSize &size);
QSize qaxFromNativeSize(const QWidget *w, const QSize &size);
QSize qaxNativeWidgetSize(const QWidget *w);
RECT qaxNativeWidgetRect(const QWidget *w);
QRect qaxFromNativeRect(const RECT &r, const QWidget *w);
HRGN qaxHrgnFromQRegion(const QRegion &region, const QWidget *widget);
#endif // QT_WIDGETS_LIB

QByteArray qaxTypeInfoName(ITypeInfo *typeInfo, MEMBERID memId);
QByteArrayList qaxTypeInfoNames(ITypeInfo *typeInfo, MEMBERID memId);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(IDispatch**)

#endif
