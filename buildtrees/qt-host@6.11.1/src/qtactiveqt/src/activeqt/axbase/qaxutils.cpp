// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#include "qaxutils_p.h"

#include <QtWidgets/qwidget.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qregion.h>
#include <QtGui/qwindow.h>
#include <QtGui/qguiapplication.h>
#include <private/qhighdpiscaling_p.h>
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatformpixmap.h>
#include <QtGui/private/qpixmap_raster_p.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qrect.h>

#include <QtCore/qdebug.h>

#include "qbstr_p.h"

QT_BEGIN_NAMESPACE

static inline QWindow *windowForWidget(QWidget *widget)
{
    if (QWindow *window = widget->windowHandle())
        return window;
    if (QWidget *nativeParent = widget->nativeParentWidget())
        return nativeParent->windowHandle();
    return nullptr;
}

HWND hwndForWidget(QWidget *widget)
{
    if (QWindow *window = windowForWidget(widget))
        return static_cast<HWND> (QGuiApplication::platformNativeInterface()->nativeResourceForWindow("handle", window));
    return nullptr;
}

// Code courtesy of QWindowsXPStyle
static void addRectToHrgn(HRGN &winRegion, const QRect &r)
{
    HRGN rgn = CreateRectRgn(r.left(), r.top(), r.x() + r.width(), r.y() + r.height());
    if (rgn) {
        HRGN dest = CreateRectRgn(0,0,0,0);
        int result = CombineRgn(dest, winRegion, rgn, RGN_OR);
        if (result) {
            DeleteObject(winRegion);
            winRegion = dest;
        }
        DeleteObject(rgn);
    }
}

HRGN qaxHrgnFromQRegion(const QRegion &region, const QWindow *window)
{
    HRGN hRegion = CreateRectRgn(0, 0, 0, 0);
    for (const QRect &rect : QHighDpi::toNativeLocalRegion(region, window))
        addRectToHrgn(hRegion, rect);
    return hRegion;
}

// HIMETRICS scaling
static const qreal himetricsPerInch = 2540;

static inline long qaxMapPixToLogHiMetrics(int x, qreal logicalDpi, qreal factor)
{
    return qRound((qreal(x) * himetricsPerInch * factor) / logicalDpi);
}

static inline int qaxMapLogHiMetricsToPix(long x, qreal logicalDpi, qreal factor)
{
    return qRound(logicalDpi * qreal(x) / (himetricsPerInch * factor));
}

SIZEL qaxMapPixToLogHiMetrics(const QSize &s, const QDpi &d, const QWindow *w)
{
    const qreal factor = QHighDpiScaling::factor(w);
    const SIZEL result = {
        qaxMapPixToLogHiMetrics(s.width(), d.first, factor),
        qaxMapPixToLogHiMetrics(s.height(), d.second, factor)
    };
    return result;
}

QSize qaxMapLogHiMetricsToPix(const SIZEL &s, const QDpi &d, const QWindow *w)
{
    const qreal factor = QHighDpiScaling::factor(w);
    const QSize result(qaxMapLogHiMetricsToPix(s.cx, d.first, factor),
                       qaxMapLogHiMetricsToPix(s.cy, d.second, factor));
    return result;
}

// Cache logical DPI in case High DPI scaling is active (which case
// the fake logical DPI it calculates is not suitable).

static QDpi cachedSystemLogicalDpi(-1, -1);

void qaxClearCachedSystemLogicalDpi() // Call from WM_DISPLAYCHANGE
{
    cachedSystemLogicalDpi = QDpi(-1, -1);
}

static inline QDpi systemLogicalDpi()
{
    if (cachedSystemLogicalDpi.first < 0) {
        const HDC displayDC = GetDC(nullptr);
        cachedSystemLogicalDpi = QDpi(GetDeviceCaps(displayDC, LOGPIXELSX), GetDeviceCaps(displayDC, LOGPIXELSY));
        ReleaseDC(nullptr, displayDC);
    }
    return cachedSystemLogicalDpi;
}

static inline QDpi paintDeviceLogicalDpi(const QPaintDevice *d)
{
    return QDpi(d->logicalDpiX(), d->logicalDpiY());
}

#ifdef QT_WIDGETS_LIB

SIZEL qaxMapPixToLogHiMetrics(const QSize &s, const QWidget *widget)
{
    return qaxMapPixToLogHiMetrics(s,
                                   QHighDpiScaling::isActive() ? systemLogicalDpi() : paintDeviceLogicalDpi(widget),
                                   widget->windowHandle());
}

QSize qaxMapLogHiMetricsToPix(const SIZEL &s, const QWidget *widget)
{
    return qaxMapLogHiMetricsToPix(s,
                                   QHighDpiScaling::isActive() ? systemLogicalDpi() : paintDeviceLogicalDpi(widget),
                                   widget->windowHandle());
}

QPoint qaxFromNativePosition(const QWidget *w, const QPoint &nativePos)
{
    const qreal factor = QHighDpiScaling::factor(w->windowHandle());
    return qFuzzyCompare(factor, 1)
        ? nativePos : (QPointF(nativePos) / factor).toPoint();
}

QPoint qaxNativeWidgetPosition(const QWidget *w)
{
    return qaxFromNativePosition(w, w->geometry().topLeft());
}

QSize qaxToNativeSize(const QWidget *w, const QSize &size)
{
    const qreal factor = QHighDpiScaling::factor(w->windowHandle());
    return qFuzzyCompare(factor, 1) ? size : (QSizeF(size) * factor).toSize();
}

QSize qaxNativeWidgetSize(const QWidget *w)
{
    return qaxToNativeSize(w, w->size());
}

QSize qaxFromNativeSize(const QWidget *w, const QSize &size)
{
    const qreal factor = QHighDpiScaling::factor(w->windowHandle());
    return qFuzzyCompare(factor, 1) ? size : (QSizeF(size) / factor).toSize();
}

RECT qaxNativeWidgetRect(const QWidget *w)
{
    return QHighDpiScaling::isActive()
        ? qaxQRect2Rect(QRect(qaxNativeWidgetPosition(w), qaxNativeWidgetSize(w)))
        : qaxQRect2Rect(w->geometry());
}

QRect qaxFromNativeRect(const RECT &r, const QWidget *w)
{
    const QRect qr = qaxRect2QRect(r);
    const qreal factor = QHighDpiScaling::factor(w->windowHandle());
    return qFuzzyCompare(factor, 1)
        ? qr
        : QRect((QPointF(qr.topLeft()) / factor).toPoint(), (QSizeF(qr.size()) / factor).toSize());
}

HRGN qaxHrgnFromQRegion(const QRegion &region, const QWidget *widget)
{
    return qaxHrgnFromQRegion(region, widget->windowHandle());
}

#endif // QT_WIDGETS_LIB

QByteArray qaxTypeInfoName(ITypeInfo *typeInfo, MEMBERID memId)
{
    QByteArray result;
    QBStr names;
    UINT cNames = 0;
    typeInfo->GetNames(memId, &names, 1, &cNames);
    if (cNames && names)
        result = names.str().toLatin1();

    return result;
}

QByteArrayList qaxTypeInfoNames(ITypeInfo *typeInfo, MEMBERID memId)
{
    QByteArrayList result;
    QBStr bstrNames[256];
    UINT maxNames = 255;
    UINT maxNamesOut = 0;
    typeInfo->GetNames(memId, reinterpret_cast<BSTR *>(&bstrNames), maxNames, &maxNamesOut);
    result.reserve(maxNamesOut);
    for (UINT p = 0; p < maxNamesOut; ++p)
        result.append(bstrNames[p].str().toLatin1());
    return result;
}

QT_END_NAMESPACE
