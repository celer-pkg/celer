// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#include <qt_windows.h>
#include "qaxtypefunctions_p.h"
#include <qcursor.h>
#include <qpixmap.h>
#include <qsize.h>
#include <qrect.h>
#include <qpoint.h>

QT_BEGIN_NAMESPACE

QColor OLEColorToQColor(uint col)
{
    return QColor(GetRValue(col),GetGValue(col),GetBValue(col));
}

/*
    Copies the data in \a var into \a data.

    Used by

    QAxServerBase:
        - QAxServerBase::qt_metacall (update out parameters/return value)

    QAxBase:
        - internalProperty(ReadProperty)
        - internalInvoke(update out parameters/return value)

*/
bool QVariantToVoidStar(const QVariant &var, void *data, const QByteArray &typeName, int type)
{
    if (!data)
        return true;

    if (type == QMetaType::QVariant || (type == QMetaType::UnknownType && typeName == "QVariant")) {
        *reinterpret_cast<QVariant *>(data) = var;
        return true;
    }

    switch (var.metaType().id()) {
    case QMetaType::UnknownType:
        break;
    case QMetaType::QString:
        *reinterpret_cast<QString *>(data) = var.toString();
        break;
    case QMetaType::Int:
        *reinterpret_cast<int *>(data) = var.toInt();
        break;
    case QMetaType::UInt:
        *reinterpret_cast<uint *>(data) = var.toUInt();
        break;
    case QMetaType::Bool:
        *reinterpret_cast<bool *>(data) = var.toBool();
        break;
    case QMetaType::Double:
        *reinterpret_cast<double *>(data) = var.toDouble();
        break;
    case QMetaType::QColor:
        *reinterpret_cast<QColor *>(data) = qvariant_cast<QColor>(var);
        break;
    case QMetaType::QDate:
        *reinterpret_cast<QDate *>(data) = var.toDate();
        break;
    case QMetaType::QTime:
        *reinterpret_cast<QTime *>(data) = var.toTime();
        break;
    case QMetaType::QDateTime:
        *reinterpret_cast<QDateTime *>(data) = var.toDateTime();
        break;
    case QMetaType::QFont:
        *reinterpret_cast<QFont *>(data) = qvariant_cast<QFont>(var);
        break;
    case QMetaType::QPixmap:
        *reinterpret_cast<QPixmap *>(data) = qvariant_cast<QPixmap>(var);
        break;
#ifndef QT_NO_CURSOR
    case QMetaType::QCursor:
        *reinterpret_cast<QCursor *>(data) = qvariant_cast<QCursor>(var);
        break;
#endif
    case QMetaType::QVariantList:
        *reinterpret_cast<QVariantList *>(data) = var.toList();
        break;
    case QMetaType::QStringList:
        *reinterpret_cast<QStringList *>(data) = var.toStringList();
        break;
    case QMetaType::QByteArray:
        *reinterpret_cast<QByteArray *>(data) = var.toByteArray();
        break;
    case QMetaType::LongLong:
        *reinterpret_cast<qint64 *>(data) = var.toLongLong();
        break;
    case QMetaType::ULongLong:
        *reinterpret_cast<quint64 *>(data) = var.toULongLong();
        break;
    case QMetaType::QRect:
        *reinterpret_cast<QRect *>(data) = var.toRect();
        break;
    case QMetaType::QSize:
        *reinterpret_cast<QSize *>(data) = var.toSize();
        break;
    case QMetaType::QPoint:
        *reinterpret_cast<QPoint *>(data) = var.toPoint();
        break;
    default:
        if (var.metaType().id() >= QMetaType::User) {
            *reinterpret_cast<void **>(data) =
                *reinterpret_cast<void **>(const_cast<void *>(var.constData()));
        } else {
            qWarning("QVariantToVoidStar: Unhandled QVariant type");
        }
        return false;
    }

    return true;
}

void clearVARIANT(VARIANT *var)
{
    if (var->vt & VT_BYREF) {
        switch (var->vt) {
        case VT_BSTR|VT_BYREF:
            SysFreeString(*var->pbstrVal);
            delete var->pbstrVal;
            break;
        case VT_BOOL|VT_BYREF:
            delete var->pboolVal;
            break;
        case VT_I1|VT_BYREF:
            delete var->pcVal;
            break;
        case VT_I2|VT_BYREF:
            delete var->piVal;
            break;
        case VT_I4|VT_BYREF:
            delete var->plVal;
            break;
        case VT_INT|VT_BYREF:
            delete var->pintVal;
            break;
        case VT_UI1|VT_BYREF:
            delete var->pbVal;
            break;
        case VT_UI2|VT_BYREF:
            delete var->puiVal;
            break;
        case VT_UI4|VT_BYREF:
            delete var->pulVal;
            break;
        case VT_UINT|VT_BYREF:
            delete var->puintVal;
            break;
        case VT_I8|VT_BYREF:
            delete var->pllVal;
            break;
        case VT_UI8|VT_BYREF:
            delete var->pullVal;
            break;
        case VT_CY|VT_BYREF:
            delete var->pcyVal;
            break;
        case VT_R4|VT_BYREF:
            delete var->pfltVal;
            break;
        case VT_R8|VT_BYREF:
            delete var->pdblVal;
            break;
        case VT_DATE|VT_BYREF:
            delete var->pdate;
            break;
        case VT_DISPATCH|VT_BYREF:
            if (var->ppdispVal) {
                if (*var->ppdispVal)
                    (*var->ppdispVal)->Release();
                delete var->ppdispVal;
            }
            break;
        case VT_ARRAY|VT_VARIANT|VT_BYREF:
        case VT_ARRAY|VT_UI1|VT_BYREF:
        case VT_ARRAY|VT_BSTR|VT_BYREF:
            SafeArrayDestroy(*var->pparray);
            delete var->pparray;
            break;
        case VT_VARIANT|VT_BYREF:
            delete var->pvarVal;
            break;
        }
        VariantInit(var);
    } else {
        VariantClear(var);
    }
}

QT_END_NAMESPACE
