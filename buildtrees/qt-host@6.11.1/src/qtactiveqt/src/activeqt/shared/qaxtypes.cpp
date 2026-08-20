// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#include <ocidl.h>
#include <olectl.h>

#include "qaxtypes_p.h"
#include <QtAxBase/private/qaxutils_p.h>
#include <QtAxBase/private/qaxtypefunctions_p.h>

#include <qcursor.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <private/qpixmap_win_p.h>
#include <qobject.h>
#include <qdebug.h>
#ifdef QAX_SERVER
#   include <qaxfactory.h>
#   include <private/qsystemlibrary_p.h>
#else
#   include <quuid.h>
#   include <qaxobject.h>
#endif

#include <QtCore/private/qcomptr_p.h>

QT_BEGIN_NAMESPACE

#ifdef QAX_SERVER
#   define QVariantToVARIANT QVariantToVARIANT_server
#   define VARIANTToQVariant VARIANTToQVariant_server
extern ITypeLib *qAxTypeLibrary;

CLSID CLSID_QRect = { 0x34030f30, 0xe359, 0x4fe6, {0xab, 0x82, 0x39, 0x76, 0x6f, 0x5d, 0x91, 0xee } };
CLSID CLSID_QSize = { 0xcb5f84b3, 0x29e5, 0x491d, {0xba, 0x18, 0x54, 0x72, 0x48, 0x8e, 0xef, 0xba } };
CLSID CLSID_QPoint = { 0x3be838a3, 0x3fac, 0xbfc4, {0x4c, 0x6c, 0x37, 0xc4, 0x4d, 0x03, 0x02, 0x52 } };

GUID IID_IAxServerBase = { 0xbd2ec165, 0xdfc9, 0x4319, { 0x8b, 0x9b, 0x60, 0xa5, 0x74, 0x78, 0xe9, 0xe3} };
#else
#   define QVariantToVARIANT QVariantToVARIANT_container
#   define VARIANTToQVariant VARIANTToQVariant_container
extern void *qax_createObjectWrapper(int metaType, IUnknown *iface);
#endif

static IFontDisp *QFontToIFont(const QFont &font)
{
    FONTDESC fdesc;
    memset(&fdesc, 0, sizeof(fdesc));
    fdesc.cbSizeofstruct = sizeof(FONTDESC);
    fdesc.cySize.Lo = font.pointSize() * 10000;
    fdesc.fItalic = font.italic();
    fdesc.fStrikethrough = font.strikeOut();
    fdesc.fUnderline = font.underline();
    fdesc.lpstrName = QStringToBSTR(font.family());
    fdesc.sWeight = font.weight() * 10;

    ComPtr<IFontDisp> f;
    HRESULT res = OleCreateFontIndirect(&fdesc, IID_IFontDisp, &f);
    if (res != S_OK) {
        f.Reset();
#if defined(QT_CHECK_STATE)
        qWarning("QFontToIFont: Failed to create IFont");
#endif
    }
    return f.Detach();
}

static QFont IFontToQFont(IFont *f)
{
    BSTR name;
    BOOL bold;
    SHORT charset;
    BOOL italic;
    CY size;
    BOOL strike;
    BOOL underline;
    SHORT weight;
    f->get_Name(&name);
    f->get_Bold(&bold);
    f->get_Charset(&charset);
    f->get_Italic(&italic);
    f->get_Size(&size);
    f->get_Strikethrough(&strike);
    f->get_Underline(&underline);
    f->get_Weight(&weight);
    QFont font(QString::fromWCharArray(name), size.Lo/9750, weight / 97, italic);
    font.setBold(bold);
    font.setStrikeOut(strike);
    font.setUnderline(underline);
    SysFreeString(name);

    return font;
}

static IPictureDisp *QPixmapToIPicture(const QPixmap &pixmap)
{
    ComPtr<IPictureDisp> pic;

    PICTDESC desc;
    desc.cbSizeofstruct = sizeof(PICTDESC);
    desc.picType = PICTYPE_BITMAP;

    desc.bmp.hbitmap = nullptr;
    desc.bmp.hpal = nullptr;

    if (!pixmap.isNull()) {
        desc.bmp.hbitmap = qt_pixmapToWinHBITMAP(pixmap);
        Q_ASSERT(desc.bmp.hbitmap);
    }

    HRESULT res = OleCreatePictureIndirect(&desc, IID_IPictureDisp, true, &pic);
    if (res != S_OK) {
        pic.Reset();
#if defined(QT_CHECK_STATE)
        qWarning("QPixmapToIPicture: Failed to create IPicture");
#endif
    }
    return pic.Detach();
}

static QPixmap IPictureToQPixmap(IPicture *ipic)
{
    SHORT type;
    ipic->get_Type(&type);
    if (type != PICTYPE_BITMAP)
        return QPixmap();

    HBITMAP hbm = nullptr;
    ipic->get_Handle(reinterpret_cast<OLE_HANDLE*>(&hbm));
    if (!hbm)
        return QPixmap();

    return qt_pixmapFromWinHBITMAP(hbm);
}

static QDateTime DATEToQDateTime(DATE ole)
{
    SYSTEMTIME stime;
    if (ole >= 949998 || !VariantTimeToSystemTime(ole, &stime))
        return QDateTime();

    QDate date(stime.wYear, stime.wMonth, stime.wDay);
    QTime time(stime.wHour, stime.wMinute, stime.wSecond, stime.wMilliseconds);
    return QDateTime(date, time);
}

static DATE QDateTimeToDATE(const QDateTime &dt)
{
    if (!dt.isValid() || dt.isNull())
        return 949998;

    SYSTEMTIME stime;
    memset(&stime, 0, sizeof(stime));
    QDate date = dt.date();
    QTime time = dt.time();
    if (date.isValid() && !date.isNull()) {
        stime.wDay = date.day();
        stime.wMonth = date.month();
        stime.wYear = date.year();
    }
    if (time.isValid() && !time.isNull()) {
        stime.wMilliseconds = time.msec();
        stime.wSecond = time.second();
        stime.wMinute = time.minute();
        stime.wHour = time.hour();
    }

    double vtime;
    SystemTimeToVariantTime(&stime, &vtime);

    return vtime;
}

static QByteArray msgOutParameterNotSupported(const QByteArray &type)
{
    return QByteArrayLiteral("QVariantToVARIANT: out-parameter not supported for \"")
        + type + QByteArrayLiteral("\".");
}

/*
    Converts \a var to \a arg, and tries to coerce \a arg to \a type.

    Used by

    QAxServerBase:
        - QAxServerBase::qt_metacall
        - IDispatch::Invoke(PROPERTYGET, METHOD)
        - IPersistPropertyBag::Save

    QAxBase:
        - IDispatch::Invoke (QAxEventSink)
        - QAxBase::internalProperty(WriteProperty)
        - QAxBase::internalInvoke()
        - QAxBase::dynamicCallHelper()
        - IPropertyBag::Read (QtPropertyBag)

    Also called recoursively for lists.
*/

// Convenience macro for function QVariantToVARIANT()
// storing a POD QVariant value in the VARIANT arg.
#define QVARIANT_TO_VARIANT_POD(type, value, out, varType, varMember, varPointerMember) \
    if (out && arg.vt == ((varType) | VT_BYREF)) { \
        *arg.varPointerMember = value; /* pre-allocated out-parameter */ \
    } else { \
        if (out) { \
            arg.vt = (varType) | VT_BYREF; \
            arg.varPointerMember = new type(value); \
        } else { \
            arg.vt = (varType); \
            arg.varMember = value; \
        } \
    }

static qsizetype columnCount2D(const QVariantList &list)
{
    if (!list.isEmpty()) {
        const auto &firstElement = list.at(0);
        const auto type = firstElement.typeId();
        if (type != QMetaType::QString && type != QMetaType::QByteArray
            && firstElement.canConvert<QVariantList>()) {
            return firstElement.toList().size();
        }
    }
    return 0;
}

bool QVariantToVARIANT(const QVariant &var, VARIANT &arg, const QByteArray &typeName, bool out)
{
    QVariant qvar = var;
    // "type" is the expected type, so coerce if necessary
    const int proptype = typeName.isEmpty()
        ? QMetaType::UnknownType
        : QMetaType::fromName(typeName).id();
    if (proptype != QMetaType::UnknownType
        && proptype != QMetaType::User
        && proptype != QMetaType::QVariant
        && proptype != qvar.metaType().id()) {
        const QMetaType metaType(proptype);
        if (qvar.canConvert(metaType))
            qvar.convert(metaType);
        else
            qvar = QVariant(metaType);
    }

    if (out && arg.vt == (VT_VARIANT|VT_BYREF) && arg.pvarVal) {
        return QVariantToVARIANT(var, *arg.pvarVal, typeName, false);
    }

    if (out && proptype == QMetaType::QVariant) {
        VARIANT *pVariant = new VARIANT;
        QVariantToVARIANT(var, *pVariant, QByteArray(), false);
        arg.vt = VT_VARIANT|VT_BYREF;
        arg.pvarVal = pVariant;
        return true;
    }

    switch (qvar.metaType().id()) {
    case QMetaType::QString:
        if (out && arg.vt == (VT_BSTR|VT_BYREF)) {
            if (*arg.pbstrVal)
                SysFreeString(*arg.pbstrVal);
            *arg.pbstrVal = QStringToBSTR(qvar.toString());
            arg.vt = VT_BSTR|VT_BYREF;
        } else {
            arg.vt = VT_BSTR;
            arg.bstrVal = QStringToBSTR(qvar.toString());
            if (out) {
                arg.pbstrVal = new BSTR(arg.bstrVal);
                arg.vt |= VT_BYREF;
            }
        }
        break;

    case QMetaType::Char:
        QVARIANT_TO_VARIANT_POD(char, char(qvar.toInt()), out, VT_I1, cVal, pcVal)
        break;

    case QMetaType::UChar:
        QVARIANT_TO_VARIANT_POD(BYTE, uchar(qvar.toUInt()), out, VT_UI1, bVal, pbVal)
        break;

    case QMetaType::Short:
        QVARIANT_TO_VARIANT_POD(short, qvariant_cast<short>(qvar), out, VT_I2, iVal, piVal)
        break;

    case QMetaType::UShort:
        QVARIANT_TO_VARIANT_POD(ushort, qvariant_cast<ushort>(qvar), out, VT_UI2, uiVal, puiVal)
        break;

    case QMetaType::Int:
        QVARIANT_TO_VARIANT_POD(long, qvar.toInt(), out, VT_I4, lVal, plVal)
        break;

    case QMetaType::UInt:
        QVARIANT_TO_VARIANT_POD(uint, qvar.toUInt(), out, VT_UI4, uintVal, puintVal)
        break;

    case QMetaType::LongLong:
        if (out && arg.vt == (VT_CY|VT_BYREF)) { // VT_CY: Currency
            arg.pcyVal->int64 = qvar.toLongLong();
        } else {
            QVARIANT_TO_VARIANT_POD(LONGLONG, qvar.toLongLong(), out, VT_I8, llVal, pllVal)
        }
        break;

    case QMetaType::ULongLong:
        if (out && arg.vt == (VT_CY|VT_BYREF)) { // VT_CY: Currency
            arg.pcyVal->int64 = qvar.toULongLong();
        } else {
            QVARIANT_TO_VARIANT_POD(ULONGLONG, qvar.toULongLong(), out, VT_UI8, ullVal, pullVal)
        }
        break;

    case QMetaType::Bool:
        QVARIANT_TO_VARIANT_POD(short, short(qvar.toBool() ? VARIANT_TRUE : VARIANT_FALSE),
                                out, VT_BOOL, boolVal, pboolVal)
        break;

    case QMetaType::Float:
        QVARIANT_TO_VARIANT_POD(float, float(qvar.toDouble()), out, VT_R4, fltVal, pfltVal)
        break;

    case QMetaType::Double:
        QVARIANT_TO_VARIANT_POD(double, qvar.toDouble(), out, VT_R8, dblVal, pdblVal)
        break;

    case QMetaType::QColor:
        QVARIANT_TO_VARIANT_POD(long, QColorToOLEColor(qvariant_cast<QColor>(qvar)),
                                out, VT_COLOR, lVal, plVal)
        break;

    case QMetaType::QDate:
    case QMetaType::QTime:
    case QMetaType::QDateTime: // DATE = double
        QVARIANT_TO_VARIANT_POD(DATE, QDateTimeToDATE(qvar.toDateTime()),
                                out, VT_DATE, date, pdate)
        break;

    case QMetaType::QFont:
        if (out && arg.vt == (VT_DISPATCH|VT_BYREF)) {
            if (*arg.ppdispVal)
                (*arg.ppdispVal)->Release();
            *arg.ppdispVal = QFontToIFont(qvariant_cast<QFont>(qvar));
        } else {
            arg.vt = VT_DISPATCH;
            arg.pdispVal = QFontToIFont(qvariant_cast<QFont>(qvar));
            if (out) {
                arg.ppdispVal = new IDispatch*(arg.pdispVal);
                arg.vt |= VT_BYREF;
            }
        }
        break;

    case QMetaType::QPixmap:
        if (out && arg.vt == (VT_DISPATCH|VT_BYREF)) {
            if (*arg.ppdispVal)
                (*arg.ppdispVal)->Release();
            *arg.ppdispVal = QPixmapToIPicture(qvariant_cast<QPixmap>(qvar));
        } else {
            arg.vt = VT_DISPATCH;
            arg.pdispVal = QPixmapToIPicture(qvariant_cast<QPixmap>(qvar));
            if (out) {
                arg.ppdispVal = new IDispatch*(arg.pdispVal);
                arg.vt |= VT_BYREF;
            }
        }
        break;

    case QMetaType::QCursor:
        {
#ifndef QT_NO_CURSOR
            int shape = qvariant_cast<QCursor>(qvar).shape();
            if (out && (arg.vt & VT_BYREF)) {
                switch(arg.vt & ~VT_BYREF) {
                case VT_I4:
                    *arg.plVal = shape;
                    break;
                case VT_I2:
                    *arg.piVal = shape;
                    break;
                case VT_UI4:
                    *arg.pulVal = shape;
                    break;
                case VT_UI2:
                    *arg.puiVal = shape;
                    break;
                case VT_INT:
                    *arg.pintVal = shape;
                    break;
                case VT_UINT:
                    *arg.puintVal = shape;
                    break;
                }
            } else {
                arg.vt = VT_I4;
                arg.lVal = shape;
                if (out) {
                    arg.plVal = new long(arg.lVal);
                    arg.vt |= VT_BYREF;
                }
            }
#endif
        }
        break;

    case QMetaType::QVariantList:
        {
            const auto list = qvar.toList();
            const qsizetype count = list.size();
            VARTYPE vt = VT_VARIANT;
            int listType = QMetaType::QVariant;
            if (!typeName.isEmpty() && typeName.startsWith("QList<")) {
                const QByteArray listTypeName = typeName.mid(6, typeName.length() - 7); // QList<int> -> int
                listType = QMetaType::fromName(listTypeName).id();
            }

            VARIANT variant;
            void *pElement = &variant;
            switch(listType) {
            case QMetaType::Int:
                vt = VT_I4;
                pElement = &variant.lVal;
                break;
            case QMetaType::Double:
                vt = VT_R8;
                pElement = &variant.dblVal;
                break;
            case QMetaType::QDateTime:
                vt = VT_DATE;
                pElement = &variant.date;
                break;
            case QMetaType::Bool:
                vt = VT_BOOL;
                pElement = &variant.boolVal;
                break;
            case QMetaType::LongLong:
                vt = VT_I8;
                pElement = &variant.llVal;
                break;
            default:
                break;
            }
            SAFEARRAY *array = nullptr;
            // If the first element in the array is a list the whole list is
            // treated as a 2D array. The column count is taken from the 1st element.
            if (qsizetype maxColumns = columnCount2D(list)) {
                SAFEARRAYBOUND rgsabound[2] = { {0, 0}, {0, 0} };
                rgsabound[0].cElements = count;
                rgsabound[1].cElements = maxColumns;
                array = SafeArrayCreate(VT_VARIANT, 2, rgsabound);
                LONG rgIndices[2];
                for (LONG i = 0; i < count; ++i) {
                    rgIndices[0] = i;
                    QVariantList columns = list.at(i).toList();
                    qsizetype columnCount = qMin(maxColumns, columns.size());
                    for (LONG j = 0;  j < columnCount; ++j) {
                        const QVariant &elem = columns.at(j);
                        VariantInit(&variant);
                        QVariantToVARIANT(elem, variant, elem.typeName());
                        rgIndices[1] = j;
                        SafeArrayPutElement(array, rgIndices, pElement);
                        clearVARIANT(&variant);
                    }
                }
            } else {
                array = SafeArrayCreateVector(vt, 0, count);
                for (LONG index = 0; index < count; ++index) {
                    QVariant elem = list.at(index);
                    if (listType != QMetaType::QVariant)
                        elem.convert(QMetaType(listType));
                    VariantInit(&variant);
                    QVariantToVARIANT(elem, variant, elem.typeName());
                    SafeArrayPutElement(array, &index, pElement);
                    clearVARIANT(&variant);
                }
            }
            if (out && arg.vt == (VT_ARRAY|vt|VT_BYREF)) {
                if (*arg.pparray)
                    SafeArrayDestroy(*arg.pparray);
                *arg.pparray = array;
            } else {
                arg.vt = VT_ARRAY|vt;
                arg.parray = array;
                if (out) {
                    arg.pparray = new SAFEARRAY*(arg.parray);
                    arg.vt |= VT_BYREF;
                }
            }
        }
        break;

    case QMetaType::QStringList:
        {
            const QStringList list = qvar.toStringList();
            const qsizetype count = list.size();
            SAFEARRAY *array = SafeArrayCreateVector(VT_BSTR, 0, count);
            for (LONG index = 0; index < count; ++index) {
                QString elem = list.at(index);
                BSTR bstr = QStringToBSTR(elem);
                SafeArrayPutElement(array, &index, bstr);
                SysFreeString(bstr);
            }

            if (out && arg.vt == (VT_ARRAY|VT_BSTR|VT_BYREF)) {
                if (*arg.pparray)
                    SafeArrayDestroy(*arg.pparray);
                *arg.pparray = array;
            } else {
                arg.vt = VT_ARRAY|VT_BSTR;
                arg.parray = array;
                if (out) {
                    arg.pparray = new SAFEARRAY*(arg.parray);
                    arg.vt |= VT_BYREF;
                }
            }
        }
        break;

    case QMetaType::QByteArray:
        {
            const QByteArray bytes = qvar.toByteArray();
            const uint count = static_cast<uint>(bytes.size());
            SAFEARRAY *array = SafeArrayCreateVector(VT_UI1, 0, count);
            if (count) {
                const char *data = bytes.constData();
                char *dest;
                SafeArrayAccessData(array, reinterpret_cast<void**>(&dest));
                memcpy(dest, data, count);
                SafeArrayUnaccessData(array);
            }

            if (out && arg.vt == (VT_ARRAY|VT_UI1|VT_BYREF)) {
                if (*arg.pparray)
                    SafeArrayDestroy(*arg.pparray);
                *arg.pparray = array;
            } else {
                arg.vt = VT_ARRAY|VT_UI1;
                arg.parray = array;
                if (out) {
                    arg.pparray = new SAFEARRAY*(arg.parray);
                    arg.vt |= VT_BYREF;
                }
            }
        }
        break;

#ifdef QAX_SERVER
    case QMetaType::QRect:
    case QMetaType::QSize:
    case QMetaType::QPoint:
        {
            typedef HRESULT(WINAPI* PGetRecordInfoFromTypeInfo)(ITypeInfo *, IRecordInfo **);
            static PGetRecordInfoFromTypeInfo pGetRecordInfoFromTypeInfo = 0;
            static bool resolved = false;
            if (!resolved) {
                resolved = true;
                pGetRecordInfoFromTypeInfo = (PGetRecordInfoFromTypeInfo)QSystemLibrary::resolve(QLatin1String("oleaut32"),
                                              "GetRecordInfoFromTypeInfo");
            }
            if (!pGetRecordInfoFromTypeInfo)
                break;

            ComPtr<ITypeInfo> typeInfo;
            IRecordInfo *recordInfo = 0;
            const int vType = qvar.metaType().id();
            CLSID clsid = vType == QMetaType::QRect
                ? CLSID_QRect
                : vType == QMetaType::QSize ? CLSID_QSize : CLSID_QPoint;
            qAxTypeLibrary->GetTypeInfoOfGuid(clsid, &typeInfo);
            if (!typeInfo)
                break;
            pGetRecordInfoFromTypeInfo(typeInfo.Get(), &recordInfo);
            if (!recordInfo)
                break;

            void *record = 0;
            switch (qvar.metaType().id()) {
            case QMetaType::QRect:
                {
                    QRect qrect(qvar.toRect());
                    recordInfo->RecordCreateCopy(&qrect, &record);
                }
                break;
            case QMetaType::QSize:
                {
                    QSize qsize(qvar.toSize());
                    recordInfo->RecordCreateCopy(&qsize, &record);
                }
                break;
            case QMetaType::QPoint:
                {
                    QPoint qpoint(qvar.toPoint());
                    recordInfo->RecordCreateCopy(&qpoint, &record);
                }
                break;
            default:
                break;
            }

            if (out) {
                qWarning().noquote() << msgOutParameterNotSupported("records");
                arg.vt = VT_EMPTY;
                arg.byref = nullptr;
                return false;
            }
            arg.vt = VT_RECORD;
            arg.pRecInfo = recordInfo,
            arg.pvRecord = record;
        }
        break;
#endif // QAX_SERVER

    case QMetaType::UnknownType: // default-parameters not set
        if (out && arg.vt == (VT_ERROR|VT_BYREF)) {
            *arg.plVal = DISP_E_PARAMNOTFOUND;
        } else {
            arg.vt = VT_ERROR;
            arg.lVal = DISP_E_PARAMNOTFOUND;
            if (out) {
                arg.plVal = new long(arg.lVal);
                arg.vt |= VT_BYREF;
            }
        }
        break;

    default:
        if (qvar.metaType().id() >= QMetaType::User) {
            QByteArray subType = qvar.typeName();
#ifdef QAX_SERVER
            if (subType.endsWith('*'))
                subType.truncate(subType.length() - 1);
#endif
            if (!qstrcmp(qvar.typeName(), "IDispatch*")) {
                if (out) {
                    qWarning().noquote() << msgOutParameterNotSupported(qvar.typeName());
                    arg.vt = VT_EMPTY;
                    arg.byref = nullptr;
                    return false;
                }
                arg.vt = VT_DISPATCH;
                arg.pdispVal = *static_cast<IDispatch**>(qvar.data());
                if (arg.pdispVal)
                    arg.pdispVal->AddRef();
            } else if (!qstrcmp(qvar.typeName(), "IDispatch**")) {
                arg.vt = VT_DISPATCH;
                arg.ppdispVal = *static_cast<IDispatch***>(qvar.data());
                if (out)
                    arg.vt |= VT_BYREF;
            } else if (!qstrcmp(qvar.typeName(), "IUnknown*")) {
                if (out) {
                    qWarning().noquote() << msgOutParameterNotSupported(qvar.typeName());
                    arg.vt = VT_EMPTY;
                    arg.byref = nullptr;
                    return false;
                }
                arg.vt = VT_UNKNOWN;
                arg.punkVal = *static_cast<IUnknown**>(qvar.data());
                if (arg.punkVal)
                    arg.punkVal->AddRef();
#ifdef QAX_SERVER
            } else if (qAxFactory()->metaObject(QString::fromLatin1(subType.constData()))) {
                if (out) {
                    qWarning().noquote() << msgOutParameterNotSupported("subtype");
                    arg.vt = VT_EMPTY;
                    arg.byref = nullptr;
                    return false;
                }
                arg.vt = VT_DISPATCH;
                void *user = *(void**)qvar.constData();
//                qVariantGet(qvar, user, qvar.typeName());
                if (!user) {
                    arg.pdispVal = 0;
                } else {
                    qAxFactory()->createObjectWrapper(static_cast<QObject*>(user), &arg.pdispVal);
                }
#else
            } else if (QMetaType::fromName(subType).id() != QMetaType::UnknownType) {
                if (out) {
                    qWarning().noquote() << msgOutParameterNotSupported("subtype");
                    arg.vt = VT_EMPTY;
                    arg.byref = nullptr;
                    return false;
                }
                QAxObject *object = *static_cast<QAxObject**>(qvar.data());
//                qVariantGet(qvar, object, subType);
                arg.vt = VT_DISPATCH;
                object->queryInterface(IID_IDispatch, reinterpret_cast<void**>(&arg.pdispVal));
#endif
            } else {
                return false;
            }
        } else { // >= User
            return false;
        }
        break;
    }

    Q_ASSERT(!out || (arg.vt & VT_BYREF));
    return true;
}

#ifdef QAX_SERVER
static QVariant axServer(IUnknown *unknown, const QByteArray &typeName)
{
    ComPtr<IAxServerBase> iface;
    if (unknown && typeName != "IDispatch*" && typeName != "IUnknown*")
        unknown->QueryInterface(IID_IAxServerBase, &iface);
    if (iface == nullptr)
        return {};

    auto *qObj = iface->qObject();
    QByteArray pointerType = qObj ? QByteArray(qObj->metaObject()->className()) + '*' : typeName;
    QMetaType pointerMetaType = QMetaType::fromName(pointerType);
    if (pointerMetaType.id() == QMetaType::UnknownType)
        pointerMetaType = QMetaType(qRegisterMetaType<QObject *>(pointerType));
    return QVariant(pointerMetaType, &qObj);
}
#endif // QAX_SERVER

#undef QVARIANT_TO_VARIANT_POD

/*
    Returns \a arg as a QVariant of type \a typeName or \a type.

    NOTE: If a \a typeName is specified, value type is assumed. to
    get/create a pointer type, provide the type id in the \a type argument.

    Used by

    QAxServerBase:
        - QAxServerBase::qt_metacall(update out parameters/return value)
        - IDispatch::Invoke(METHOD, PROPERTYPUT)
        - IPersistPropertyBag::Load

    QAxBase:
        - IDispatch::Invoke (QAxEventSink)
        - QAxBase::internalProperty(ReadProperty)
        - QAxBase::internalInvoke(update out parameters/return value)
        - QAxBase::dynamicCallHelper(update out parameters)
        - QAxBase::dynamicCall(return value)
        - IPropertyBag::Write (QtPropertyBag)
*/
QVariant VARIANTToQVariant(const VARIANT &arg, const QByteArray &typeName, int type)
{
    int nameTypeId = QMetaType::UnknownType;
    if (type == QMetaType::UnknownType && !typeName.isEmpty()) {
        auto name = typeName.endsWith('*')
            ? QByteArrayView{typeName.constData(), typeName.size() - 1}
            : QByteArrayView{typeName};
        nameTypeId = QMetaType::fromName(name).id();
    }

    QVariant var;
    switch(arg.vt) {
    case VT_BSTR:
        var = QString::fromWCharArray(arg.bstrVal);
        break;
    case VT_BSTR|VT_BYREF:
        var = QString::fromWCharArray(*arg.pbstrVal);
        break;
    case VT_BOOL:
        var = QVariant((bool)arg.boolVal);
        break;
    case VT_BOOL|VT_BYREF:
        var = QVariant((bool)*arg.pboolVal);
        break;
    case VT_I1:
        var = arg.cVal;
        if (typeName == "char")
            type = QMetaType::Int;
        break;
    case VT_I1|VT_BYREF:
        var = *arg.pcVal;
        if (typeName == "char")
            type = QMetaType::Int;
        break;
    case VT_I2:
        var = arg.iVal;
        if (typeName == "short")
            type = QMetaType::Int;
        break;
    case VT_I2|VT_BYREF:
        var = *arg.piVal;
        if (typeName == "short")
            type = QMetaType::Int;
        break;
    case VT_I4:
        if (type == QMetaType::QColor || nameTypeId == QMetaType::QColor)
            var = QVariant::fromValue(OLEColorToQColor(arg.lVal));
#ifndef QT_NO_CURSOR
        else if (type == QMetaType::QCursor || nameTypeId == QMetaType::QCursor)
            var = QVariant::fromValue(QCursor(static_cast<Qt::CursorShape>(arg.lVal)));
#endif
        else
            var = (int)arg.lVal;
        break;
    case VT_I4|VT_BYREF:
        if (type == QMetaType::QColor || nameTypeId == QMetaType::QColor)
            var = QVariant::fromValue(OLEColorToQColor((int)*arg.plVal));
#ifndef QT_NO_CURSOR
        else if (type == QMetaType::QCursor || nameTypeId == QMetaType::QCursor)
            var = QVariant::fromValue(QCursor(static_cast<Qt::CursorShape>(*arg.plVal)));
#endif
        else
            var = (int)*arg.plVal;
        break;
    case VT_INT:
        var = arg.intVal;
        break;
    case VT_INT|VT_BYREF:
        var = *arg.pintVal;
        break;
    case VT_UI1:
        var = arg.bVal;
        break;
    case VT_UI1|VT_BYREF:
        var = *arg.pbVal;
        break;
    case VT_UI2:
        var = arg.uiVal;
        break;
    case VT_UI2|VT_BYREF:
        var = *arg.puiVal;
        break;
    case VT_UI4:
        if (type == QMetaType::QColor || nameTypeId == QMetaType::QColor)
            var = QVariant::fromValue(OLEColorToQColor(arg.ulVal));
#ifndef QT_NO_CURSOR
        else if (type == QMetaType::QCursor || nameTypeId == QMetaType::QCursor)
            var = QVariant::fromValue(QCursor(static_cast<Qt::CursorShape>(arg.ulVal)));
#endif
        else
            var = (int)arg.ulVal;
        break;
    case VT_UI4|VT_BYREF:
        if (type == QMetaType::QColor || nameTypeId == QMetaType::QColor)
            var = QVariant::fromValue(OLEColorToQColor((uint)*arg.pulVal));
#ifndef QT_NO_CURSOR
        else if (type == QMetaType::QCursor || nameTypeId == QMetaType::QCursor)
            var = QVariant::fromValue(QCursor(static_cast<Qt::CursorShape>(*arg.pulVal)));
#endif
        else
            var = (int)*arg.pulVal;
        break;
    case VT_UINT:
        var = arg.uintVal;
        break;
    case VT_UINT|VT_BYREF:
        var = *arg.puintVal;
        break;
    case VT_CY:
        var = arg.cyVal.int64;
        break;
    case VT_CY|VT_BYREF:
        var = arg.pcyVal->int64;
        break;
    case VT_I8:
        var = arg.llVal;
        break;
    case VT_I8|VT_BYREF:
        var = *arg.pllVal;
        break;
    case VT_UI8:
        var = arg.ullVal;
        break;
    case VT_UI8|VT_BYREF:
        var = *arg.pullVal;
        break;
    case VT_R4:
        var = arg.fltVal;
        break;
    case VT_R4|VT_BYREF:
        var = *arg.pfltVal;
        break;
    case VT_R8:
        var = arg.dblVal;
        break;
    case VT_R8|VT_BYREF:
        var = *arg.pdblVal;
        break;
    case VT_DATE:
        var = DATEToQDateTime(arg.date);
        if (type == QMetaType::QDate || nameTypeId == QMetaType::QDate) {
            var.convert(QMetaType(QMetaType::QDate));
        } else if (type == QMetaType::QTime || nameTypeId == QMetaType::QTime) {
            var.convert(QMetaType(QMetaType::QTime));
        }
        break;
    case VT_DATE|VT_BYREF:
        var = DATEToQDateTime(*arg.pdate);
        if (type == QMetaType::QDate || nameTypeId == QMetaType::QDate) {
            var.convert(QMetaType(QMetaType::QDate));
        } else if (type == QMetaType::QTime || nameTypeId == QMetaType::QTime) {
            var.convert(QMetaType(QMetaType::QTime));
        }
        break;
    case VT_VARIANT:
    case VT_VARIANT|VT_BYREF:
        if (arg.pvarVal)
            var = VARIANTToQVariant(*arg.pvarVal, typeName);
        break;

    case VT_DISPATCH:
    case VT_DISPATCH|VT_BYREF:
        {
            // pdispVal and ppdispVal are a union
            IDispatch *disp = nullptr;
            if (arg.vt & VT_BYREF)
                disp = *arg.ppdispVal;
            else
                disp = arg.pdispVal;
            if (type == QMetaType::QFont || nameTypeId == QMetaType::QFont) {
                ComPtr<IFont> ifont;
                if (disp)
                    disp->QueryInterface(IID_IFont, &ifont);
                if (ifont) {
                    var = QVariant::fromValue(IFontToQFont(ifont.Get()));
                } else {
                    var = QVariant::fromValue(QFont());
                }
            } else if (type == QMetaType::QPixmap || nameTypeId == QMetaType::QPixmap) {
                ComPtr<IPicture> ipic;
                if (disp)
                    disp->QueryInterface(IID_IPicture, &ipic);
                if (ipic) {
                    var = QVariant::fromValue(IPictureToQPixmap(ipic.Get()));
                } else {
                    var = QVariant::fromValue(QPixmap());
                }
            } else {
#ifdef QAX_SERVER
                if (auto axs = axServer(disp, typeName); axs.isValid()) {
                    var = axs;
                } else
#endif
                {
                    if (!typeName.isEmpty()) {
                        if (arg.vt & VT_BYREF) {
                            // When the dispinterface is a return value, just assign it to a QVariant
                            static const int dispatchId = qRegisterMetaType<IDispatch**>("IDispatch**");
                            var = QVariant(QMetaType(dispatchId), &arg.ppdispVal);
                        } else {
#ifndef QAX_SERVER
                            if (typeName == "QVariant") {
                                // If a QVariant is requested, wrap the dispinterface in a QAxObject
                                QAxObject *object = new QAxObject(disp);
                                var = QVariant::fromValue<QAxObject*>(object);
                            } else if (typeName != "IDispatch*" &&  QMetaType::fromName(typeName).id() != QMetaType::UnknownType) {
                                // Conversion from IDispatch* to a wrapper type is requested. Here, the requested
                                // wrapper type is constructed around the dispinterface, and then returned as
                                // a QVariant containing a pointer to the wrapper type.

                                // Calculate the value type from a potential pointer type
                                QByteArray valueTypeStr = QByteArray(typeName);
                                int pIndex = typeName.lastIndexOf('*');
                                if (pIndex != -1)
                                    valueTypeStr = typeName.left(pIndex);

                                const QMetaType metaValueType = QMetaType::fromName(valueTypeStr);
                                Q_ASSERT(metaValueType.id() != QMetaType::UnknownType);

                                auto object = static_cast<QAxObject*>(qax_createObjectWrapper(metaValueType.id(), disp));

                                // Return object as the original type
                                const QMetaType returnType = QMetaType::fromName(typeName);
                                Q_ASSERT(metaValueType.id() != QMetaType::UnknownType);

                                var = QVariant(returnType, &object);

                                // The result must be a pointer to an instance derived from QObject
                                Q_ASSERT((var.metaType().flags() & QMetaType::PointerToQObject) != 0);
                            } else {
#endif
                                // An IDispatch pointer is requested, no conversion required, just return as QVariant
                                // containing the pointer.
                                static const int dispatchId = qRegisterMetaType<IDispatch*>(typeName.constData());
                                var = QVariant(QMetaType(dispatchId), &disp);
#ifndef QAX_SERVER
                            }
#endif
                        }
                    }
                }
            }
        }
        break;
    case VT_UNKNOWN:
    case VT_UNKNOWN|VT_BYREF:
        {
            IUnknown *unkn = nullptr;
            if (arg.vt & VT_BYREF)
                unkn = *arg.ppunkVal;
            else
                unkn = arg.punkVal;
            var.setValue(unkn);
#ifdef QAX_SERVER
            if (auto axs = axServer(unkn, typeName); axs.isValid())
                var = axs;
#endif
        }
        break;
    case VT_ARRAY|VT_VARIANT:
    case VT_ARRAY|VT_VARIANT|VT_BYREF:
        {
            SAFEARRAY *array = nullptr;
            if ( arg.vt & VT_BYREF )
                array = *arg.pparray;
            else
                array = arg.parray;

            UINT cDims = array ? SafeArrayGetDim(array) : 0;
            switch(cDims) {
            case 1:
                {
                    QVariantList list;

                    long lBound, uBound;
                    SafeArrayGetLBound( array, 1, &lBound );
                    SafeArrayGetUBound( array, 1, &uBound );

                    for ( long i = lBound; i <= uBound; ++i ) {
                        VARIANT var;
                        VariantInit( &var );
                        SafeArrayGetElement( array, &i, &var );

                        QVariant qvar = VARIANTToQVariant( var, nullptr );
                        clearVARIANT( &var );
                        list << qvar;
                    }

                    var = list;
                }
                break;

            case 2:
                {
                    QVariantList listList; //  a list of lists
                    long dimIndices[2];

                    long xlBound, xuBound, ylBound, yuBound;
                    SafeArrayGetLBound(array, 1, &xlBound);
                    SafeArrayGetUBound(array, 1, &xuBound);
                    SafeArrayGetLBound(array, 2, &ylBound);
                    SafeArrayGetUBound(array, 2, &yuBound);

                    for (long x = xlBound; x <= xuBound; ++x) {
                        QVariantList list;

                        dimIndices[0] = x;
                        for (long y = ylBound; y <= yuBound; ++y) {
                            VARIANT var;
                            VariantInit(&var);
                            dimIndices[1] = y;
                            SafeArrayGetElement(array, dimIndices, &var);

                            QVariant qvar = VARIANTToQVariant(var, nullptr);
                            clearVARIANT(&var);
                            list << qvar;
                        }

                        listList << QVariant(list);
                    }
                    var = listList;
                }
                break;
            default:
                var = QVariantList();
                break;
            }
        }
        break;

    case VT_ARRAY|VT_BSTR:
    case VT_ARRAY|VT_BSTR|VT_BYREF:
        {
            SAFEARRAY *array = nullptr;
            if (arg.vt & VT_BYREF)
                array = *arg.pparray;
            else
                array = arg.parray;

            QStringList strings;
            if (!array || array->cDims != 1) {
                var = strings;
                break;
            }

            long lBound, uBound;
            SafeArrayGetLBound(array, 1, &lBound);
            SafeArrayGetUBound(array, 1, &uBound);

            for (long i = lBound; i <= uBound; ++i) {
                BSTR bstr;
                SafeArrayGetElement(array, &i, &bstr);
                strings << QString::fromWCharArray(bstr);
                SysFreeString(bstr);
            }

            var = strings;
        }
        break;

    case VT_ARRAY|VT_UI1:
    case VT_ARRAY|VT_UI1|VT_BYREF:
        {
            SAFEARRAY *array = nullptr;
            if (arg.vt & VT_BYREF)
                array = *arg.pparray;
            else
                array = arg.parray;

            QByteArray bytes;
            if (!array || array->cDims != 1) {
                var = bytes;
                break;
            }

            long lBound, uBound;
            SafeArrayGetLBound(array, 1, &lBound);
            SafeArrayGetUBound(array, 1, &uBound);

            if (uBound != -1) { // non-empty array
                bytes.resize(uBound - lBound + 1);
                char *data = bytes.data();
                char *src;
                SafeArrayAccessData(array, reinterpret_cast<void**>(&src));
                memcpy(data, src, bytes.size());
                SafeArrayUnaccessData(array);
            }

            var = bytes;
        }
        break;

#if defined(QAX_SERVER)
    case VT_RECORD:
    case VT_RECORD|VT_BYREF:
        if (arg.pvRecord && arg.pRecInfo) {
            IRecordInfo *recordInfo = arg.pRecInfo;
            void *record = arg.pvRecord;
            GUID guid;
            recordInfo->GetGuid(&guid);

            if (guid == CLSID_QRect) {
                QRect qrect;
                recordInfo->RecordCopy(record, &qrect);
                var = qrect;
            } else if (guid == CLSID_QSize) {
                QSize qsize;
                recordInfo->RecordCopy(record, &qsize);
                var = qsize;
            } else if (guid == CLSID_QPoint) {
                QPoint qpoint;
                recordInfo->RecordCopy(record, &qpoint);
                var = qpoint;
            }
        }
        break;
#endif // QAX_SERVER
    default:
        // support for any SAFEARRAY(Type) where Type can be converted to a QVariant
        // -> QVariantList
        if (arg.vt & VT_ARRAY) {
            SAFEARRAY *array = nullptr;
            if (arg.vt & VT_BYREF)
                array = *arg.pparray;
            else
                array = arg.parray;

            QVariantList list;
            if (!array || array->cDims != 1) {
                var = list;
                break;
            }

            // find out where to store the element
            VARTYPE vt;
            VARIANT variant;
            SafeArrayGetVartype(array, &vt);

            void *pElement = nullptr;
            switch(vt) {
            case VT_BSTR: Q_ASSERT(false); break; // already covered
            case VT_BOOL: pElement = &variant.boolVal; break;
            case VT_I1: pElement = &variant.cVal; break;
            case VT_I2: pElement = &variant.iVal; break;
            case VT_I4: pElement = &variant.lVal; break;
            case VT_I8: pElement = &variant.llVal; break;
            case VT_UI8: pElement = &variant.ullVal; break;
            case VT_INT: pElement = &variant.intVal; break;
            case VT_UI1: Q_ASSERT(false); break; // already covered
            case VT_UI2: pElement = &variant.uiVal; break;
            case VT_UI4: pElement = &variant.ulVal; break;
            case VT_UINT: pElement = &variant.uintVal; break;
            case VT_CY: pElement = &variant.cyVal; break;
            case VT_R4: pElement = &variant.fltVal; break;
            case VT_R8: pElement = &variant.dblVal; break;
            case VT_DATE: pElement = &variant.date; break;
            case VT_VARIANT: Q_ASSERT(false); break; // already covered
            default:
                break;
            }
            if (!pElement) {
                var = list;
                break;
            }

            long lBound, uBound;
            SafeArrayGetLBound( array, 1, &lBound );
            SafeArrayGetUBound( array, 1, &uBound );

            for ( long i = lBound; i <= uBound; ++i ) {
                variant.vt = vt;
                SafeArrayGetElement(array, &i, pElement);
                QVariant qvar = VARIANTToQVariant(variant, nullptr);
                clearVARIANT(&variant);
                list << qvar;
            }

            var = list;
        }
        break;
    }

    const int proptype = type != QMetaType::UnknownType || typeName.isEmpty() || typeName == "QVariant"
        ? type : nameTypeId;

    if (proptype != QMetaType::QVariant && proptype != QMetaType::UnknownType
        && var.metaType().id() != proptype) {
        QMetaType propertyMetaType(proptype);
        if (var.canConvert(propertyMetaType)) {
            QVariant oldvar = var;
            if (oldvar.convert(propertyMetaType))
                var = oldvar;
        } else if (proptype == QMetaType::QStringList
                   && var.metaType().id() == QMetaType::QVariantList) {
            bool allStrings = true;
            QStringList strings;
            const QVariantList list(var.toList());
            for (const QVariant &variant : list) {
                if (variant.canConvert<QString>())
                    strings << variant.toString();
                else
                    allStrings = false;
            }
            if (allStrings)
                var = strings;
        } else {
            var = QVariant();
        }
    }
    return var;
}

QT_END_NAMESPACE
