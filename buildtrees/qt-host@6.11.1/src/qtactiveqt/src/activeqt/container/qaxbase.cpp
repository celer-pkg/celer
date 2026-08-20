// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


//#define QAX_NO_CLASSINFO

#define QT_CHECK_STATE

#include "qaxobject.h"
#include "qaxbase_p.h"

#include <QtAxBase/private/qaxtypefunctions_p.h>

#include <qfile.h>
#include <qwidget.h>
#include <quuid.h>
#include <qhash.h>
#include <qset.h>
#include <qpair.h>
#include <qbitarray.h>
#include <qmetaobject.h>
#include <qsettings.h>
#include <qdebug.h>
#include <QGuiApplication>
#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>

#ifndef QT_NO_THREAD
#   include <qmutex.h>
#endif

#include <private/qobject_p.h>
#include <private/qmetaobject_p.h>
#include <private/qmetaobjectbuilder_p.h>
#include <private/qtools_p.h>
#include <private/qbstr_p.h>
#include <qt_windows.h>
#include <ocidl.h>
#include <ctype.h>

#include "../shared/qaxtypes_p.h"
#include <QtAxBase/private/qaxutils_p.h>
#include <QtCore/private/qcomobject_p.h>
#include <QtCore/private/qcomptr_p.h>

QT_BEGIN_NAMESPACE

static inline HRESULT Invoke(IDispatch *disp,
                             DISPID dispIdMember,
                             REFIID riid,
                             LCID lcid,
                             DWORD wFlags,
                             DISPPARAMS *pDispParams,
                             VARIANT *pVarResult,
                             EXCEPINFO *pExcepInfo,
                             unsigned int *puArgErr)
{
    if ((wFlags & DISPATCH_PROPERTYPUT) &&
        pDispParams &&
        pDispParams->cArgs == 1 &&
        pDispParams->cNamedArgs == 1 &&
        pDispParams->rgdispidNamedArgs &&
        *pDispParams->rgdispidNamedArgs == DISPID_PROPERTYPUT &&
        pDispParams->rgvarg) {
        VARTYPE vt = pDispParams->rgvarg->vt;

        if (vt == VT_UNKNOWN || vt == VT_DISPATCH || (vt & VT_ARRAY) || (vt & VT_BYREF)) {
            const HRESULT hr =
                disp->Invoke(dispIdMember, riid, lcid, (WORD(wFlags) & ~DISPATCH_PROPERTYPUT) | DISPATCH_PROPERTYPUTREF,
                             pDispParams, pVarResult, pExcepInfo, puArgErr);
            if (SUCCEEDED(hr))
                return hr;
        }
    }

    return disp->Invoke(dispIdMember, riid, lcid, WORD(wFlags), pDispParams, pVarResult, pExcepInfo, puArgErr);
}

/*
    \internal
    \class QAxMetaObject

    \brief The QAxMetaObject class stores extended information
*/
class QMetaObjectExtra
{
public:
    using DispIdNameMapping = QMap<DISPID, QByteArray>;
    using UuidDispIdMapping = QMap< QUuid, QMap<DISPID, QByteArray> >;

    QMetaObjectExtra() = default;

    int numParameter(const QByteArray &prototype) const;
    QByteArray paramType(const QByteArray &signature, int index, bool *out = nullptr) const;
    QByteArray propertyType(const QByteArray &propertyName) const;
    void parsePrototype(const QByteArray &prototype) const;
    DISPID dispIDofName(const QByteArray &name, IDispatch *disp) const;

    const QList<QUuid> &connectionInterfaces() const { return m_connectionInterfaces; }

    const UuidDispIdMapping &sigs() const { return m_sigs; }
    const UuidDispIdMapping &propsigs() const { return m_propsigs; }
    const UuidDispIdMapping &props() const { return m_props; }

private:
    friend class MetaObjectGenerator;
    // save information about QAxEventSink connections, and connect when found in cache
    QList<QUuid> m_connectionInterfaces;
    // DISPID -> signal name
    UuidDispIdMapping m_sigs;
    // DISPID -> property changed signal name
    UuidDispIdMapping m_propsigs;
    // DISPID -> property name
    UuidDispIdMapping m_props;

    // Prototype -> member info
    mutable QHash<QByteArray, QByteArrayList> memberInfo;
    QMap<QByteArray, QByteArray> realPrototype;

    // DISPID cache
    mutable QHash<QByteArray, DISPID> dispIDs;
};

void QMetaObjectExtra::parsePrototype(const QByteArray &prototype) const
{
    QByteArray realProto = realPrototype.value(prototype, prototype);
    QByteArray parameters = realProto.mid(realProto.indexOf('(') + 1);
    parameters.truncate(parameters.length() - 1);

    if (parameters.isEmpty())
        memberInfo.insert(prototype, {});
    else
        memberInfo.insert(prototype, parameters.split(','));
}

inline QByteArray QMetaObjectExtra::propertyType(const QByteArray &propertyName) const
{
    return realPrototype.value(propertyName);
}

int QMetaObjectExtra::numParameter(const QByteArray &prototype) const
{
    if (!memberInfo.contains(prototype))
        parsePrototype(prototype);

    return memberInfo.value(prototype).size();
}

QByteArray QMetaObjectExtra::paramType(const QByteArray &prototype, int index, bool *out) const
{
    if (!memberInfo.contains(prototype))
        parsePrototype(prototype);

    if (out)
        *out = false;

    const auto plist = memberInfo.value(prototype);
    if (index > plist.size() - 1)
        return QByteArray();

    QByteArray param(plist.at(index));
    if (param.isEmpty())
        return QByteArray();

    bool byRef = param.endsWith('&') || param.endsWith("**");
    if (byRef) {
        param.truncate(param.length() - 1);
        if (out)
            *out = true;
    }

    return param;
}

inline DISPID QMetaObjectExtra::dispIDofName(const QByteArray &name, IDispatch *disp) const
{
    DISPID dispid = dispIDs.value(name, DISPID_UNKNOWN);
    if (dispid == DISPID_UNKNOWN) {
        // get the Dispatch ID from the object
        QString unicodeName = QLatin1String(name);
        OLECHAR *names = qaxQString2MutableOleChars(unicodeName);
        disp->GetIDsOfNames(IID_NULL, &names, 1, LOCALE_USER_DEFAULT, &dispid);
        if (dispid != DISPID_UNKNOWN)
            dispIDs.insert(name, dispid);
    }
    return dispid;
}


static QHash<QString, QMetaObject*> mo_cache;
static QHash<const QMetaObject*, QMetaObjectExtra> moextra_cache;
static QHash<QUuid, QMap<QByteArray, QList<QPair<QByteArray, int> > > > enum_cache;
static int mo_cache_ref = 0;
static QMutex cache_mutex;


static const char *const type_conversion[][2] =
{
    { "float", "double"},
    { "short", "int"},
    { "char", "int"},
    { "QList<int>", "QVariantList" },
    { "QList<uint>", "QVariantList" },
    { "QList<double>", "QVariantList" },
    { "QList<bool>", "QVariantList" },
    { "QList<QDateTime>", "QVariantList" },
    { "QList<qlonglong>", "QVariantList" },
    { nullptr, nullptr }
};

/*
    \internal
    \class QAxEventSink

    \brief The QAxEventSink class implements the event sink for all
           IConnectionPoints implemented in the COM object.
*/

class QAxEventSink : public QComObject<IDispatch, IPropertyNotifySink>
{
    Q_DISABLE_COPY_MOVE(QAxEventSink)
public:
    explicit QAxEventSink(QAxBase *com) : combase(com) {}
    virtual ~QAxEventSink()
    {
        Q_ASSERT(!cpoint);
    }

    QUuid connectionInterface() const
    {
        return ciid;
    }
    QMap<DISPID, QByteArray> signalMap() const
    {
        return sigs;
    }
    QMap<DISPID, QByteArray> propertyMap() const
    {
        return props;
    }
    QMap<DISPID, QByteArray> propSignalMap() const
    {
        return propsigs;
    }

    // add a connection
    void advise(IConnectionPoint *cp, IID iid)
    {
        cpoint = cp;
        ciid = iid;
        cpoint->Advise(static_cast<IUnknown *>(static_cast<IDispatch *>(this)), &cookie);
    }

    // disconnect from all connection points
    void unadvise()
    {
        combase = nullptr;
        if (cpoint) {
            cpoint->Unadvise(cookie);
            cpoint.Reset();
        }
    }

    void addSignal(DISPID memid, const char *name)
    {
        QByteArray signalname = name;
        int pi = signalname.indexOf('(');
        int i = 0;
        while (type_conversion[i][0]) {
            int ti = pi;
            int len = int(strlen(type_conversion[i][0]));
            while ((ti = signalname.indexOf(type_conversion[i][0], ti)) != -1)
                signalname.replace(ti, len, type_conversion[i][1]);
            ++i;
        }

        sigs.insert(memid, signalname);
        const DISPID id = propsigs.key(signalname, -1);
        if (id != -1)
            propsigs.remove(id);
    }
    void addProperty(DISPID propid, const char *name, const char *signal)
    {
        props.insert(propid, name);
        propsigs.insert(propid, signal);
    }

    // IDispatch
    HRESULT __stdcall GetTypeInfoCount(unsigned int *) override
    { return E_NOTIMPL; }
    HRESULT __stdcall GetTypeInfo(UINT, LCID, ITypeInfo **) override
    { return E_NOTIMPL; }
    HRESULT __stdcall GetIDsOfNames(const _GUID &, wchar_t **, unsigned int,
                                    unsigned long, long *) override
    { return E_NOTIMPL; }

    HRESULT __stdcall Invoke(DISPID dispIdMember, REFIID riid, LCID,
                             WORD wFlags, DISPPARAMS *pDispParams,
                             VARIANT *, EXCEPINFO *, UINT *) override
    {
        // verify input
        if (riid != IID_NULL)
            return DISP_E_UNKNOWNINTERFACE;
        if (!(wFlags & DISPATCH_METHOD))
            return DISP_E_MEMBERNOTFOUND;
        if (!combase)
            return E_UNEXPECTED;

        QByteArray signame = sigs.value(dispIdMember);
        if (signame.isEmpty())
            return DISP_E_MEMBERNOTFOUND;

        QObject *qobject = combase->qObject();
        if (qobject->signalsBlocked())
            return S_OK;

        const QMetaObject *meta = qobject->metaObject();
        const QMetaObjectExtra &moExtra =  moextra_cache.value(meta);

        int index = -1;
        // emit the generic signal "as is"
        combase->d->emitSignal(QLatin1String(signame),
                               pDispParams->cArgs, pDispParams->rgvarg);
        HRESULT hres = S_OK;

        // get the signal information from the metaobject
        index = -1;
        if (signalHasReceivers(qobject, signame)) {
            index = meta->indexOfSignal(signame);
            Q_ASSERT(index != -1);
            const QMetaMethod signal = meta->method(index);
            Q_ASSERT(signal.methodType() == QMetaMethod::Signal);
            Q_ASSERT(signame == signal.methodSignature());
            // verify parameter count
            const int pcount = moExtra.numParameter(signame);
            const int argcount = int(pDispParams->cArgs);
            if (pcount > argcount)
                return DISP_E_PARAMNOTOPTIONAL;
            if (pcount < argcount)
                return DISP_E_BADPARAMCOUNT;

            // setup parameters (no return values in signals)
            bool ok = true;
            void *static_argv[QAX_NUM_PARAMS + 1];
            void *static_argv_pointer[QAX_NUM_PARAMS + 1];
            QVariant static_varp[QAX_NUM_PARAMS + 1];

            void **argv = nullptr;
            void **argv_pointer = nullptr; // in case we need an additional level of indirection
            QVariant *varp = nullptr;

            if (pcount) {
                if (pcount <= QAX_NUM_PARAMS) {
                    argv = static_argv;
                    argv_pointer = static_argv_pointer;
                    varp = static_varp;
                } else {
                    argv = new void*[pcount + 1];
                    argv_pointer = new void*[pcount + 1];
                    varp = new QVariant[pcount + 1];
                }

                argv[0] = nullptr;
                argv_pointer[0] = nullptr;
            }

            int p;
            for (p = 0; p < pcount && ok; ++p) {
                // map the VARIANT to the void*
                QByteArray ptype = moExtra.paramType(signame, p);
                varp[p + 1] = VARIANTToQVariant(pDispParams->rgvarg[pcount - p - 1], ptype);
                argv_pointer[p + 1] = nullptr;
                if (varp[p + 1].isValid()) {
                    if (varp[p + 1].metaType().id() >= QMetaType::User) {
                        argv[p + 1] = varp[p + 1].data();
                    } else if (ptype == "QVariant") {
                        argv[p + 1] = varp + p + 1;
                    } else {
                        argv[p + 1] = const_cast<void*>(varp[p + 1].constData());
                        if (ptype.endsWith('*')) {
                            argv_pointer[p + 1] = argv[p + 1];
                            argv[p + 1] = argv_pointer + p + 1;
                        }
                    }
                } else if (ptype == "QVariant") {
                    argv[p + 1] = varp + p + 1;
                } else {
                    ok = false;
                }
            }

            if (ok) {
                // emit the generated signal if everything went well
                QAxBasePrivate::qtStaticMetaCall(combase, QMetaObject::InvokeMetaMethod, index - meta->methodOffset(), argv);
                // update the VARIANT for references and free memory
                for (p = 0; p < pcount; ++p) {
                    bool out;
                    QByteArray ptype = moExtra.paramType(signame, p, &out);
                    if (out) {
                        if (!QVariantToVARIANT(varp[p + 1], pDispParams->rgvarg[pcount - p - 1], ptype, out))
                            ok = false;
                    }
                }
            }

            if (argv != static_argv) {
                delete [] argv;
                delete [] argv_pointer;
                delete [] varp;
            }
            hres = ok ? S_OK : (ok ? DISP_E_MEMBERNOTFOUND : DISP_E_TYPEMISMATCH);
        }

        return hres;
    }

    QByteArray findProperty(DISPID dispID);

    // IPropertyNotifySink
    HRESULT __stdcall OnChanged(DISPID dispID) override
    {
        // verify input
        if (dispID == DISPID_UNKNOWN || !combase)
            return S_OK;

        QObject *qobject = combase->qObject();
        if (qobject->signalsBlocked())
            return S_OK;

        const QMetaObject *meta = qobject->metaObject();
        if (!meta)
            return S_OK;

        QByteArray propname(findProperty(dispID));
        if (propname.isEmpty())
            return S_OK;

        // emit the generic signal
        combase->d->emitPropertyChanged(QString::fromLatin1(propname));

        QByteArray signame = propsigs.value(dispID);
        if (signame.isEmpty())
            return S_OK;

        const int index = meta->indexOfSignal(signame);
        if (index == -1) // bindable but not marked as bindable in typelib
            return S_OK;

        // get the signal information from the metaobject
        if (signalHasReceivers(qobject, signame)) {
            // setup parameters
            QVariant var = qobject->property(propname);
            if (!var.isValid())
                return S_OK;

            const QMetaProperty metaProp = meta->property(meta->indexOfProperty(propname));
            void *argv[] = {nullptr, var.data()};
            if (metaProp.metaType().id() == QMetaType::QVariant)
                argv[1] = &var;

            // emit the "changed" signal
            QAxBasePrivate::qtStaticMetaCall(combase, QMetaObject::InvokeMetaMethod,
                                                      index - meta->methodOffset(), argv);
        }
        return S_OK;
    }
    HRESULT __stdcall OnRequestEdit(DISPID dispID) override
    {
        if (dispID == DISPID_UNKNOWN || !combase)
            return S_OK;

        QByteArray propname(findProperty(dispID));
        if (propname.isEmpty())
            return S_OK;

        return combase->propertyWritable(propname) ? S_OK : S_FALSE;
    }

    static bool signalHasReceivers(QObject *qobject, const char *signalName)
    {
        Q_ASSERT(qobject);
        const QByteArray name = QByteArray::number(QSIGNAL_CODE) + signalName;
        return static_cast<QAxObject *>(qobject)->receivers(name.constData()) > 0;
    }

    ComPtr<IConnectionPoint> cpoint;
    IID ciid = IID_NULL;
    ULONG cookie = 0;

    QMap<DISPID, QByteArray> sigs;
    QMap<DISPID, QByteArray> propsigs;
    QMap<DISPID, QByteArray> props;

    QAxBase *combase = nullptr;
};

/*
    \internal
    \class QAxBasePrivate
*/

QAxBasePrivate::QAxBasePrivate()
    : useEventSink(true), useMetaObject(true), useClassInfo(true),
     cachedMetaObject(false), initialized(false), tryCache(false)
{
    // protect initialization
    QMutexLocker locker(&cache_mutex);
    mo_cache_ref++;

    qRegisterMetaType<IUnknown*>("IUnknown*");
    qRegisterMetaType<IDispatch*>("IDispatch*");
}

QAxBasePrivate::~QAxBasePrivate()
{
    Q_ASSERT(!ptr);
    Q_ASSERT(!disp);

    // protect cleanup
    QMutexLocker locker(&cache_mutex);
    if (!--mo_cache_ref) {
        qDeleteAll(mo_cache);
        mo_cache.clear();
    }

    CoFreeUnusedLibraries();
}

QByteArray QAxEventSink::findProperty(DISPID dispID)
{
    // look up in cache, and fall back to
    // type info for precompiled metaobjects
    QByteArray propname(props.value(dispID));

    if (!propname.isEmpty())
        return propname;

    IDispatch *dispatch = combase->d->dispatch();
    ComPtr<ITypeInfo> typeinfo;
    if (dispatch)
        dispatch->GetTypeInfo(0, LOCALE_USER_DEFAULT, &typeinfo);
    if (!typeinfo)
        return propname;


    const QByteArray propnameI = qaxTypeInfoName(typeinfo.Get(), dispID);
    if (!propnameI.isEmpty())
        propname = propnameI;

    QByteArray propsignal(propname + "Changed(");
    const QMetaObject *mo = combase->qObject()->metaObject();
    int index = mo->indexOfProperty(propname);
    const QMetaProperty prop = mo->property(index);
    propsignal += prop.typeName();
    propsignal += ')';
    addProperty(dispID, propname, propsignal);

    return propname;
}

QVariant QAxBasePrivate::VARIANTToQVariant(const VARIANT &arg, const QByteArray &typeName, uint type)
{
    return ::VARIANTToQVariant(arg, typeName, type);
}

/*!
    \class QAxBase
    \brief The QAxBase class is an abstract class that provides an API
    to initialize and access a COM object.

    \inmodule QAxContainer

    QAxBase is an abstract class that cannot be used directly, and is
    instantiated through the subclasses QAxObject and QAxWidget. This
    class provides the API to access the COM object directly
    through its IUnknown implementation. If the COM object implements
    the IDispatch interface, the properties and methods of that object
    become available as Qt properties and slots.

    \snippet src_activeqt_container_qaxbase.cpp 0

    Properties exposed by the object's IDispatch implementation can
    be read and written through the property system provided by the
    Qt Object Model (both subclasses are \l{QObject}s, so you can use
    QObject::setProperty() and QObject::property()). Properties with
    multiple parameters are not supported.

    \snippet src_activeqt_container_qaxbase.cpp 1

    Write-functions for properties and other methods exposed by the
    object's IDispatch implementation can be called directly using
    dynamicCall(), or indirectly as slots connected to a signal.

    \snippet src_activeqt_container_qaxbase.cpp 2

    Outgoing events supported by the COM object are emitted as
    standard Qt signals.

    \snippet src_activeqt_container_qaxbase.cpp 3

    QAxBase transparently converts between COM data types and the
    equivalent Qt data types. Some COM types have no equivalent Qt data structure.

    Supported COM datatypes are listed in the first column of following table.
    The second column is the Qt type that can be used with the QObject property
    functions. The third column is the Qt type that is used in the prototype of
    generated signals and slots for in-parameters, and the last column is the Qt
    type that is used in the prototype of signals and slots for out-parameters.
    \table
    \header
    \li COM type
    \li Qt property
    \li in-parameter
    \li out-parameter
    \row
    \li VARIANT_BOOL
    \li bool
    \li bool
    \li bool&
    \row
    \li BSTR
    \li QString
    \li const QString&
    \li QString&
    \row
    \li char, short, int, long
    \li int
    \li int
    \li int&
    \row
    \li uchar, ushort, uint, ulong
    \li uint
    \li uint
    \li uint&
    \row
    \li float, double
    \li double
    \li double
    \li double&
    \row
    \li DATE
    \li QDateTime
    \li const QDateTime&
    \li QDateTime&
    \row
    \li CY
    \li qlonglong
    \li qlonglong
    \li qlonglong&
    \row
    \li OLE_COLOR
    \li QColor
    \li const QColor&
    \li QColor&
    \row
    \li SAFEARRAY(VARIANT)
    \li QList\<QVariant\>
    \li const QList\<QVariant\>&
    \li QList\<QVariant\>&
    \row
    \li SAFEARRAY(int), SAFEARRAY(double), SAFEARRAY(Date)
    \li QList\<QVariant\>
    \li const QList\<QVariant\>&
    \li QList\<QVariant\>&
    \row
    \li SAFEARRAY(BYTE)
    \li QByteArray
    \li const QByteArray&
    \li QByteArray&
    \row
    \li SAFEARRAY(BSTR)
    \li QStringList
    \li const QStringList&
    \li QStringList&
    \row
    \li VARIANT
    \li type-dependent
    \li const QVariant&
    \li QVariant&
    \row
    \li IFontDisp*
    \li QFont
    \li const QFont&
    \li QFont&
    \row
    \li IPictureDisp*
    \li QPixmap
    \li const QPixmap&
    \li QPixmap&
    \row
    \li IDispatch*
    \li QAxObject*
    \li \c QAxBase::asVariant()
    \li QAxObject* (return value)
    \row
    \li IUnknown*
    \li QAxObject*
    \li \c QAxBase::asVariant()
    \li QAxObject* (return value)
    \row
    \li SCODE, DECIMAL
    \li \e unsupported
    \li \e unsupported
    \li \e unsupported
    \row
    \li VARIANT* (Since Qt 4.5)
    \li \e unsupported
    \li \e QVariant&
    \li \e QVariant&
    \endtable

    Supported are also enumerations, and typedefs to supported types.

    To call the methods of a COM interface described by the following IDL

    \snippet src_activeqt_container_qaxbase.cpp 4

    use the QAxBase API like this:

    \snippet src_activeqt_container_qaxbase.cpp 5

    Note that the QList the object should fill has to be provided as an
    element in the parameter list of \l{QVariant}s.

    If you need to access properties or pass parameters of
    unsupported datatypes you must access the COM object directly
    through its \c IDispatch implementation or other interfaces.
    Those interfaces can be retrieved through queryInterface().

    \snippet src_activeqt_container_qaxbase.cpp 6

    To get the definition of the COM interfaces you will have to use the header
    files provided with the component you want to use. Some compilers can also
    import type libraries using the #import compiler directive. See the component
    documentation to find out which type libraries you have to import, and how to use
    them.

    If you need to react to events that pass parameters of unsupported
    datatypes you can use the generic signal that delivers the event
    data as provided by the COM event.

    \sa QAxObject, QAxWidget, QAxScript, {ActiveQt Framework}
*/

/*!
    \typealias QAxBase::PropertyBag

    A QMap<QString,QVariant> that can store properties as name:value pairs.
*/

/*!
    Creates a QAxBase object.
*/
QAxBase::QAxBase() = default;

/*!
    Shuts down the COM object and destroys the QAxBase object.

    \sa clear()
*/
QAxBase::~QAxBase()
{

    clear();
}

/*!
    \internal

    Used by subclasses generated with dumpcpp to balance reference count.
*/
void QAxBase::internalRelease()
{
    d->ptr.Reset();
}

/*!
    \internal

    Used by subclasses generated with dumpcpp to implement cast-operators.
*/
void QAxBase::initializeFrom(QAxBase *that)
{
    if (d->ptr)
        return;

    d->ptr = that->d->ptr;
    if (d->ptr) {
        d->initialized = true;
    }
}

bool QAxBase::setControl(const QString &c)
{
    if (!c.compare(d->ctrl, Qt::CaseInsensitive))
        return !d->ctrl.isEmpty();

    QString search = c;
    // don't waste time for DCOM requests
    int dcomIDIndex = search.indexOf(QLatin1String("/{"));
    if ((dcomIDIndex == -1 || dcomIDIndex != search.length()-39) && !search.endsWith(QLatin1String("}&"))) {
        QUuid uuid(search);
        if (uuid.isNull()) {
            CLSID clsid;
            HRESULT res = CLSIDFromProgID(reinterpret_cast<const wchar_t *>(c.utf16()), &clsid);
            if (res == S_OK)
                search = QUuid(clsid).toString();
            else {
                QSettings controls(QLatin1String("HKEY_LOCAL_MACHINE\\Software\\Classes\\"), QSettings::NativeFormat);
                search = controls.value(c + QLatin1String("/CLSID/Default")).toString();
                if (search.isEmpty()) {
                    controls.beginGroup(QLatin1String("/CLSID"));
                    const QStringList clsids = controls.childGroups();
                    for (const QString &clsid : clsids) {
                        const QString name = controls.value(clsid + QLatin1String("/Default")).toString();
                        if (name == c) {
                            search = clsid;
                            break;
                        }
                    }
                    controls.endGroup();
                }
            }
        }
        if (search.isEmpty())
            search = c;
    }

    if (!search.compare(d->ctrl, Qt::CaseInsensitive))
        return !d->ctrl.isEmpty();

    clear();
    d->ctrl = search;

    d->tryCache = true;
    if (!initialize(&d->ptr))
        d->initialized = true;
    if (isNull()) {
        qWarning("QAxBase::setControl: requested control %s could not be instantiated", c.toLatin1().data());
        clear();
        return false;
    }
    return true;
}

/*!
    Returns the ActiveX control.
*/
QString QAxBase::control() const
{
    return d->ctrl;
}

/*!
    Disables the event sink implementation for this ActiveX container.
    If you don't intend to listen to the ActiveX control's events use
    this function to speed up the meta object generation.

    Some ActiveX controls might be unstable when connected to an event
    sink. To get OLE events you must use standard COM methods to
    register your own event sink. Use queryInterface() to get access
    to the raw COM object.

    Note that this function should be called immediately after
    construction of the object.
*/
void QAxBase::disableEventSink()
{
    d->useEventSink = false;
}

/*!
    \since 5.13

    \return the context the ActiveX control will run in (default CLSCTX_SERVER).
*/
ulong QAxBase::classContext() const
{
    return d->classContext;
}

/*!
    \since 5.13

    Sets the context the ActiveX control will run in to \a classContext

    Affects the "dwClsContext" argument when calling CoCreateInstance.
    This can be used to control in-proc vs. out-of-proc startup for controls
    supporting both alternatives. Also, it can be used to modify/reduce control
    permissions when used with CLSCTX_ENABLE_CLOAKING and an impersonation token.

    Note that this function must be called before setControl() to have any
    effect.
*/
void QAxBase::setClassContext(ulong classContext)
{
    d->classContext = classContext;
}

/*!
    Disables the meta object generation for this ActiveX container.
    This also disables the event sink and class info generation. If
    you don't intend to use the Qt meta object implementation call
    this function to speed up instantiation of the control. You will
    still be able to call the object through \l dynamicCall(), but
    signals, slots and properties will not be available with QObject
    APIs.

    Some ActiveX controls might be unstable when used with OLE
    automation. Use standard COM methods to use those controls through
    the COM interfaces provided by queryInterface().

    Note that this function must be called immediately after
    construction of the object.
*/
void QAxBase::disableMetaObject()
{
    d->useMetaObject    = false;
    d->useEventSink     = false;
    d->useClassInfo     = false;
}

/*!
    Disables the class info generation for this ActiveX container. If
    you don't require any class information about the ActiveX control
    use this function to speed up the meta object generation.

    Note that this function must be called immediately after
    construction of the object
*/
void QAxBase::disableClassInfo()
{
    d->useClassInfo = false;
}

/*!
    Disconnects and destroys the COM object.

    If you reimplement this function you must also reimplement the
    destructor to call clear(), and call this implementation at the
    end of your clear() function.
*/
void QAxBase::clear()
{
    for (auto it = d->eventSink.cbegin(), end = d->eventSink.cend(); it != end; ++it) {
        if (QAxEventSink *eventSink = it.value()) {
            eventSink->unadvise();
            eventSink->Release();
        }
    }
    d->eventSink.clear();
    if (d->disp) {
        d->disp.Reset();
    }
    if (d->ptr) {
        d->ptr.Reset();
        d->initialized = false;
    }

    d->ctrl.clear();

    if (!d->cachedMetaObject)
        delete d->metaobj;
    d->metaobj = nullptr;
}

/*!
    \since 4.1

    Returns the list of verbs that the COM object can execute. If
    the object does not implement IOleObject, or does not support
    any verbs, then this function returns an empty stringlist.

    Note that the OLE default verbs (OLEIVERB_SHOW etc) are not
    included in the list.
*/
QStringList QAxBase::verbs() const
{
    if (!d->ptr)
        return QStringList();

    if (d->verbs.isEmpty()) {
        ComPtr<IOleObject> ole;
        d->ptr->QueryInterface(IID_IOleObject, &ole);
        if (ole) {
            ComPtr<IEnumOLEVERB> enumVerbs;
            ole->EnumVerbs(&enumVerbs);
            if (enumVerbs) {
                enumVerbs->Reset();
                ULONG c;
                OLEVERB verb;
                while (enumVerbs->Next(1, &verb, &c) == S_OK) {
                    if (!verb.lpszVerbName)
                        continue;
                    QString verbName = QString::fromWCharArray(verb.lpszVerbName);
                    if (!verbName.isEmpty())
                        d->verbs.insert(verbName, verb.lVerb);
                }
            }
        }
    }

    return d->verbs.keys();
}

/*!
    \internal
*/

long QAxBase::indexOfVerb(const QString &verb) const
{
    return d->verbs.value(verb);
}

/*!
    This virtual function is called by setControl() and creates the
    requested COM object. \a ptr is set to the object's IUnknown
    implementation. The function returns true if the object
    initialization succeeded; otherwise the function returns false.

    The default implementation interprets the string returned by
    control(), and calls initializeRemote(), initializeLicensed()
    or initializeActive() if the string matches the respective
    patterns. If control() is the name of an existing file,
    initializeFromFile() is called. If no pattern is matched, or
    if remote or licensed initialization fails, CoCreateInstance
    is used directly to create the object.

    See the \l QAxBaseWidget::control property documentation for details about
    supported patterns.

    The interface returned in \a ptr must be referenced exactly once
    when this function returns. The interface provided by e.g.
    CoCreateInstance is already referenced, and there is no need to
    reference it again.
*/
bool QAxBase::initialize(IUnknown **ptr)
{
    using QWindowsApplication = QNativeInterface::Private::QWindowsApplication;

    if (*ptr || control().isEmpty())
        return false;

    // Request asynchronous expose events to be used when application uses ActiveQt objects.
    // Otherwise painter can get corrupted if Invoke or some other COM method that cause Windows
    // messages to be processed is called during an existing paint operation when WM_PAINT is
    // also in the queue.
    if (auto nativeWindowsApp = dynamic_cast<QWindowsApplication *>(QGuiApplicationPrivate::platformIntegration()))
        nativeWindowsApp->setAsyncExpose(true);

    *ptr = nullptr;

    bool res = false;

    const QString ctrl(d->ctrl);
    if (ctrl.contains(QLatin1String("/{"))) // DCOM request
        res = initializeRemote(ptr);
    else if (ctrl.contains(QLatin1String("}:"))) // licensed control
        res = initializeLicensed(ptr);
    else if (ctrl.contains(QLatin1String("}&"))) // running object
        res = initializeActive(ptr);
    else if (QFile::exists(ctrl)) // existing file
        res = initializeFromFile(ptr);

    if (!res) { // standard
        HRESULT hres = CoCreateInstance(QUuid(ctrl), nullptr, d->classContext, IID_IUnknown,
                                        reinterpret_cast<void **>(ptr));
        res = S_OK == hres;
#ifndef QT_NO_DEBUG
        if (!res)
            qErrnoWarning(hres, "CoCreateInstance failure");
#endif
    }

    return *ptr != nullptr;
}

/*!
    \internal
*/
void QAxBase::axBaseInit(QAxBasePrivate *b, IUnknown *iface)
{
    d = b;
    d->q = this;
    d->ptr = iface;
    if (d->ptr) {
        d->initialized = true;
    }
}

/*!
    Creates an instance of a licensed control, and returns the IUnknown interface
    to the object in \a ptr. This functions returns true if successful, otherwise
    returns false.

    This function is called by initialize() if the control string contains the
    substring "}:". The license key needs to follow this substring.

    \sa initialize()
*/
bool QAxBase::initializeLicensed(IUnknown** ptr)
{
    const QString ctl = control();
    int at = ctl.lastIndexOf(QLatin1String("}:"));

    QString clsid(ctl.left(at));
    QString key(ctl.mid(at+2));

    ComPtr<IClassFactory> factory;
    CoGetClassObject(QUuid(clsid), CLSCTX_SERVER, nullptr, IID_IClassFactory, &factory);
    if (!factory)
        return false;
    initializeLicensedHelper(factory.Get(), key, ptr);

    return *ptr != nullptr;
}

/* \internal
    Called by initializeLicensed and initializedRemote to create an object
    via IClassFactory2.
*/
bool QAxBase::initializeLicensedHelper(void *f, const QString &key, IUnknown **ptr)
{
    IClassFactory *factory = reinterpret_cast<IClassFactory *>(f);
    ComPtr<IClassFactory2> factory2;
    factory->QueryInterface(IID_IClassFactory2, &factory2);
    if (factory2) {
        BSTR bkey = QStringToBSTR(key);
        HRESULT hres = factory2->CreateInstanceLic(nullptr, nullptr, IID_IUnknown, bkey,
                                                   reinterpret_cast<void **>(ptr));
        SysFreeString(bkey);
#ifdef QT_DEBUG
        LICINFO licinfo;
        licinfo.cbLicInfo = sizeof(LICINFO);
        factory2->GetLicInfo(&licinfo);

        if (hres != S_OK) {
            SetLastError(DWORD(hres));
            qErrnoWarning("CreateInstanceLic failed");
            if (!licinfo.fLicVerified) {
                qWarning("Wrong license key specified, and machine is not fully licensed.");
            } else if (licinfo.fRuntimeKeyAvail) {
                BSTR licenseKey;
                factory2->RequestLicKey(0, &licenseKey);
                QString qlicenseKey = QString::fromWCharArray(licenseKey);
                SysFreeString(licenseKey);
                qWarning("Use license key is '%s' to create object on unlicensed machine.",
                    qlicenseKey.toLatin1().constData());
            }
        } else if (licinfo.fLicVerified) {
            qWarning("Machine is fully licensed for '%s'", control().toLatin1().constData());
            if (licinfo.fRuntimeKeyAvail) {
                BSTR licenseKey;
                factory2->RequestLicKey(0, &licenseKey);
                QString qlicenseKey = QString::fromWCharArray(licenseKey);
                SysFreeString(licenseKey);

                if (qlicenseKey != key)
                    qWarning("Runtime license key is '%s'", qlicenseKey.toLatin1().constData());
            }
        }
#else
        Q_UNUSED(hres);
#endif
    } else {  // give it a shot without license
        factory->CreateInstance(nullptr, IID_IUnknown, reinterpret_cast<void **>(ptr));
    }
    return *ptr != nullptr;
}


/*!
    Connects to an active instance running on the current machine, and returns the
    IUnknown interface to the running object in \a ptr. This function returns true
    if successful, otherwise returns false.

    This function is called by initialize() if the control string contains the
    substring "}&".

    \sa initialize()
*/
bool QAxBase::initializeActive(IUnknown** ptr)
{
    const QString ctl = control();
    int at = ctl.lastIndexOf(QLatin1String("}&"));
    QString clsid(ctl.left(at));

    GetActiveObject(QUuid(clsid), nullptr, ptr);

    return *ptr != nullptr;
}

#ifdef Q_CC_GNU
#   ifndef OLEPENDER_NONE
#   define OLERENDER_NONE 0
#   endif
#endif

/*!
    Creates the COM object handling the filename in the control property, and
    returns the IUnknown interface to the object in \a ptr. This function returns
    true if successful, otherwise returns false.

    This function is called by initialize() if the control string is the name of
    an existing file.

    \sa initialize()
*/
bool QAxBase::initializeFromFile(IUnknown** ptr)
{
    ComPtr<IStorage> storage;
    ComPtr<ILockBytes> bytes;
    HRESULT hres = ::CreateILockBytesOnHGlobal(nullptr, TRUE, &bytes);
    hres = ::StgCreateDocfileOnILockBytes(bytes.Get(), STGM_SHARE_EXCLUSIVE|STGM_CREATE|STGM_READWRITE, 0, &storage);

    hres = OleCreateFromFile(CLSID_NULL, reinterpret_cast<const wchar_t*>(control().utf16()),
                             IID_IUnknown, OLERENDER_NONE, nullptr, nullptr, storage.Get(),
                             reinterpret_cast<void **>(ptr));

    return hres == S_OK;
}


// There seams to be a naming problem in mingw headers
#if defined(Q_CC_GNU) && !defined(COAUTHIDENTITY) && !defined(__MINGW64_VERSION_MAJOR)
#define COAUTHIDENTITY AUTH_IDENTITY
#endif

// Set string values to COAUTHIDENTITY fields. Use data(), no need for 0-termination.
static inline void setIdentityString(const QString &v, ULONG &length, USHORT *&target)
{
    length = ULONG(v.size());
    target = length > 0
        ? const_cast<USHORT *>(reinterpret_cast<const USHORT *>(v.data()))
        : nullptr;
}

/*!
    Creates the instance on a remote server, and returns the IUnknown interface
    to the object in \a ptr. This function returns true if successful, otherwise
    returns false.

    This function is called by initialize() if the control string contains the
    substring "/{". The information about the remote machine needs to be provided
    in front of the substring.

    \sa initialize()
*/
bool QAxBase::initializeRemote(IUnknown** ptr)
{
    const QString ctl = control();
    int at = ctl.lastIndexOf(QLatin1String("/{"));

    QString server(ctl.left(at));
    QString clsid(ctl.mid(at+1));

    QString user;
    QString domain;
    QString passwd;
    QString key;

    at = server.indexOf(QChar::fromLatin1('@'));
    if (at != -1) {
        user = server.left(at);
        server.remove(0, at + 1);

        at = user.indexOf(QChar::fromLatin1(':'));
        if (at != -1) {
            passwd = user.mid(at+1);
            user.truncate(at);
        }
        at = user.indexOf(QChar::fromLatin1('/'));
        if (at != -1) {
            domain = user.left(at);
            user.remove(0, at + 1);
        }
    }

    at = clsid.lastIndexOf(QLatin1String("}:"));
    if (at != -1) {
        key = clsid.mid(at+2);
        clsid.truncate(at);
    }

    d->ctrl = server + QChar::fromLatin1('/') + clsid;
    if (!key.isEmpty())
        d->ctrl = d->ctrl + QChar::fromLatin1(':') + key;

    COAUTHIDENTITY authIdentity;
    setIdentityString(user, authIdentity.UserLength, authIdentity.User);
    setIdentityString(domain, authIdentity.DomainLength,  authIdentity.Domain);
    setIdentityString(passwd, authIdentity.PasswordLength, authIdentity.Password);
    authIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

    COAUTHINFO authInfo;
    authInfo.dwAuthnSvc = RPC_C_AUTHN_WINNT;
    authInfo.dwAuthzSvc = RPC_C_AUTHZ_NONE;
    authInfo.pwszServerPrincName = nullptr;
    authInfo.dwAuthnLevel = RPC_C_AUTHN_LEVEL_DEFAULT;
    authInfo.dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
    authInfo.pAuthIdentityData = &authIdentity;
    authInfo.dwCapabilities = 0;

    COSERVERINFO serverInfo;
    serverInfo.dwReserved1 = 0;
    serverInfo.dwReserved2 = 0;
    serverInfo.pAuthInfo = &authInfo;
    serverInfo.pwszName = qaxQString2MutableOleChars(server);

    ComPtr<IClassFactory> factory;
    HRESULT res = CoGetClassObject(QUuid(clsid), CLSCTX_REMOTE_SERVER, &serverInfo,
                                   IID_IClassFactory, &factory);
    if (factory) {
        if (!key.isEmpty())
            initializeLicensedHelper(factory.Get(), key, ptr);
        else
            res = factory->CreateInstance(nullptr, IID_IUnknown, reinterpret_cast<void **>(ptr));
    }
#ifndef QT_NO_DEBUG
    if (res != S_OK)
        qErrnoWarning(res, "initializeRemote Failed");
#endif

    return res == S_OK;
}

/*!
    Requests the interface \a uuid from the COM object and sets the
    value of \a iface to the provided interface, or to 0 if the
    requested interface could not be provided.

    Returns the result of the QueryInterface implementation of the COM object.

    \sa control()
*/
long QAxBase::queryInterface(const QUuid &uuid, void **iface) const
{
    *iface = nullptr;
    if (!d->ptr) {
        const_cast<QAxBase *>(this)->initialize(&d->ptr);
        d->initialized = true;
    }

    if (d->ptr && !uuid.isNull())
        return d->ptr->QueryInterface(uuid, iface);

    return E_NOTIMPL;
}

class MetaObjectGenerator
{
public:
    MetaObjectGenerator(QAxBase *ax, QAxBasePrivate *dptr);
    MetaObjectGenerator(ITypeLib *typelib, ITypeInfo *typeinfo);
    ~MetaObjectGenerator() = default;

    QMetaObject *metaObject(const QMetaObject *parentObject, const QByteArray &className = QByteArray());

    void readClassInfo();
    void readEnumInfo();
    void readInterfaceInfo();
    void readFuncsInfo(ITypeInfo *typeinfo, ushort nFuncs);
    void readVarsInfo(ITypeInfo *typeinfo, ushort nVars);
    void readEventInfo();
    void readEventInterface(ITypeInfo *eventinfo, IConnectionPoint *cpoint);

    inline void addClassInfo(const QByteArray &key, const  QByteArray &value)
    {
        classinfo_list.insert(key, value);
    }

private:
    using ByteArrayIntPair = QPair<QByteArray, int>;
    using ByteArrayIntPairList = QList<ByteArrayIntPair>;

    void init();


    QMetaObject *tryCache();

    QByteArray createPrototype(FUNCDESC *funcdesc, ITypeInfo *typeinfo, const QByteArrayList &names,
                               QByteArray &type, QByteArrayList &parameters);

    QByteArray usertypeToString(const TYPEDESC &tdesc, ITypeInfo *info, const QByteArray &function);
    QByteArray guessTypes(const TYPEDESC &tdesc, ITypeInfo *info, const QByteArray &function);

    // ActiveQt's extensions to the PropertyFlags defined in qmetaobject_p.h.
    // This will break if new overlapping flags are added in qmetaobject_p.h!
    enum ProperyFlags  {
        RequestingEdit          = 0x01000000,
        Bindable                = 0x02000000
    };

    static inline bool checkForValidFunctionParameters(const QByteArray &name, const QList<QByteArray> &parameters)
    {
        qsizetype leftBrace = name.indexOf('(');
        if (leftBrace == -1)
            return false;
        qsizetype rightBrace = name.indexOf(')', leftBrace + 1);
        if (rightBrace == -1)
            return false;

        if (rightBrace - leftBrace == 1)
            return true;

        const QList<QByteArray> parameterTypes =
                name.mid(leftBrace + 1, rightBrace - 1 - leftBrace).split(',');
        if (parameterTypes.length() != parameters.length())
            return false;

        for (const auto &parameterType : parameterTypes)
            if (parameterType.isEmpty())
                return false;

        return true;
    };

    static inline QByteArrayList paramList(const QByteArray &prototype)
    {
        QByteArray parameters = prototype.mid(prototype.indexOf('(') + 1);
        parameters.truncate(parameters.length() - 1);
        if (parameters.isEmpty() || parameters == "void")
            return {};
        return parameters.split(',');
    }

    inline QByteArray replaceType(const QByteArray &type)
    {
        if (type.isEmpty())
            return QByteArray("void");
        int i = 0;
        while (type_conversion[i][0]) {
            int len = int(strlen(type_conversion[i][0]));
            int ti;
            if ((ti = type.indexOf(type_conversion[i][0])) != -1) {
                QByteArray rtype(type);
                rtype.replace(ti, len, type_conversion[i][1]);
                return rtype;
            }
            ++i;
        }
        return type;
    }

    QByteArray replacePrototype(const QByteArray &prototype)
    {
        QByteArray proto(prototype);

        const auto plist = paramList(prototype);
        for (const QByteArray &param : plist) {
            if (param != replaceType(param)) {
                int type = 0;
                while (type_conversion[type][0]) {
                    int paren = proto.indexOf('(');
                    while ((paren = proto.indexOf(type_conversion[type][0])) != -1) {
                        proto.replace(paren, int(qstrlen(type_conversion[type][0])),
                                      type_conversion[type][1]);
                    }
                    ++type;
                }
                break;
            }
        }

        return proto;
    }

    QMap<QByteArray, QByteArray> classinfo_list;

    inline bool hasClassInfo(const char *key)
    {
        return classinfo_list.contains(key);
    }

    struct Method {
        QByteArray type;
        QByteArray parameters;
        int attributes = 0;
        QByteArray realPrototype;
    };
    QMap<QByteArray, Method> signal_list;
    inline void addSignal(const QByteArray &prototype, const QByteArray &parameters)
    {
        QByteArray proto(replacePrototype(prototype));

        Method &signal = signal_list[proto];
        signal.type = "void";
        signal.parameters = parameters;
        if (proto != prototype)
            signal.realPrototype = prototype;
    }

    void addChangedSignal(const QByteArray &function, const QByteArray &type, int memid);

    inline bool hasSignal(const QByteArray &prototype)
    {
        return signal_list.contains(prototype);
    }

    QMap<QByteArray, Method> slot_list;
    inline void addSlot(const QByteArray &type, const QByteArray &prototype, const QByteArray &parameters,
                        int attributes = 0)
    {
        QByteArray proto = replacePrototype(prototype);

        Method &slot = slot_list[proto];

        slot.type = replaceType(type);
        slot.parameters = parameters;
        slot.attributes = attributes;
        if (proto != prototype)
            slot.realPrototype = prototype;
    }

    void addSetterSlot(const QByteArray &property);

    inline bool hasSlot(const QByteArray &prototype)
    {
        return slot_list.contains(prototype);
    }

    static int aggregateParameterCount(const QMap<QByteArray, Method> &map);

    struct Property {
        Property() = default;

        QByteArray type;
        uint flags = 0;
        QByteArray realType;
    };
    QMap<QByteArray, Property> property_list;
    void addProperty(const QByteArray &type, const QByteArray &name, uint flags)
    {
        QByteArray propertyType(type);
        if (propertyType.endsWith('&'))
            propertyType.chop(1);

        Property &prop = property_list[name];
        if (!propertyType.isEmpty() && propertyType != "HRESULT") {
            prop.type = replaceType(propertyType);
            if (prop.type != propertyType)
                prop.realType = propertyType;
        }
        if (flags & Writable)
            flags |= Stored;
        prop.flags |= flags;
    }

    inline bool hasProperty(const QByteArray &name)
    {
        return property_list.contains(name);
    }

    inline QByteArray propertyType(const QByteArray &name)
    {
        return property_list.value(name).type;
    }

    QMap<QByteArray, ByteArrayIntPairList> enum_list;

    inline void addEnumValue(const QByteArray &enumname, const QByteArray &key, int value)
    {
        enum_list[enumname].append(ByteArrayIntPair(key, value));
    }

    inline bool hasEnum(const QByteArray &enumname)
    {
        return enum_list.contains(enumname);
    }

    using BuilderMethodCreationFunc = QMetaMethodBuilder (QMetaObjectBuilder::*)(const QByteArray &);

    static void addMetaMethod(QMetaObjectBuilder &builder,
                              BuilderMethodCreationFunc creationFunc,
                              const QByteArray &name,
                              const QByteArray &parameters,
                              const QByteArray &returnType = QByteArray(),
                              int attributes = 0);
    static void buildMethods(const QMap<QByteArray, Method> &list,
                             QMetaObjectExtra &moExtra,
                             QMetaObjectBuilder &builder,
                             BuilderMethodCreationFunc creationFunc);

    QAxBase *that = nullptr;
    QAxBasePrivate *d = nullptr;

    IDispatch *disp = nullptr;
    ComPtr<ITypeInfo> dispInfo;
    ComPtr<ITypeInfo> classInfo;
    ComPtr<ITypeLib> typelib;
    QByteArray current_typelib;

    QSettings iidnames;
    QString cacheKey;
    QByteArray debugInfo;

    QUuid iid_propNotifySink;

    friend QMetaObject *qax_readClassInfo(ITypeLib *typeLib, ITypeInfo *classInfo, const QMetaObject *parentObject);
};

QMetaObject *qax_readEnumInfo(ITypeLib *typeLib, const QMetaObject *parentObject)
{
    MetaObjectGenerator generator(typeLib, nullptr);

    generator.readEnumInfo();
    return generator.metaObject(parentObject, "EnumInfo");
}

QMetaObject *qax_readInterfaceInfo(ITypeLib *typeLib, ITypeInfo *typeInfo, const QMetaObject *parentObject)
{
    MetaObjectGenerator generator(typeLib, typeInfo);

    QString className;
    BSTR bstr;
    if (S_OK != typeInfo->GetDocumentation(-1, &bstr, nullptr, nullptr, nullptr))
        return nullptr;

    className = QString::fromWCharArray(bstr);
    SysFreeString(bstr);

    generator.readEnumInfo();
    generator.readFuncsInfo(typeInfo, 0);
    generator.readVarsInfo(typeInfo, 0);

    return generator.metaObject(parentObject, className.toLatin1());
}

QMetaObject *qax_readClassInfo(ITypeLib *typeLib, ITypeInfo *classInfo, const QMetaObject *parentObject)
{
    MetaObjectGenerator generator(typeLib, nullptr);
    QString className;
    BSTR bstr;
    if (S_OK != classInfo->GetDocumentation(-1, &bstr, nullptr, nullptr, nullptr))
        return nullptr;

    className = QString::fromWCharArray(bstr);
    SysFreeString(bstr);

    generator.readEnumInfo();

    TYPEATTR *typeattr;
    classInfo->GetTypeAttr(&typeattr);
    if (typeattr) {
        const UINT nInterfaces = typeattr->cImplTypes;
        classInfo->ReleaseTypeAttr(typeattr);

        for (UINT index = 0; index < nInterfaces; ++index) {
            HREFTYPE refType;
            if (S_OK != classInfo->GetRefTypeOfImplType(index, &refType))
                continue;

            int flags = 0;
            classInfo->GetImplTypeFlags(index, &flags);
            if (flags & IMPLTYPEFLAG_FRESTRICTED)
                continue;

            ComPtr<ITypeInfo> interfaceInfo;
            classInfo->GetRefTypeInfo(refType, &interfaceInfo);
            if (!interfaceInfo)
                continue;

            interfaceInfo->GetDocumentation(-1, &bstr, nullptr, nullptr, nullptr);
            QString interfaceName = QString::fromWCharArray(bstr);
            SysFreeString(bstr);
            QByteArray key;

            TYPEATTR *typeattr = nullptr;
            interfaceInfo->GetTypeAttr(&typeattr);

            if (flags & IMPLTYPEFLAG_FSOURCE) {
                if (typeattr && !(typeattr->wTypeFlags & TYPEFLAG_FHIDDEN))
                    key = "Event Interface " + QByteArray::number(index);
                generator.readEventInterface(interfaceInfo.Get(), nullptr);
            } else {
                if (typeattr && !(typeattr->wTypeFlags & TYPEFLAG_FHIDDEN))
                    key = "Interface " + QByteArray::number(index);
                generator.readFuncsInfo(interfaceInfo.Get(), 0);
                generator.readVarsInfo(interfaceInfo.Get(), 0);
            }
            if (!key.isEmpty())
                generator.addClassInfo(key.data(), interfaceName.toLatin1());

            if (typeattr)
                interfaceInfo->ReleaseTypeAttr(typeattr);
        }
    }

    return generator.metaObject(parentObject, className.toLatin1());
}

void qax_deleteMetaObject(QMetaObject *metaObject)
{
    free(metaObject); // Result of QMetaObjectBuilder::toMetaObject() needs free()
}

MetaObjectGenerator::MetaObjectGenerator(QAxBase *ax, QAxBasePrivate *dptr)
    : that(ax), d(dptr),
      iidnames(QLatin1String("HKEY_LOCAL_MACHINE\\Software\\Classes"), QSettings::NativeFormat)
{
    init();
}

MetaObjectGenerator::MetaObjectGenerator(ITypeLib *tlib, ITypeInfo *tinfo)
    : dispInfo(tinfo), typelib(tlib),
      iidnames(QLatin1String("HKEY_LOCAL_MACHINE\\Software\\Classes"), QSettings::NativeFormat)
{
    init();

    if (typelib) {
        BSTR bstr;
        typelib->GetDocumentation(-1, &bstr, nullptr, nullptr, nullptr);
        current_typelib = QString::fromWCharArray(bstr).toLatin1();
        SysFreeString(bstr);
    }
    readClassInfo();
}

void MetaObjectGenerator::init()
{
    if (d)
        disp = d->dispatch();

    iid_propNotifySink = IID_IPropertyNotifySink;
}

bool qax_dispatchEqualsIDispatch = true;
QByteArrayList qax_qualified_usertypes;
// Value strings for enums
QHash<QByteArray, QByteArray> qax_enum_values;

// Read enum values
QList<QPair<QByteArray, int> > qax_readEnumValues(ITypeLib *typelib, UINT index)
{
    QList<QPair<QByteArray, int> > result;

    // Get the type information for the enum
    ComPtr<ITypeInfo> enuminfo;
    typelib->GetTypeInfo(index, &enuminfo);
    if (!enuminfo)
        return result;

    // Get the attributes of the enum type
    TYPEATTR *typeattr = nullptr;
    enuminfo->GetTypeAttr(&typeattr);
    if (typeattr == nullptr) {
        return result;
    }

    // Get all values of the enumeration
    result.reserve(typeattr->cVars);
    for (UINT vd = 0; vd < typeattr->cVars; ++vd) {
        VARDESC *vardesc = nullptr;
        enuminfo->GetVarDesc(vd, &vardesc);
        if (vardesc != nullptr) {
            if (vardesc->varkind == VAR_CONST) {
                const int value = vardesc->lpvarValue->lVal;
                QByteArray valueName = qaxTypeInfoName(enuminfo.Get(),  vardesc->memid);
                result.append({valueName, value});
            }
            enuminfo->ReleaseVarDesc(vardesc);
        }
    }
    enuminfo->ReleaseTypeAttr(typeattr);
    return result;
}

// Format enum values into a string v1=1, v2=2,...
static QByteArray enumValueString(ITypeLib *typelib, UINT index)
{
    QByteArray result;
    const auto enumValues = qax_readEnumValues(typelib, index);
    const auto last = enumValues.size() - 1;
    for (qsizetype i = 0; i <= last; ++i) {
        const auto &enumValue = enumValues.at(i);
        result += "        " + enumValue.first + '='
                  + QByteArray::number(enumValue.second);
        if (i < last)
            result += ',';
        result += '\n';
    }
    return result;
}

QByteArray MetaObjectGenerator::usertypeToString(const TYPEDESC &tdesc, ITypeInfo *info, const QByteArray &function)
{
    HREFTYPE usertype = tdesc.hreftype;
    if (tdesc.vt != VT_USERDEFINED)
        return nullptr;

    QByteArray typeName;
    ComPtr<ITypeInfo> usertypeinfo;
    info->GetRefTypeInfo(usertype, &usertypeinfo);
    if (usertypeinfo) {
        ComPtr<ITypeLib> usertypelib;
        UINT index;
        usertypeinfo->GetContainingTypeLib(&usertypelib, &index);
        if (usertypelib) {
            // get type library name
            QBStr typelibname;
            usertypelib->GetDocumentation(-1, &typelibname, nullptr, nullptr, nullptr);
            QByteArray typeLibName = typelibname.str().toLatin1();

            // get type name
            QBStr usertypename;
            usertypelib->GetDocumentation(INT(index), &usertypename, nullptr, nullptr, nullptr);
            QByteArray userTypeName = usertypename.str().toLatin1();

            if (hasEnum(userTypeName)) // known enum?
                typeName = userTypeName;
            else if (userTypeName == "OLE_COLOR" || userTypeName == "VB_OLE_COLOR")
                typeName = "QColor";
            else if (userTypeName == "IFontDisp" || userTypeName == "IFontDisp*" || userTypeName == "IFont" || userTypeName == "IFont*")
                typeName = "QFont";
            else if (userTypeName == "Picture" || userTypeName == "Picture*")
                typeName = "QPixmap";

            if (typeName.isEmpty()) {
                TYPEATTR *typeattr = nullptr;
                usertypeinfo->GetTypeAttr(&typeattr);
                if (typeattr) {
                    switch(typeattr->typekind) {
                    case TKIND_ALIAS:
                        userTypeName = guessTypes(typeattr->tdescAlias, usertypeinfo.Get(), function);
                        break;
                    case TKIND_DISPATCH:
                    case TKIND_COCLASS:
                        if (qax_dispatchEqualsIDispatch) {
                            userTypeName = "IDispatch";
                        } else {
                            if (typeLibName != current_typelib)
                                userTypeName.prepend(typeLibName + "::");
                            if (!qax_qualified_usertypes.contains(userTypeName))
                                qax_qualified_usertypes << userTypeName;
                        }
                        break;
                    case TKIND_ENUM:
                        if (typeLibName != current_typelib) {
                            userTypeName.prepend(typeLibName + "::");
                            // For dumpcpp
                            qax_enum_values.insert(userTypeName, enumValueString(usertypelib.Get(), index));
                        }
                        if (!qax_qualified_usertypes.contains("enum " + userTypeName))
                            qax_qualified_usertypes << "enum " + userTypeName;
                        break;
                    case TKIND_INTERFACE:
                        if (typeLibName != current_typelib)
                            userTypeName.prepend(typeLibName + "::");
                        if (!qax_qualified_usertypes.contains(userTypeName))
                            qax_qualified_usertypes << userTypeName;
                        break;
                    case TKIND_RECORD:
                        if (!qax_qualified_usertypes.contains("struct " + userTypeName))
                            qax_qualified_usertypes << "struct "+ userTypeName;
                        break;
                    default:
                        break;
                    }
                }

                usertypeinfo->ReleaseTypeAttr(typeattr);
                typeName = userTypeName;
            }
        }
    }

    return typeName;
}

#define VT_UNHANDLED(x) case VT_##x: qWarning("QAxBase: Unhandled type %s", #x); str = #x; break;

QByteArray MetaObjectGenerator::guessTypes(const TYPEDESC &tdesc, ITypeInfo *info, const QByteArray &function)
{
    QByteArray str;
    switch (tdesc.vt) {
    case VT_EMPTY:
        break;
    case VT_VOID:
        str = "void";
        break;
    case VT_LPWSTR:
        str = "wchar_t *";
        break;
    case VT_BSTR:
        str = "QString";
        break;
    case VT_BOOL:
        str = "bool";
        break;
    case VT_I1:
        str = "char";
        break;
    case VT_I2:
        str = "short";
        break;
    case VT_I4:
    case VT_INT:
        str = "int";
        break;
    case VT_I8:
        str = "qlonglong";
        break;
    case VT_UI1:
    case VT_UI2:
    case VT_UI4:
    case VT_UINT:
        str = "uint";
        break;
    case VT_UI8:
        str = "qulonglong";
        break;
    case VT_CY:
        str = "qlonglong";
        break;
    case VT_R4:
        str = "float";
        break;
    case VT_R8:
        str = "double";
        break;
    case VT_DATE:
        str = "QDateTime";
        break;
    case VT_DISPATCH:
        str = "IDispatch*";
        break;
    case VT_VARIANT:
        str = "QVariant";
        break;
    case VT_UNKNOWN:
        str = "IUnknown*";
        break;
    case VT_HRESULT:
        str = "HRESULT";
        break;
    case VT_PTR:
        str = guessTypes(*tdesc.lptdesc, info, function);
        switch(tdesc.lptdesc->vt) {
        case VT_VOID:
            str = "void*";
            break;
        case VT_VARIANT:
        case VT_BSTR:
        case VT_I1:
        case VT_I2:
        case VT_I4:
        case VT_I8:
        case VT_UI1:
        case VT_UI2:
        case VT_UI4:
        case VT_UI8:
        case VT_BOOL:
        case VT_R4:
        case VT_R8:
        case VT_INT:
        case VT_UINT:
        case VT_CY:
            str += '&';
            break;
        case VT_PTR:
            if (str == "QFont" || str == "QPixmap") {
                str += '&';
                break;
            } else if (str == "void*") {
                str = "void **";
                break;
            }
            // FALLTHROUGH
        default:
            if (str == "QColor")
                str += '&';
            else if (str == "QDateTime")
                str += '&';
            else if (str == "QVariantList")
                str += '&';
            else if (str == "QByteArray")
                str += '&';
            else if (str == "QStringList")
                str += '&';
            else if (!str.isEmpty() && hasEnum(str))
                str += '&';
            else if (!str.isEmpty() && str != "QFont" && str != "QPixmap" && str != "QVariant")
                str += '*';
        }
        break;
    case VT_SAFEARRAY:
        switch(tdesc.lpadesc->tdescElem.vt) {
        // some shortcuts, and generic support for lists of QVariant-supported types
        case VT_UI1:
            str = "QByteArray";
            break;
        case VT_BSTR:
            str = "QStringList";
            break;
        case VT_VARIANT:
            str = "QVariantList";
            break;
        default:
            str = guessTypes(tdesc.lpadesc->tdescElem, info, function);
            if (!str.isEmpty())
                str = "QList<" + str + '>';
            break;
        }
        break;
    case VT_CARRAY:
        str = guessTypes(tdesc.lpadesc->tdescElem, info, function);
        if (!str.isEmpty()) {
            for (int index = 0; index < tdesc.lpadesc->cDims; ++index)
                str += '[' + QByteArray::number(int(tdesc.lpadesc->rgbounds[index].cElements)) + ']';
        }
        break;
    case VT_USERDEFINED:
        str = usertypeToString(tdesc, info, function);
        break;

    VT_UNHANDLED(FILETIME);
    VT_UNHANDLED(BLOB);
    VT_UNHANDLED(ERROR);
    VT_UNHANDLED(DECIMAL);
    VT_UNHANDLED(LPSTR);
    default:
        break;
    }

    if (tdesc.vt & VT_BYREF)
        str += '&';

    str.replace("&*", "**");
    return str;
}

void MetaObjectGenerator::readClassInfo()
{
    // Read class information
    ComPtr<IProvideClassInfo> provideClassInfo;
    if (d)
        d->ptr->QueryInterface(IID_IProvideClassInfo, &provideClassInfo);
    if (provideClassInfo) {
        provideClassInfo->GetClassInfo(&classInfo);
        TYPEATTR *typeattr = nullptr;
        if (classInfo)
            classInfo->GetTypeAttr(&typeattr);

        QString coClassID;
        if (typeattr) {
            QUuid clsid(typeattr->guid);
            coClassID = clsid.toString().toUpper();
#ifndef QAX_NO_CLASSINFO
            // UUID
            if (d->useClassInfo && !hasClassInfo("CoClass")) {
                QString coClassIDstr = iidnames.value(QLatin1String("/CLSID/") + coClassID + QLatin1String("/Default"), coClassID).toString();
                addClassInfo("CoClass", coClassIDstr.isEmpty() ? coClassID.toLatin1() : coClassIDstr.toLatin1());
                QByteArray version = QByteArray::number(typeattr->wMajorVerNum) + '.' + QByteArray::number(typeattr->wMinorVerNum);
                if (version != "0.0")
                    addClassInfo("Version", version);
            }
#endif
            classInfo->ReleaseTypeAttr(typeattr);
        }

        if (d->tryCache && !coClassID.isEmpty())
            cacheKey = QString::fromLatin1("%1$%2$%3$%4").arg(coClassID)
                .arg(d->useEventSink).arg(d->useClassInfo).arg(int(qax_dispatchEqualsIDispatch));
    }

    UINT index = 0;
    if (disp && !dispInfo)
        disp->GetTypeInfo(index, LOCALE_USER_DEFAULT, &dispInfo);

    if (dispInfo && !typelib)
        dispInfo->GetContainingTypeLib(&typelib, &index);

    if (!typelib && !that->control().isEmpty()) {
        QSettings controls(QLatin1String("HKEY_LOCAL_MACHINE\\Software"), QSettings::NativeFormat);
        QString tlid = controls.value(QLatin1String("/Classes/CLSID/") + that->control() + QLatin1String("/TypeLib/.")).toString();
        QString tlfile;
        if (!tlid.isEmpty()) {
            controls.beginGroup(QLatin1String("/Classes/TypeLib/") + tlid);
            const QStringList versions = controls.childGroups();
            for (const QString &version : versions) {
                tlfile = controls.value(QLatin1Char('/') + version + QLatin1String("/0/win32/.")).toString();
                if (!tlfile.isEmpty())
                    break;
            }
            controls.endGroup();
        } else {
            tlfile = controls.value(QLatin1String("/Classes/CLSID/") + that->control() + QLatin1String("/InprocServer32/.")).toString();
            if (tlfile.isEmpty())
                tlfile = controls.value(QLatin1String("/Classes/CLSID/") + that->control() + QLatin1String("/LocalServer32/.")).toString();
        }
        if (!tlfile.isEmpty()) {
            LoadTypeLib(reinterpret_cast<const OLECHAR *>(tlfile.utf16()), &typelib);
            if (!typelib) {
                tlfile.truncate(tlfile.lastIndexOf(QLatin1Char('.')));
                tlfile += QLatin1String(".tlb");
                LoadTypeLib(reinterpret_cast<const OLECHAR *>(tlfile.utf16()), &typelib);
            }
            if (!typelib) {
                tlfile.truncate(tlfile.lastIndexOf(QLatin1Char('.')));
                tlfile.append(QLatin1String(".olb"));
                LoadTypeLib(reinterpret_cast<const OLECHAR *>(tlfile.utf16()), &typelib);
            }
        }
    }

    if (!classInfo && typelib && that)
        typelib->GetTypeInfoOfGuid(QUuid(that->control()), &classInfo);

    if (classInfo && !dispInfo) {
        TYPEATTR *classAttr;
        classInfo->GetTypeAttr(&classAttr);
        if (classAttr) {
            for (UINT i = 0; i < classAttr->cImplTypes; ++i) {
                int typeFlags = 0;
                classInfo->GetImplTypeFlags(i, &typeFlags);
                if (typeFlags & IMPLTYPEFLAG_FSOURCE)
                    continue;

                HREFTYPE hrefType;
                if (S_OK == classInfo->GetRefTypeOfImplType(i, &hrefType))
                    classInfo->GetRefTypeInfo(hrefType, &dispInfo);
                if (dispInfo) {
                    TYPEATTR *ifaceAttr;
                    dispInfo->GetTypeAttr(&ifaceAttr);
                    WORD typekind = ifaceAttr->typekind;
                    dispInfo->ReleaseTypeAttr(ifaceAttr);

                    if (typekind & TKIND_DISPATCH) {
                        break;
                    } else {
                        dispInfo.Reset();
                    }
                }
            }
            classInfo->ReleaseTypeAttr(classAttr);
        }
    }

    if (!d || !dispInfo || !cacheKey.isEmpty() || !d->tryCache) {
        if (disp && !dispInfo)
            qWarning("%s: IDispatch %p does not provide interface information", Q_FUNC_INFO, disp);
        return;
    }

    TYPEATTR *typeattr = nullptr;
    dispInfo->GetTypeAttr(&typeattr);

    QString interfaceID;
    if (typeattr) {
        QUuid iid(typeattr->guid);
        interfaceID = iid.toString().toUpper();

        dispInfo->ReleaseTypeAttr(typeattr);
        // ### event interfaces!!
        if (!interfaceID.isEmpty())
            cacheKey = QString::fromLatin1("%1$%2$%3$%4").arg(interfaceID)
                .arg(d->useEventSink).arg(d->useClassInfo).arg(int(qax_dispatchEqualsIDispatch));
    }
}

void MetaObjectGenerator::readEnumInfo()
{
    if (!typelib)
        return;

    QUuid libUuid;

    if (d && d->tryCache) {
        TLIBATTR *libAttr = nullptr;
        typelib->GetLibAttr(&libAttr);
        if (libAttr) {
            libUuid = QUuid(libAttr->guid);
            typelib->ReleaseTLibAttr(libAttr);
            enum_list = enum_cache.value(libUuid);
            if (!enum_list.isEmpty())
                return;
        }
    }

    int valueindex = 0;
    QSet<QString> clashCheck;
    int clashIndex = 0;

    int enum_serial = 0;
    UINT index = typelib->GetTypeInfoCount();
    for (UINT i = 0; i < index; ++i) {
        TYPEKIND typekind;
        typelib->GetTypeInfoType(i, &typekind);
        if (typekind == TKIND_ENUM) {
            // Get the values of the enum
            const auto values = qax_readEnumValues(typelib.Get(), i);
            if (values.isEmpty())
                continue;

            // Get the name of the enumeration
            QBStr enumname;
            QByteArray enumName;
            if (typelib->GetDocumentation(INT(i), &enumname, nullptr, nullptr, nullptr) == S_OK)
                enumName = enumname.str().toLatin1();
            else
                enumName = "enum" + QByteArray::number(++enum_serial);

            // Get the attributes of the enum type
            for (const auto &value : values) {
                QByteArray valueName = value.first;
                if (valueName.isEmpty())
                    valueName = "value" + QByteArray::number(valueindex++);
                if (clashCheck.contains(QString::fromLatin1(valueName)))
                    valueName += QByteArray::number(++clashIndex);
                clashCheck.insert(QString::fromLatin1(valueName));
                addEnumValue(enumName, valueName, value.second);
            }
        }
    }

    if (!libUuid.isNull())
        enum_cache.insert(libUuid, enum_list);
}

void MetaObjectGenerator::addChangedSignal(const QByteArray &function, const QByteArray &type, int memid)
{
    QAxEventSink *eventSink = nullptr;
    if (d) {
        eventSink = d->eventSink.value(iid_propNotifySink);
        if (!eventSink && d->useEventSink) {
            eventSink = new QAxEventSink(that);
            d->eventSink.insert(iid_propNotifySink, eventSink);
        }
    }
    // generate changed signal
    QByteArray signalName(function);
    signalName += "Changed";
    QByteArray signalProto = signalName + '(' + replaceType(type) + ')';
    if (!hasSignal(signalProto))
        addSignal(signalProto, function);
    if (eventSink)
        eventSink->addProperty(memid, function, signalProto);
}

void MetaObjectGenerator::addSetterSlot(const QByteArray &property)
{
    QByteArray prototype(property);
    if (isupper(prototype.at(0))) {
        prototype.insert(0, "Set");
    } else {
        prototype[0] = QtMiscUtils::toAsciiUpper(prototype[0]);
        prototype.insert(0, "set");
    }
    const QByteArray type = propertyType(property);
    if (type.isEmpty() || type == "void") {
        qWarning("%s: Invalid property '%s' of type '%s' encountered.",
                 Q_FUNC_INFO, property.constData(), type.constData());
    } else {
        prototype += '(';
        prototype += type;
        prototype += ')';
        if (!hasSlot(prototype))
            addSlot("void", prototype, property);
    }
}

QByteArray MetaObjectGenerator::createPrototype(FUNCDESC *funcdesc, ITypeInfo *typeinfo, const QByteArrayList &names,
                                                QByteArray &type, QByteArrayList &parameters)
{
    const QByteArray &function = names.at(0);
    const QByteArray hresult("HRESULT");
    // get function prototype
    type = guessTypes(funcdesc->elemdescFunc.tdesc, typeinfo, function);
    if ((type.isEmpty() || type == hresult || type == "void") &&
        (funcdesc->invkind == INVOKE_PROPERTYPUT || funcdesc->invkind == INVOKE_PROPERTYPUTREF) &&
        funcdesc->lprgelemdescParam) {
        type = guessTypes(funcdesc->lprgelemdescParam->tdesc, typeinfo, function);
    }

    QByteArray prototype = function + '(';
    if (funcdesc->invkind == INVOKE_FUNC && type == hresult)
        type = nullptr;

    qsizetype p;
    for (p = 1; p < names.size(); ++p) {
        // parameter
        QByteArray paramName = names.at(p);
        bool optional = p > (funcdesc->cParams - funcdesc->cParamsOpt);
        TYPEDESC tdesc = funcdesc->lprgelemdescParam[p-1].tdesc;
        PARAMDESC pdesc = funcdesc->lprgelemdescParam[p-1].paramdesc;

        QByteArray ptype = guessTypes(tdesc, typeinfo, function);
        if (pdesc.wParamFlags & PARAMFLAG_FRETVAL) {
            if (ptype.endsWith('&')) {
                ptype.truncate(ptype.length() - 1);
            } else if (ptype.endsWith("**")) {
                ptype.truncate(ptype.length() - 1);
            }
            type = ptype;
        } else {
            prototype += ptype;
            if (pdesc.wParamFlags & PARAMFLAG_FOUT && !ptype.endsWith('&') && !ptype.endsWith("**"))
                prototype += '&';
            if (optional || pdesc.wParamFlags & PARAMFLAG_FOPT)
                paramName += "=0";
            else if (pdesc.wParamFlags & PARAMFLAG_FHASDEFAULT) {
                // ### get the value from pdesc.pparamdescex
                paramName += "=0";
            }
            parameters << paramName;
        }
        if (p < funcdesc->cParams && !(pdesc.wParamFlags & PARAMFLAG_FRETVAL))
            prototype += ',';
    }

    if (!prototype.isEmpty()) {
        if (prototype.endsWith(',')) {
            if ((funcdesc->invkind == INVOKE_PROPERTYPUT || funcdesc->invkind == INVOKE_PROPERTYPUTREF) &&
                p == funcdesc->cParams) {
                TYPEDESC tdesc = funcdesc->lprgelemdescParam[p-1].tdesc;
                QByteArray ptype = guessTypes(tdesc, typeinfo, function);
                prototype += ptype;
                prototype += ')';
                parameters << "rhs";
            } else {
                prototype[prototype.length()-1] = ')';
            }
        } else {
            prototype += ')';
        }
    }

    return prototype;
}

void MetaObjectGenerator::readFuncsInfo(ITypeInfo *typeinfo, ushort nFuncs)
{
    if (!nFuncs) {
        TYPEATTR *typeattr = nullptr;
        typeinfo->GetTypeAttr(&typeattr);
        if (typeattr) {
            nFuncs = typeattr->cFuncs;
            typeinfo->ReleaseTypeAttr(typeattr);
        }
    }

    // get information about all functions
    for (ushort fd = 0; fd < nFuncs ; ++fd) {
        FUNCDESC *funcdesc = nullptr;
        typeinfo->GetFuncDesc(fd, &funcdesc);
        if (!funcdesc)
            break;

        // parse function description
        const QByteArrayList names = qaxTypeInfoNames(typeinfo, funcdesc->memid);
        if (names.isEmpty()) {
            typeinfo->ReleaseFuncDesc(funcdesc);
            continue;
        }

        // function name
        const auto maxNamesOut = names.size();
        const QByteArray &function = names.at(0);
        if ((maxNamesOut == 3 && function == "QueryInterface") ||
            (maxNamesOut == 1 && function == "AddRef") ||
            (maxNamesOut == 1 && function == "Release") ||
            (maxNamesOut == 9 && function == "Invoke") ||
            (maxNamesOut == 6 && function == "GetIDsOfNames") ||
            (maxNamesOut == 2 && function == "GetTypeInfoCount") ||
            (maxNamesOut == 4 && function == "GetTypeInfo")) {
            typeinfo->ReleaseFuncDesc(funcdesc);
            continue;
        }

        QByteArray type;
        QByteArray prototype;
        QByteArrayList parameters;
        prototype = createPrototype(/*in*/ funcdesc, typeinfo, names, /*out*/type, parameters);

        // get type of function
        switch(funcdesc->invkind) {
        case INVOKE_PROPERTYGET: // property
        case INVOKE_PROPERTYPUT:
        case INVOKE_PROPERTYPUTREF:
            if (funcdesc->cParams - funcdesc->cParamsOpt <= 1) {
                bool dontBreak = false;
                // getter with non-default-parameters -> fall through to function handling
                if (funcdesc->invkind == INVOKE_PROPERTYGET && !parameters.isEmpty() && funcdesc->cParams - funcdesc->cParamsOpt) {
                    dontBreak = true;
                } else {
                    uint flags = Readable;
                    if (funcdesc->invkind != INVOKE_PROPERTYGET)
                        flags |= Writable;
                    if (!(funcdesc->wFuncFlags & (FUNCFLAG_FNONBROWSABLE | FUNCFLAG_FHIDDEN)))
                        flags |= Designable;
                    if (!(funcdesc->wFuncFlags & FUNCFLAG_FRESTRICTED))
                        flags |= Scriptable;
                    if (funcdesc->wFuncFlags & FUNCFLAG_FREQUESTEDIT)
                        flags |= RequestingEdit;
                    if (hasEnum(type))
                        flags |= EnumOrFlag;

                    if (funcdesc->wFuncFlags & FUNCFLAG_FBINDABLE && funcdesc->invkind == INVOKE_PROPERTYGET) {
                        addChangedSignal(function, type, funcdesc->memid);
                        flags |= Bindable;
                    }
                    // Don't generate code for properties without type
                    if (type.isEmpty() || type == "void")
                        break;
                    addProperty(type, function, flags);

                    // more parameters -> function handling
                    if (funcdesc->invkind == INVOKE_PROPERTYGET && funcdesc->cParams)
                        dontBreak = true;
                }

                if (!funcdesc->cParams) {
                    // don't generate slots for incomplete properties
                    if (type.isEmpty())
                        break;

                    // Done for getters
                    if (funcdesc->invkind == INVOKE_PROPERTYGET)
                        break;

                    // generate setter slot
                    if ((funcdesc->invkind == INVOKE_PROPERTYPUT || funcdesc->invkind == INVOKE_PROPERTYPUTREF) &&
                        hasProperty(function)) {
                        addSetterSlot(function);
                        break;
                    }
                } else if ((funcdesc->invkind == INVOKE_PROPERTYPUT || funcdesc->invkind == INVOKE_PROPERTYPUTREF) &&
                           hasProperty(function)) {
                    addSetterSlot(function);
                    // more parameters -> function handling
                    if (funcdesc->cParams > 1)
                        dontBreak = true;
                }
                if (!dontBreak)
                    break;
            }
            if (funcdesc->invkind == INVOKE_PROPERTYPUT || funcdesc->invkind == INVOKE_PROPERTYPUTREF) {
                // remove the typename guessed for property setters
                // its done only for setter's with more than one parameter.
                if (funcdesc->cParams - funcdesc->cParamsOpt > 1) {
                    type.clear();
                }
                QByteArray set;
                if (isupper(prototype.at(0))) {
                    set = "Set";
                } else {
                    set = "set";
                    prototype[0] = QtMiscUtils::toAsciiUpper(prototype[0]);
                }

                prototype = set + prototype;
            }
            Q_FALLTHROUGH(); // Fall through to support multi-variate properties
        case INVOKE_FUNC: // method
            {
                bool cloned = false;
                bool defargs;
                do {
                    QByteArray pnames;
                    for (qsizetype p = 0; p < parameters.size(); ++p) {
                        pnames += parameters.at(p);
                        if (p < parameters.size() - 1)
                            pnames += ',';
                    }
                    defargs = pnames.contains("=0");
                    int attributes = 0;
                    if (cloned)
                        attributes |= QMetaMethod::Cloned;
                    cloned |= defargs;
                    addSlot(type, prototype, pnames.replace("=0", ""), attributes);

                    if (defargs) {
                        parameters.takeLast();
                        qsizetype lastParam = prototype.lastIndexOf(',');
                        if (lastParam == -1)
                            lastParam = prototype.indexOf('(') + 1;
                        prototype.truncate(lastParam);
                        prototype += ')';
                    }
                } while (defargs);
            }
            break;

        default:
            break;
        }
#if 0 // documentation in metaobject would be cool?
        // get function documentation
        BSTR bstrDocu;
        info->GetDocumentation(funcdesc->memid, 0, &bstrDocu, 0, 0);
        QString strDocu = QString::fromWCharArray(bstrDocu);
        SysFreeString(bstrDocu);
        if (!!strDocu)
            desc += '[' + strDocu + ']';
        desc += '\n';
#endif
        typeinfo->ReleaseFuncDesc(funcdesc);
    }
}

void MetaObjectGenerator::readVarsInfo(ITypeInfo *typeinfo, ushort nVars)
{
    if (!nVars) {
        TYPEATTR *typeattr = nullptr;
        typeinfo->GetTypeAttr(&typeattr);
        if (typeattr) {
            nVars = typeattr->cVars;
            typeinfo->ReleaseTypeAttr(typeattr);
        }
    }

    // get information about all variables
    for (ushort vd = 0; vd < nVars; ++vd) {
        VARDESC *vardesc;
        typeinfo->GetVarDesc(vd, &vardesc);
        if (!vardesc)
            break;

        // no use if it's not a dispatched variable
        if (vardesc->varkind != VAR_DISPATCH) {
            typeinfo->ReleaseVarDesc(vardesc);
            continue;
        }

        // get variable name
        const QByteArray variableName = qaxTypeInfoName(typeinfo, vardesc->memid);
        if (variableName.isEmpty()) {
            typeinfo->ReleaseVarDesc(vardesc);
            continue;
        }

        uint flags = 0;

        // get variable type
        TYPEDESC typedesc = vardesc->elemdescVar.tdesc;
        const QByteArray variableType = guessTypes(typedesc, typeinfo, variableName);

        // generate meta property
        if (!hasProperty(variableName)) {
            flags = Readable;
            if (!(vardesc->wVarFlags & VARFLAG_FREADONLY))
                flags |= Writable;
            if (!(vardesc->wVarFlags & (VARFLAG_FNONBROWSABLE | VARFLAG_FHIDDEN)))
                flags |= Designable;
            if (!(vardesc->wVarFlags & VARFLAG_FRESTRICTED))
                flags |= Scriptable;
            if (vardesc->wVarFlags & VARFLAG_FREQUESTEDIT)
                flags |= RequestingEdit;
            if (hasEnum(variableType))
                flags |= EnumOrFlag;

            if (vardesc->wVarFlags & VARFLAG_FBINDABLE) {
                addChangedSignal(variableName, variableType, vardesc->memid);
                flags |= Bindable;
            }
            addProperty(variableType, variableName, flags);
        }

        // generate a set slot
        if (!(vardesc->wVarFlags & VARFLAG_FREADONLY))
            addSetterSlot(variableName);

#if 0 // documentation in metaobject would be cool?
        // get function documentation
        BSTR bstrDocu;
        info->GetDocumentation(vardesc->memid, 0, &bstrDocu, 0, 0);
        QString strDocu = QString::fromWCharArray(bstrDocu);
        SysFreeString(bstrDocu);
        if (!!strDocu)
            desc += '[' + strDocu + ']';
        desc += '\n';
#endif
        typeinfo->ReleaseVarDesc(vardesc);
    }
}

void MetaObjectGenerator::readInterfaceInfo()
{
    ComPtr<ITypeInfo> typeinfo = dispInfo;
    if (!typeinfo)
        return;
    int interface_serial = 0;
    while (typeinfo) {
        ushort nFuncs = 0;
        ushort nVars = 0;
        ushort nImpl = 0;
        // get information about type
        TYPEATTR *typeattr;
        typeinfo->GetTypeAttr(&typeattr);
        bool interesting = true;
        if (typeattr) {
            // get number of functions, variables, and implemented interfaces
            nFuncs = typeattr->cFuncs;
            nVars = typeattr->cVars;
            nImpl = typeattr->cImplTypes;

            if ((typeattr->typekind == TKIND_DISPATCH || typeattr->typekind == TKIND_INTERFACE) &&
                (typeattr->guid != IID_IDispatch && typeattr->guid != IID_IUnknown)) {
#ifndef QAX_NO_CLASSINFO
                if (d && d->useClassInfo) {
                    // UUID
                    QUuid uuid(typeattr->guid);
                    QString uuidstr = uuid.toString().toUpper();
                    uuidstr = iidnames.value(QLatin1String("/Interface/") + uuidstr + QLatin1String("/Default"), uuidstr).toString();
                    addClassInfo("Interface " + QByteArray::number(++interface_serial), uuidstr.toLatin1());
                }
#endif
                typeinfo->ReleaseTypeAttr(typeattr);
            } else {
                interesting = false;
                typeinfo->ReleaseTypeAttr(typeattr);
            }
        }

        if (interesting) {
            readFuncsInfo(typeinfo.Get(), nFuncs);
            readVarsInfo(typeinfo.Get(), nVars);
        }

        if (!nImpl) {
            typeinfo = nullptr;
            break;
        }

        // go up one base class
        HREFTYPE pRefType;
        typeinfo->GetRefTypeOfImplType(0, &pRefType);
        ComPtr<ITypeInfo> baseInfo;
        typeinfo->GetRefTypeInfo(pRefType, &baseInfo);
        if (typeinfo == baseInfo) { // IUnknown inherits IUnknown ???
            baseInfo.Reset();
            typeinfo.Reset();
            break;
        }
        typeinfo = baseInfo;
    }
}

void MetaObjectGenerator::readEventInterface(ITypeInfo *eventinfo, IConnectionPoint *cpoint)
{
    TYPEATTR *eventattr;
    eventinfo->GetTypeAttr(&eventattr);
    if (!eventattr)
        return;
    if (eventattr->typekind != TKIND_DISPATCH) {
        eventinfo->ReleaseTypeAttr(eventattr);
        return;
    }

    QAxEventSink *eventSink = nullptr;
    if (d) {
        IID conniid;
        cpoint->GetConnectionInterface(&conniid);
        eventSink = d->eventSink.value(QUuid(conniid));
        if (!eventSink) {
            eventSink = new QAxEventSink(that);
            d->eventSink.insert(QUuid(conniid), eventSink);
            eventSink->advise(cpoint, conniid);
        }
    }

    // get information about all event functions
    for (UINT fd = 0; fd < eventattr->cFuncs; ++fd) {
        FUNCDESC *funcdesc;
        eventinfo->GetFuncDesc(fd, &funcdesc);
        if (!funcdesc)
            break;
        if (funcdesc->invkind != INVOKE_FUNC ||
            funcdesc->funckind != FUNC_DISPATCH) {
            eventinfo->ReleaseFuncDesc(funcdesc);
            continue;
        }

        QByteArray prototype;
        QByteArrayList parameters;

        // parse event function description, get event function prototype
        const QByteArrayList names = qaxTypeInfoNames(eventinfo, funcdesc->memid);

        QByteArray type; // dummy - we don't care about return values for signals
        prototype = createPrototype(/*in*/ funcdesc, eventinfo, names, /*out*/type, parameters);
        if (!hasSignal(prototype)) {
            QByteArray pnames;
            for (qsizetype p = 0; p < parameters.size(); ++p) {
                pnames += parameters.at(p);
                if (p < parameters.size() - 1)
                    pnames += ',';
            }
            addSignal(prototype, pnames);
        }
        if (eventSink)
            eventSink->addSignal(funcdesc->memid, prototype);

#if 0 // documentation in metaobject would be cool?
        // get function documentation
        BSTR bstrDocu;
        eventinfo->GetDocumentation(funcdesc->memid, 0, &bstrDocu, 0, 0);
        QString strDocu = QString::fromWCharArray(bstrDocu);
        SysFreeString(bstrDocu);
        if (!!strDocu)
            desc += '[' + strDocu + ']';
        desc += '\n';
#endif
        eventinfo->ReleaseFuncDesc(funcdesc);
    }
    eventinfo->ReleaseTypeAttr(eventattr);
}

void MetaObjectGenerator::readEventInfo()
{
    int event_serial = 0;
    ComPtr<IConnectionPointContainer> cpoints;
    if (d && d->useEventSink)
        d->ptr->QueryInterface(IID_IConnectionPointContainer, &cpoints);
    if (cpoints) {
        // Get connection point enumerator
        ComPtr<IEnumConnectionPoints> epoints;
        cpoints->EnumConnectionPoints(&epoints);
        if (epoints) {
            ULONG c = 1;
            epoints->Reset();
            QList<QUuid> cpointlist;
            do {
                ComPtr<IConnectionPoint> cpoint;
                HRESULT hr = epoints->Next(c, &cpoint, &c);
                if (!c || hr != S_OK)
                    break;

                IID conniid;
                cpoint->GetConnectionInterface(&conniid);
                // workaround for typelibrary bug of Word.Application
                QUuid connuuid(conniid);
                if (cpointlist.contains(connuuid))
                    break;

#ifndef QAX_NO_CLASSINFO
                if (d->useClassInfo) {
                    QString uuidstr = connuuid.toString().toUpper();
                    uuidstr = iidnames.value(QLatin1String("/Interface/") + uuidstr + QLatin1String("/Default"), uuidstr).toString();
                    addClassInfo("Event Interface " + QByteArray::number(++event_serial), uuidstr.toLatin1());
                }
#endif

                // get information about type
                if (conniid == IID_IPropertyNotifySink) {
                    // test whether property notify sink has been created already, and advise on it
                    QAxEventSink *eventSink = d->eventSink.value(iid_propNotifySink);
                    if (eventSink)
                        eventSink->advise(cpoint.Get(), conniid);
                    continue;
                }

                ComPtr<ITypeInfo> eventinfo;
                if (typelib)
                    typelib->GetTypeInfoOfGuid(conniid, &eventinfo);

                if (eventinfo) {
                    // avoid recursion (see workaround above)
                    cpointlist.append(connuuid);

                    readEventInterface(eventinfo.Get(), cpoint.Get());
                }
            } while (c);
        } else if (classInfo) { // no enumeration - search source interfaces and ask for those
            TYPEATTR *typeattr = nullptr;
            classInfo->GetTypeAttr(&typeattr);
            if (typeattr) {
                for (UINT i = 0; i < typeattr->cImplTypes; ++i) {
                    int flags = 0;
                    classInfo->GetImplTypeFlags(i, &flags);
                    if (!(flags & IMPLTYPEFLAG_FSOURCE))
                        continue;
                    HREFTYPE reference;
                    if (S_OK != classInfo->GetRefTypeOfImplType(i, &reference))
                        continue;
                    ComPtr<ITypeInfo> eventInfo;
                    classInfo->GetRefTypeInfo(reference, &eventInfo);
                    if (!eventInfo)
                        continue;
                    TYPEATTR *eventattr = nullptr;
                    eventInfo->GetTypeAttr(&eventattr);
                    if (eventattr) {
                        ComPtr<IConnectionPoint> cpoint;
                        cpoints->FindConnectionPoint(eventattr->guid, &cpoint);
                        if (cpoint) {
                            if (eventattr->guid == IID_IPropertyNotifySink) {
                                // test whether property notify sink has been created already, and advise on it
                                QAxEventSink *eventSink = d->eventSink.value(iid_propNotifySink);
                                if (eventSink)
                                    eventSink->advise(cpoint.Get(), eventattr->guid);
                                continue;
                            }

                            readEventInterface(eventInfo.Get(), cpoint.Get());
                        }
                        eventInfo->ReleaseTypeAttr(eventattr);
                    }
                }
                classInfo->ReleaseTypeAttr(typeattr);
            }
        }
    }
}

QMetaObject *MetaObjectGenerator::tryCache()
{
    if (!cacheKey.isEmpty()) {
        d->metaobj = mo_cache.value(cacheKey);
        if (d->metaobj) {
            d->cachedMetaObject = true;
            const QMetaObjectExtra &moExtra = moextra_cache.value(d->metaobj);

            ComPtr<IConnectionPointContainer> cpoints;
            d->ptr->QueryInterface(IID_IConnectionPointContainer, &cpoints);
            if (cpoints) {
                for (const QUuid &iid : moExtra.connectionInterfaces()) {
                    ComPtr<IConnectionPoint> cpoint;
                    cpoints->FindConnectionPoint(iid, &cpoint);
                    if (cpoint) {
                        QAxEventSink *sink = new QAxEventSink(that);
                        sink->advise(cpoint.Get(), iid);
                        d->eventSink.insert(iid, sink);
                        sink->sigs = moExtra.sigs().value(iid);
                        sink->props = moExtra.props().value(iid);
                        sink->propsigs = moExtra.propsigs().value(iid);
                    }
                }
            }

            return d->metaobj;
        }
    }
    return nullptr;
}

static void addMetaProperty(QMetaObjectBuilder &builder, const QByteArray &name,
                            const QByteArray &type, uint flags)
{
    QMetaPropertyBuilder propertyBuilder = builder.addProperty(name, type);
    propertyBuilder.setReadable(flags & Readable);
    propertyBuilder.setWritable(flags & Writable);
    propertyBuilder.setResettable(flags & Resettable);
    propertyBuilder.setEnumOrFlag(flags & EnumOrFlag);
    propertyBuilder.setStdCppSet(flags & StdCppSet);
    propertyBuilder.setConstant(flags & Constant);
    propertyBuilder.setFinal(flags & Final);
    propertyBuilder.setDesignable(flags & Designable);
    propertyBuilder.setScriptable(flags & Scriptable);
    propertyBuilder.setStored(flags & Stored);
    propertyBuilder.setUser(flags & User);
}

// Returns the sum of all parameters (including return type) for the given
// \a map of methods. This is needed for calculating the size of the methods'
// parameter type/name meta-data.
int MetaObjectGenerator::aggregateParameterCount(const QMap<QByteArray, Method> &map)
{
    int sum = 0;
    QMap<QByteArray, Method>::const_iterator it;
    for (it = map.constBegin(); it != map.constEnd(); ++it)
        sum += paramList(it.key()).size() + 1; // +1 for return type
    return sum;
}

void MetaObjectGenerator::addMetaMethod(QMetaObjectBuilder &builder,
                                        BuilderMethodCreationFunc creationFunc,
                                        const QByteArray &name,
                                        const QByteArray &parameters,
                                        const QByteArray &returnType,
                                        int attributes)
{
    const QList<QByteArray> parameterList = parameters.split(',');

    // name is of format "functioneName(parameterType1,parameterType2)". If a parameter type is
    // unknown (due to a missing dependency for example) the list of these parameter types will
    // contain an empty sting. This situation will throw off QMetaMethodBuilder so we skip these
    // functions.
    if (!checkForValidFunctionParameters(name, parameterList))
        return;

    QMetaMethodBuilder methodBuilder = (builder.*creationFunc)(name);
    if (!parameters.isEmpty())
        methodBuilder.setParameterNames(parameterList);
    if (!returnType.isEmpty() && returnType != QByteArrayLiteral("void"))
        methodBuilder.setReturnType(returnType);
    methodBuilder.setAttributes(attributes);
}

void MetaObjectGenerator::buildMethods(const QMap<QByteArray, Method> &map,
                                       QMetaObjectExtra &moExtra,
                                       QMetaObjectBuilder &builder,
                                       BuilderMethodCreationFunc creationFunc)
{
    for (auto it = map.constBegin(), end = map.constEnd(); it != end; ++it) {
        QByteArray prototype(QMetaObject::normalizedSignature(it.key()));
        const Method &method(it.value());
        if (!method.realPrototype.isEmpty())
            moExtra.realPrototype.insert(prototype, method.realPrototype);
        addMetaMethod(builder, creationFunc, prototype, method.parameters,
                      method.type, method.attributes);
    }
}

QMetaObject *MetaObjectGenerator::metaObject(const QMetaObject *parentObject, const QByteArray &className)
{
    QSignalBlocker blockSignals(that ? that->qObject() : nullptr);
    if (that) {
        readClassInfo();
        if (typelib) {
            QBStr bstr;
            typelib->GetDocumentation(-1, &bstr, nullptr, nullptr, nullptr);
            current_typelib = bstr.str().toLatin1();
        }
        if (d->tryCache && tryCache())
            return d->metaobj;
        readEnumInfo();
        readInterfaceInfo();
        readEventInfo();
    }

    current_typelib = QByteArray();

#ifndef QAX_NO_CLASSINFO
    if (!debugInfo.isEmpty() && d->useClassInfo)
        addClassInfo("debugInfo", debugInfo);
#endif

    QMetaObjectExtra moExtra;

    if (parentObject == nullptr)
        parentObject = &QObject::staticMetaObject;
    QMetaObjectBuilder builder(parentObject, {} /* Don't add anything from prototype */);
    builder.setSuperClass(parentObject);
    builder.setClassName(that ? QByteArray(that->className()) : className);

    // each class info
    for (auto it = classinfo_list.cbegin(), cend = classinfo_list.cend(); it != cend; ++it)
        builder.addClassInfo(it.key(), it.value());

    buildMethods(signal_list, moExtra, builder, &QMetaObjectBuilder::addSignal);
    buildMethods(slot_list, moExtra, builder, &QMetaObjectBuilder::addSlot);

    for (auto it = property_list.cbegin(), end = property_list.cend(); it != end; ++it) {
        const QByteArray &name = it.key();
        const QByteArray &type = it.value().type;
        Q_ASSERT(!type.isEmpty());
        QByteArray realType(it.value().realType);
        if (!realType.isEmpty() && realType != type)
            moExtra.realPrototype.insert(name, realType);
        if (it.value().flags & EnumOrFlag)
            addMetaProperty(builder, name, "int", it.value().flags);
        else
            addMetaProperty(builder, name, type, it.value().flags);
    }

    // each enum in form name\0
    for (auto it = enum_list.cbegin(), end = enum_list.cend(); it != end; ++it) {
        QMetaEnumBuilder enumBuilder = builder.addEnumerator(it.key());
        for (const auto &v : it.value())
            enumBuilder.addKey(v.first, v.second);
    }

    auto metaobj = builder.toMetaObject();
    if (d)
        d->metaobj = metaobj;

    if (!cacheKey.isEmpty()) {
        mo_cache.insert(cacheKey, d->metaobj);
        d->cachedMetaObject = true;
        for (auto it = d->eventSink.cbegin(), end = d->eventSink.cend(); it != end; ++it) {
            if (QAxEventSink *sink = it.value()) {
                QUuid ciid = sink->connectionInterface();

                moExtra.m_connectionInterfaces.append(ciid);
                moExtra.m_sigs.insert(ciid, sink->signalMap());
                moExtra.m_props.insert(ciid, sink->propertyMap());
                moExtra.m_propsigs.insert(ciid, sink->propSignalMap());
            }
        }
        moextra_cache.insert(d->metaobj, moExtra);
    }

    return metaobj;
}

/*!
    \fn const QMetaObject *QAxBase::fallbackMetaObject() const
    \internal
*/

/*!
    \internal

    The metaobject is generated on the fly from the information
    provided by the IDispatch and ITypeInfo interface implementations
    in the COM object.
*/
const QMetaObject *QAxBase::axBaseMetaObject() const
{
    if (d->metaobj)
        return d->metaobj;
    const QMetaObject* parentObject = d->parentMetaObject();

    if (!d->ptr && !d->initialized) {
        const_cast<QAxBase*>(this)->initialize(&d->ptr);
        d->initialized = true;
    }

#ifndef QT_NO_THREAD
    // only one thread at a time can generate meta objects
    QMutexLocker locker(&cache_mutex);
#endif

    // return the default meta object if not yet initialized
    if (!d->ptr || !d->useMetaObject)
        return d->fallbackMetaObject();

    MetaObjectGenerator generator(const_cast<QAxBase *>(this), d);
    return generator.metaObject(parentObject);
}

/*!
    \internal

    Connects to all event interfaces of the object.

    Called by the subclasses' connectNotify() reimplementations, so
    at this point the connection as actually been created already.
*/
void QAxBase::connectNotify()
{
    if (!d->eventSink.isEmpty()) // already listening
        return;

    ComPtr<IEnumConnectionPoints> epoints;
    if (d->ptr && d->useEventSink) {
        ComPtr<IConnectionPointContainer> cpoints;
        d->ptr->QueryInterface(IID_IConnectionPointContainer, &cpoints);
        if (!cpoints)
            return;

        cpoints->EnumConnectionPoints(&epoints);
    }

    if (!epoints)
        return;

    UINT index;
    IDispatch *disp = d->dispatch();
    ITypeInfo *typeinfo = nullptr;
    ComPtr<ITypeLib> typelib;
    if (disp)
        disp->GetTypeInfo(0, LOCALE_USER_DEFAULT, &typeinfo);
    if (typeinfo)
        typeinfo->GetContainingTypeLib(&typelib, &index);

    if (!typelib) {
        return;
    }

    MetaObjectGenerator generator(this, d);
    bool haveEnumInfo = false;

    ULONG c = 1;
    epoints->Reset();
    do {
        ComPtr<IConnectionPoint> cpoint;
        epoints->Next(c, &cpoint, &c);
        if (!c || !cpoint)
            break;

        IID conniid;
        cpoint->GetConnectionInterface(&conniid);
        // workaround for typelibrary bug of Word.Application
        const QUuid connuuid(conniid);
        if (d->eventSink.contains(connuuid))
            break;

        // Get ITypeInfo for source-interface, and skip if not supporting IDispatch
        ComPtr<ITypeInfo> eventinfo;
        typelib->GetTypeInfoOfGuid(conniid, &eventinfo);
        if (eventinfo) {
            TYPEATTR *eventAttr;
            eventinfo->GetTypeAttr(&eventAttr);
            if (!eventAttr) {
                break;
            }

            TYPEKIND eventKind = eventAttr->typekind;
            eventinfo->ReleaseTypeAttr(eventAttr);
            if (eventKind != TKIND_DISPATCH) {
                break;
            }
        }

        // always into the cache to avoid recoursion
        QAxEventSink *eventSink = eventinfo ? new QAxEventSink(this) : nullptr;
        d->eventSink.insert(connuuid, eventSink);

        if (!eventinfo)
            continue;

        // have to get type info to support signals with enum parameters
        if (!haveEnumInfo) {
            bool wasTryCache = d->tryCache;
            d->tryCache = true;
            generator.readClassInfo();
            generator.readEnumInfo();
            d->tryCache = wasTryCache;
            haveEnumInfo = true;
        }
        generator.readEventInterface(eventinfo.Get(), cpoint.Get());
        eventSink->advise(cpoint.Get(), conniid);
    } while (c);

    // make sure we don't try again
    if (d->eventSink.isEmpty())
        d->eventSink.insert(QUuid{}, nullptr);
}

/*!
    \fn QString QAxBase::generateDocumentation()

    Returns a rich text string with documentation for the
    wrapped COM object. Dump the string to an HTML-file,
    or use it in e.g. a QTextBrowser widget.
*/

void QAxBasePrivate::handleException(tagEXCEPINFO *exc, const QString &name)
{
    if (exc->pfnDeferredFillIn)
        exc->pfnDeferredFillIn(exc);
    int code = exc->wCode ? exc->wCode : exc->scode;
    QString source = QString::fromWCharArray(exc->bstrSource);
    QString desc = QString::fromWCharArray(exc->bstrDescription);
    QString help = QString::fromWCharArray(exc->bstrHelpFile);
    const uint helpContext = exc->dwHelpContext;
    if (helpContext && !help.isEmpty())
        help += QString::fromLatin1(" [%1]").arg(helpContext);

    emitException(code, source, desc, help);
    if (!QAxEventSink::signalHasReceivers(q->qObject(), "exception(int,QString,QString,QString)")) {
        qWarning(R"(QAxBase: Error calling IDispatch member %s: Exception thrown by server
             Code       : %d
             Source     : %s
             Description: %s
             Help       : %s
         Connect to the exception(int,QString,QString,QString) signal to catch this exception)",
                 qPrintable(name), code, qPrintable(source), qPrintable(desc),
                 qPrintable(help));
    }
}

bool QAxBasePrivate::checkHRESULT(HRESULT hres, EXCEPINFO *exc, const QString &name, uint argerr)
{
    switch(hres) {
    case S_OK:
        return true;
    case DISP_E_BADPARAMCOUNT:
        qWarning("QAxBase: Error calling IDispatch member %s: Bad parameter count",
                 qPrintable(name));
        return false;
    case DISP_E_BADVARTYPE:
        qWarning("QAxBase: Error calling IDispatch member %s: Bad variant type",
                 qPrintable(name));
        return false;
    case DISP_E_EXCEPTION:
        handleException(exc, name);
        return false;
    case DISP_E_MEMBERNOTFOUND:
        qWarning("QAxBase: Error calling IDispatch member %s: Member not found",
                 qPrintable(name));
        return false;
    case DISP_E_NONAMEDARGS:
        qWarning("QAxBase: Error calling IDispatch member %s: No named arguments",
                 qPrintable(name));
        return false;
    case DISP_E_OVERFLOW:
        qWarning("QAxBase: Error calling IDispatch member %s: Overflow",
                 qPrintable(name));
        return false;
    case DISP_E_PARAMNOTFOUND:
        qWarning("QAxBase: Error calling IDispatch member %s: Parameter %d not found",
                 qPrintable(name), argerr);
        return false;
    case DISP_E_TYPEMISMATCH:
        qWarning("QAxBase: Error calling IDispatch member %s: Type mismatch in parameter %d",
                 qPrintable(name), argerr);
        return false;
    case DISP_E_UNKNOWNINTERFACE:
        qWarning("QAxBase: Error calling IDispatch member %s: Unknown interface",
                 qPrintable(name));
        return false;
    case DISP_E_UNKNOWNLCID:
        qWarning("QAxBase: Error calling IDispatch member %s: Unknown locale ID",
                 qPrintable(name));
        return false;
    case DISP_E_PARAMNOTOPTIONAL:
        qWarning("QAxBase: Error calling IDispatch member %s: Non-optional parameter missing",
                 qPrintable(name));
        return false;
    default:
        qWarning("QAxBase: Error calling IDispatch member %s: Unknown error",
                 qPrintable(name));
        return false;
    }
}

/*!
    \internal
*/
int QAxBase::internalProperty(QMetaObject::Call call, int index, void **v)
{
    const QMetaObject *mo = qObject()->metaObject();
    const QMetaProperty prop = mo->property(index + mo->propertyOffset());
    QByteArray propname = prop.name();

    // get the IDispatch
    if (!d->ptr || !prop.isValid())
        return index;
    IDispatch *disp = d->dispatch();
    if (!disp)
        return index;

    const QMetaObjectExtra &moExtra = moextra_cache.value(d->metaObject());
    DISPID dispid = moExtra.dispIDofName(propname, disp);
    if (dispid == DISPID_UNKNOWN)
        return index;

    // property found, so everthing that goes wrong now should not bother the caller
    index -= mo->propertyCount();

    VARIANTARG arg;
    VariantInit(&arg);
    DISPPARAMS params;
    EXCEPINFO excepinfo;
    memset(&excepinfo, 0, sizeof(excepinfo));
    UINT argerr = 0;
    HRESULT hres = E_FAIL;

    QByteArray proptype(prop.typeName());
    switch (call) {
    case QMetaObject::ReadProperty:
        {
            params.cArgs = 0;
            params.cNamedArgs = 0;
            params.rgdispidNamedArgs = nullptr;
            params.rgvarg = nullptr;

            hres = Invoke(disp, dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &arg, &excepinfo, nullptr);

            // map result VARIANTARG to void*
            int type = QMetaType::Int;
            if (!prop.isEnumType())
                type = prop.metaType().id();
            QVariantToVoidStar(VARIANTToQVariant(arg, proptype, type), *v, proptype, type);
            if ((arg.vt != VT_DISPATCH && arg.vt != VT_UNKNOWN)
                || type == QMetaType::QPixmap || type == QMetaType::QFont)
                clearVARIANT(&arg);
        }
        break;

    case QMetaObject::WriteProperty:
        {
            DISPID dispidNamed = DISPID_PROPERTYPUT;
            params.cArgs = 1;
            params.cNamedArgs = 1;
            params.rgdispidNamedArgs = &dispidNamed;
            params.rgvarg = &arg;

            arg.vt = VT_ERROR;
            arg.scode = DISP_E_TYPEMISMATCH;

            // map void* to VARIANTARG via QVariant
            QVariant qvar;
            if (prop.isEnumType()) {
                qvar = *reinterpret_cast<const int *>(v[0]);
                proptype = nullptr;
            } else {
                int typeId = prop.userType();
                if (typeId == int(QMetaType::QVariant)) {
                    qvar = *reinterpret_cast<const QVariant *>(v[0]);
                    proptype = nullptr;
                } else {
                    qvar = QVariant(prop.metaType(), v[0]);
                    if (typeId < QMetaType::User)
                        proptype = moExtra.propertyType(propname);
                }
            }

            QVariantToVARIANT(qvar, arg, proptype);
            if (arg.vt == VT_EMPTY || arg.vt == VT_ERROR) {
                qWarning("QAxBase::setProperty: Unhandled property type %s", prop.typeName());
                break;
            }
        }
        hres = Invoke(disp, dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &params, nullptr, &excepinfo, &argerr);
        clearVARIANT(&arg);
        break;

    default:
        break;
    }

    d->checkHRESULT(hres, &excepinfo, QLatin1String(propname), argerr);
    return index;
}

int QAxBase::internalInvoke(QMetaObject::Call call, int index, void **v)
{
    Q_ASSERT(call == QMetaObject::InvokeMetaMethod);
    Q_UNUSED(call);

    // get the IDispatch
    IDispatch *disp = d->dispatch();
    if (!disp)
        return index;

    const QMetaObject *mo = qObject()->metaObject();
    // get the slot information
    const QMetaMethod slot = mo->method(index + mo->methodOffset());
    Q_ASSERT(slot.methodType() == QMetaMethod::Slot);

    QByteArray signature(slot.methodSignature());
    QByteArray slotname(signature);
    slotname.truncate(slotname.indexOf('('));

    // Get the Dispatch ID of the method to be called
    bool isProperty = false;
    const QMetaObjectExtra &moExtra = moextra_cache.value(d->metaObject());
    DISPID dispid = moExtra.dispIDofName(slotname, disp);

    if (dispid == DISPID_UNKNOWN && slotname.toLower().startsWith("set")) {
        // see if we are calling a property set function as a slot
        slotname.remove(0, 3);
        dispid = moExtra.dispIDofName(slotname, disp);
        isProperty = true;
    }
    if (dispid == DISPID_UNKNOWN)
        return index;

    // slot found, so everthing that goes wrong now should not bother the caller
    index -= mo->methodCount();

    // setup the parameters
    DISPPARAMS params;
    DISPID dispidNamed = DISPID_PROPERTYPUT;
    params.cArgs = UINT(moExtra.numParameter(signature));
    params.cNamedArgs = isProperty ? 1 : 0;
    params.rgdispidNamedArgs = isProperty ? &dispidNamed : nullptr;
    params.rgvarg = nullptr;
    VARIANTARG static_rgvarg[QAX_NUM_PARAMS];
    if (params.cArgs) {
        if (params.cArgs <= QAX_NUM_PARAMS)
            params.rgvarg = static_rgvarg;
        else
            params.rgvarg = new VARIANTARG[params.cArgs];
    }
    for (VARIANTARG *vp = params.rgvarg, *vEnd = params.rgvarg + params.cArgs; vp < vEnd; ++vp)
        VariantInit(vp);

    int p;
    for (p = 0; p < int(params.cArgs); ++p) {
        bool out;
        const QByteArray type = moExtra.paramType(signature, p, &out);
        const QMetaType metaType = QMetaType::fromName(type);
        QVariant qvar;
        if (metaType.id() != QMetaType::User && metaType.id() != QMetaType::QVariant)
            qvar = QVariant(metaType, v[p + 1]);

        if (!qvar.isValid()) {
            if (type == "IDispatch*") {
                if (out)
                    qvar.setValue(*reinterpret_cast<IDispatch ***>(v[p+1]));
                else
                    qvar.setValue(*reinterpret_cast<IDispatch **>(v[p+1]));
            } else if (type == "IUnknown*") {
                    qvar.setValue(*reinterpret_cast<IUnknown **>(v[p+1]));
            } else if (type == "QVariant") {
                qvar = *reinterpret_cast<const QVariant *>(v[p + 1]);
            } else if (mo->indexOfEnumerator(type) != -1) {
                qvar = *reinterpret_cast<const int *>(v[p + 1]);
            } else {
                qvar = QVariant(metaType, v[p + 1]);
            }
        }

        QVariantToVARIANT(qvar, params.rgvarg[int(params.cArgs) - p - 1], type, out);
    }

    // call the method
    VARIANT ret;
    VariantInit(&ret);
    UINT argerr = 0;
    HRESULT hres = E_FAIL;
    EXCEPINFO excepinfo;
    memset(&excepinfo, 0, sizeof(excepinfo));

    WORD wFlags = isProperty ? DISPATCH_PROPERTYPUT : DISPATCH_METHOD | DISPATCH_PROPERTYGET;
    hres = Invoke(disp, dispid, IID_NULL, LOCALE_USER_DEFAULT, wFlags, &params, &ret, &excepinfo, &argerr);

    // get return value
    if (hres == S_OK && ret.vt != VT_EMPTY) {
        QVariantToVoidStar(VARIANTToQVariant(ret, slot.typeName(), slot.returnType()), v[0], slot.typeName());
        if (ret.vt != VT_DISPATCH)
            clearVARIANT(&ret);
        else
            VariantInit(&ret);
    }

    // update out parameters
    for (p = 0; p < int(params.cArgs); ++p) {
        bool out;
        QByteArray ptype = moExtra.paramType(signature, p, &out);
        if (out) {
            VARIANTARG &var = params.rgvarg[int(params.cArgs) - p - 1];
            QVariantToVoidStar(VARIANTToQVariant(var, ptype), v[p+1], ptype);
            if (var.vt == (VT_DISPATCH | VT_BYREF))
                VariantInit(&var); // Prevent clearVARIANT() from releasing returned IDispatch* out parameters.
        }
    }
    // clean up
    for (p = 0; p < int(params.cArgs); ++p)
        clearVARIANT(params.rgvarg+p);
    if (params.rgvarg != static_rgvarg)
        delete [] params.rgvarg;

    d->checkHRESULT(hres, &excepinfo, QString::fromLatin1(slotname), params.cArgs-argerr-1);
    return index;
}

int QAxBasePrivate::qtStaticMetaCall(QAxBase *_t, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c != QMetaObject::InvokeMetaMethod)
        return 0;
    Q_ASSERT(_t != nullptr);
    const QMetaObject *mo = _t->qObject()->metaObject();
    switch (mo->method(_id + mo->methodOffset()).methodType()) {
    case QMetaMethod::Signal:
        QMetaObject::activate(_t->qObject(), mo, _id, _a);
        return _id - mo->methodCount();
    case QMetaMethod::Method:
    case QMetaMethod::Slot:
        return _t->internalInvoke(_c, _id, _a);
    default:
        break;
    }
    return 0;
}

int QAxBasePrivate::qtMetaCall(QMetaObject::Call call, int id, void **v)
{
    const QMetaObject *mo = q->qObject()->metaObject();
    if (q->isNull() && mo->property(id + mo->propertyOffset()).name() != QByteArray("control")) {
        qWarning("QAxBase::qt_metacall: Object is not initialized, or initialization failed");
        return id;
    }

    switch(call) {
    case QMetaObject::InvokeMetaMethod:
        id = qtStaticMetaCall(q, call, id, v);
        break;
    case QMetaObject::ReadProperty:
    case QMetaObject::WriteProperty:
    case QMetaObject::ResetProperty:
        id = q->internalProperty(call, id, v);
        break;
    default:
        break;
    }
    Q_ASSERT(id < 0);
    return id;
}

#ifdef QT_CHECK_STATE
static void qax_noSuchFunction(int disptype, const QByteArray &name, const QByteArray &function, const QAxBase *that)
{
    const QMetaObject *metaObject = that->qObject()->metaObject();
    const char *coclass = metaObject->classInfo(metaObject->indexOfClassInfo("CoClass")).value();

    if (disptype == DISPATCH_METHOD) {
        qWarning("QAxBase::dynamicCallHelper: %s: No such method in %s [%s]", name.data(), that->control().toLatin1().data(), coclass ? coclass: "unknown");
        qWarning("\tCandidates are:");
        for (int i = 0; i < metaObject->methodCount(); ++i) {
            const QMetaMethod slot(metaObject->method(i));
            if (slot.methodType() != QMetaMethod::Slot)
                continue;
            QByteArray signature = slot.methodSignature();
            if (signature.toLower().startsWith(function.toLower()))
                qWarning("\t\t%s", signature.data());
        }
    } else {
        qWarning("QAxBase::dynamicCallHelper: %s: No such property in %s [%s]", name.data(), that->control().toLatin1().data(), coclass ? coclass: "unknown");
        if (!function.isEmpty()) {
            qWarning("\tCandidates are:");
            char f0 = function.toLower().at(0);
            for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i) {
                QByteArray signature(metaObject->property(i).name());
                if (!signature.isEmpty() && signature.toLower().at(0) == f0)
                    qWarning("\t\t%s", signature.data());
            }
        }
    }
}
#endif

/*!
    \internal

    \a name is already normalized?
*/
bool QAxBase::dynamicCallHelper(const char *name, void *inout, QList<QVariant> &vars,
                                QByteArray &type, unsigned flags)
{
    if (isNull()) {
        qWarning("QAxBase::dynamicCallHelper: Object is not initialized, or initialization failed");
        return false;
    }

    IDispatch *disp = d->dispatch();
    if (!disp) {
        qWarning("QAxBase::dynamicCallHelper: Object does not support automation");
        return false;
    }

    const QMetaObject *mo = qObject()->metaObject();
    Q_ASSERT(d->metaobj);
    const QMetaObjectExtra &moExtra = moextra_cache.value(d->metaobj);

    qsizetype varc = vars.size();

    QByteArray normFunction = QMetaObject::normalizedSignature(name);
    QByteArray function(normFunction);
    VARIANT staticarg[QAX_NUM_PARAMS];
    VARIANT *arg = nullptr;
    VARIANTARG *res = reinterpret_cast<VARIANTARG *>(inout);

    unsigned short disptype;

    int id = -1;
    bool parse = false;

    if (function.contains('(')) {
        disptype = DISPATCH_METHOD;
        if (!(flags & NoPropertyGet))
            disptype |= DISPATCH_PROPERTYGET; // Support Excel/VB.
        if (d->useMetaObject)
            id = mo->indexOfSlot(function);
        if (id >= 0) {
            const QMetaMethod slot = mo->method(id);
            Q_ASSERT(slot.methodType() == QMetaMethod::Slot);
            function = slot.methodSignature();
            type = slot.typeName();
        }
        function.truncate(function.indexOf('('));
        parse = !varc && normFunction.length() > function.length() + 2;
        if (parse) {
            QString args = QLatin1String(normFunction);
            args.remove(0, function.length() + 1);
            // parse argument string int list of arguments
            QString curArg;
            const QChar *c = args.unicode();
            int index = 0;
            bool inString = false;
            bool inEscape = false;
            while (index < args.length()) {
                QChar cc = *c;
                ++c;
                ++index;
                switch(cc.toLatin1()) {
                case 'n':
                    if (inEscape)
                        cc = QLatin1Char('\n');
                    break;
                case 'r':
                    if (inEscape)
                        cc = QLatin1Char('\r');
                    break;
                case 't':
                    if (inEscape)
                        cc = QLatin1Char('\t');
                    break;
                case '\\':
                    if (!inEscape && inString) {
                        inEscape = true;
                        continue;
                    }
                    break;
                case '"':
                    if (!inEscape) {
                        inString = !inString;
                        curArg += cc;
                        continue;
                    }
                    break;
                case ' ':
                    if (!inString && curArg.isEmpty())
                        continue;
                    break;
                case ',':
                case ')':
                    if (inString)
                        break;
                    curArg = curArg.trimmed();
                    if (curArg.at(0) == QLatin1Char('\"') && curArg.at(curArg.length()-1) == QLatin1Char('\"')) {
                        vars << curArg.mid(1, curArg.length() - 2);
                    } else {
                        bool isNumber = false;
                        bool isDouble = false;
                        int number = curArg.toInt(&isNumber);
                        double dbl = curArg.toDouble(&isDouble);
                        if (isNumber) {
                            vars << number;
                        } else if (isDouble) {
                            vars << dbl;
                        } else {
                            bool isEnum = false;
                            for (int enumIndex = 0; enumIndex < mo->enumeratorCount(); ++enumIndex) {
                                QMetaEnum metaEnum =mo->enumerator(enumIndex);
                                int value = metaEnum.keyToValue(curArg.toLatin1());
                                if (value != -1 && !QByteArray(metaEnum.valueToKey(value)).isEmpty()) {
                                    vars << value;
                                    isEnum = true;
                                    break;
                                }
                            }
                            if (!isEnum)
                                vars << curArg;
                        }
                    }
                    curArg.clear();
                    continue;
                default:
                    break;
                }
                inEscape = false;
                curArg += cc;
            }

            varc = vars.size();
        }
    } else {
        if (d->useMetaObject)
            id = mo->indexOfProperty(normFunction);

        if (id >= 0) {
            const QMetaProperty prop =mo->property(id);
            type = prop.typeName();
        }
        if (varc == 1) {
            res = nullptr;
            disptype = DISPATCH_PROPERTYPUT;
        } else {
            disptype = DISPATCH_PROPERTYGET;
        }
    }
    QBitArray outArgs;
    if (varc) {
        varc = qMin(varc, moExtra.numParameter(normFunction));
        arg = varc <= QAX_NUM_PARAMS ? staticarg : new VARIANT[varc];
        outArgs = QBitArray(varc);
        for (qsizetype i = 0; i < varc; ++i) {
            const QVariant &var = vars.at(i);
            VariantInit(arg + (varc - i - 1));
            bool out = false;
            QByteArray paramType;
            if (disptype == DISPATCH_PROPERTYPUT)
                paramType = type;
            else if (parse || disptype == DISPATCH_PROPERTYGET)
                paramType = nullptr;
            else
                paramType = moExtra.paramType(normFunction, i, &out);

            const int vType = var.metaType().id();
            if ((!parse && d->useMetaObject && vType == QMetaType::QString)
                || vType == QMetaType::QByteArray) {
                int enumIndex =mo->indexOfEnumerator(paramType);
                if (enumIndex != -1) {
                    QMetaEnum metaEnum =mo->enumerator(enumIndex);
                    QVariantToVARIANT(metaEnum.keyToValue(var.toByteArray()), arg[varc - i - 1], "int", out);
                }
            }

            if (arg[varc - i - 1].vt == VT_EMPTY)
                QVariantToVARIANT(var, arg[varc - i - 1], paramType, out);
            outArgs[i] = out;
        }
    }

    DISPID dispid = moExtra.dispIDofName(function, disp);
    if (dispid == DISPID_UNKNOWN && function.toLower().startsWith("set")) {
        function = function.mid(3);
        dispid = moExtra.dispIDofName(function, disp);
        disptype = DISPATCH_PROPERTYPUT;
    }

    if (dispid == DISPID_UNKNOWN) {
#ifdef QT_CHECK_STATE
        qax_noSuchFunction(disptype, normFunction, function, this);
#endif
        return false;
    }

    DISPPARAMS params;
    DISPID dispidNamed = DISPID_PROPERTYPUT;

    params.cArgs = UINT(varc);
    params.cNamedArgs = (disptype == DISPATCH_PROPERTYPUT) ? 1 : 0;
    params.rgdispidNamedArgs = (disptype == DISPATCH_PROPERTYPUT) ? &dispidNamed : nullptr;
    params.rgvarg = arg;
    EXCEPINFO excepinfo;
    memset(&excepinfo, 0, sizeof(excepinfo));
    UINT argerr = 0;

    HRESULT hres = Invoke(disp, dispid, IID_NULL, LOCALE_USER_DEFAULT, disptype, &params, res, &excepinfo, &argerr);
    if (hres == DISP_E_MEMBERNOTFOUND && (disptype & DISPATCH_METHOD) != 0) {
        // Retry with pVarResult = 0, workaround for "Word.Application.WordBasic.DisableAutoMacros(bool)"
        memset(&excepinfo, 0, sizeof(excepinfo));
        argerr = 0;
        hres = Invoke(disp, dispid, IID_NULL, LOCALE_USER_DEFAULT, disptype, &params, nullptr, &excepinfo, &argerr);
    }

    if ((disptype & (DISPATCH_METHOD|DISPATCH_PROPERTYGET)) && hres == S_OK && varc) {
        for (qsizetype i = 0; i < varc; ++i)
            if ((arg[varc-i-1].vt & VT_BYREF) || outArgs[i]) // update out-parameters
                vars[i] = VARIANTToQVariant(arg[varc-i-1], vars.at(i).typeName());
    }

    // clean up
    for (qsizetype i = 0; i < varc; ++i)
        clearVARIANT(params.rgvarg+i);
    if (arg && arg != staticarg)
        delete[] arg;

    return d->checkHRESULT(hres, &excepinfo, QLatin1String(function), uint(varc) - argerr - 1);
}

/*!
    \internal
*/
QVariantList QAxBase::argumentsToList(const QVariant &var1, const QVariant &var2,
                                      const QVariant &var3, const QVariant &var4,
                                      const QVariant &var5, const QVariant &var6,
                                      const QVariant &var7, const QVariant &var8)
{
    QVariantList vars;
    QVariant var = var1;
    int argc = 1;
    while (var.isValid()) {
        vars << var;
        switch (++argc) {
        case 2: var = var2; break;
        case 3: var = var3; break;
        case 4: var = var4; break;
        case 5: var = var5; break;
        case 6: var = var6; break;
        case 7: var = var7; break;
        case 8: var = var8; break;
        default:var = QVariant(); break;
        }
    }
    return vars;
}

/*!
    Calls the COM object's method \a function, passing the
    parameters \a var1, \a var1, \a var2, \a var3, \a var4, \a var5,
    \a var6, \a var7 and \a var8, and returns the value returned by
    the method, or an invalid QVariant if the method does not return
    a value or when the function call failed.

    If \a function is a method of the object the string must be provided
    as the full prototype, for example as it would be written in a
    QObject::connect() call.

    \snippet src_activeqt_container_qaxbase.cpp 15

    Alternatively a function can be called passing the parameters embedded
    in the string, e.g. above function can also be invoked using

    \snippet src_activeqt_container_qaxbase.cpp 16

    All parameters are passed as strings; it depends on the control whether
    they are interpreted correctly, and is slower than using the prototype
    with correctly typed parameters.

    If \a function is a property the string has to be the name of the
    property. The property setter is called when \a var1 is a valid QVariant,
    otherwise the getter is called.

    \snippet src_activeqt_container_qaxbase.cpp 17

    Note that it is faster to get and set properties using
    QObject::property() and QObject::setProperty().

    dynamicCall() can also be used to call objects with a
    \l{QAxBase::disableMetaObject()}{disabled metaobject} wrapper,
    which can improve performance significantely, esp. when calling many
    different objects of different types during an automation process.
    ActiveQt will then however not validate parameters.

    It is only possible to call functions through dynamicCall() that
    have parameters or return values of datatypes supported by
    QVariant. See the QAxBase class documentation for a list of
    supported and unsupported datatypes. If you want to call functions
    that have unsupported datatypes in the parameter list, use
    queryInterface() to retrieve the appropriate COM interface, and
    use the function directly.

    \snippet src_activeqt_container_qaxbase.cpp 18

    This is also more efficient.
*/
QVariant QAxBase::dynamicCall(const char *function,
                              const QVariant &var1,
                              const QVariant &var2,
                              const QVariant &var3,
                              const QVariant &var4,
                              const QVariant &var5,
                              const QVariant &var6,
                              const QVariant &var7,
                              const QVariant &var8)
{
    QVariantList vars = QAxBase::argumentsToList(var1, var2, var3, var4, var5, var6, var7, var8);
    return dynamicCall(function, vars); // Use overload taking "QVariantList &" to avoid recursion
}

/*!
    \overload

    Calls the COM object's method \a function, passing the
    parameters in \a vars, and returns the value returned by
    the method. If the method does not return a value or when
    the function call failed this function returns an invalid
    QVariant object.

    The QVariant objects in \a vars are updated when the method has
    out-parameters.
*/
QVariant QAxBase::dynamicCall(const char *function, QList<QVariant> &vars)
{
    return dynamicCall(function, vars, 0);
}

/*!
    \internal
*/
QVariant QAxBase::dynamicCall(const char *function, QList<QVariant> &vars, unsigned flags)
{
    VARIANTARG res;
    VariantInit(&res);

    QByteArray rettype;
    if (!dynamicCallHelper(function, &res, vars, rettype, flags))
        return QVariant();

    QVariant qvar = VARIANTToQVariant(res, rettype);
    const int vType = qvar.metaType().id();
    if ((res.vt != VT_DISPATCH && res.vt != VT_UNKNOWN)
        || vType == QMetaType::QPixmap || vType == QMetaType::QFont) {
        clearVARIANT(&res);
    }

    return qvar;
}

/*!
    Returns a pointer to a QAxObject wrapping the COM object provided
    by the method or property \a name, passing passing the parameters
    \a var1, \a var1, \a var2, \a var3, \a var4, \a var5, \a var6,
    \a var7 and \a var8.

    If \a name is provided by a method the string must include the
    full function prototype.

    If \a name is a property the string must be the name of the property,
    and \a var1, ... \a var8 are ignored.

    The returned QAxObject is a child of this object (which is either of
    type QAxObject or QAxWidget), and is deleted when this object is
    deleted. It is however safe to delete the returned object yourself,
    and you should do so when you iterate over lists of subobjects.

    COM enabled applications usually have an object model publishing
    certain elements of the application as dispatch interfaces. Use
    this method to navigate the hierarchy of the object model, e.g.

    \snippet src_activeqt_container_qaxbase.cpp 19
*/
QAxObject *QAxBase::querySubObject(const char *name,
                                   const QVariant &var1,
                                   const QVariant &var2,
                                   const QVariant &var3,
                                   const QVariant &var4,
                                   const QVariant &var5,
                                   const QVariant &var6,
                                   const QVariant &var7,
                                   const QVariant &var8)
{
    QVariantList vars;
    QVariant var = var1;
    int argc = 1;
    while(var.isValid()) {
        vars << var;
        switch(++argc) {
        case 2: var = var2; break;
        case 3: var = var3; break;
        case 4: var = var4; break;
        case 5: var = var5; break;
        case 6: var = var6; break;
        case 7: var = var7; break;
        case 8: var = var8; break;
        default:var = QVariant(); break;
        }
    }

    return querySubObject(name, vars);
}

/*!
    \overload

    The QVariant objects in \a vars are updated when the method has
    out-parameters.
*/
QAxObject *QAxBase::querySubObject(const char *name, QList<QVariant> &vars)
{
    QAxObject *object = nullptr;
    VARIANTARG res;
    VariantInit(&res);

    QByteArray rettype;
    if (!dynamicCallHelper(name, &res, vars, rettype))
        return nullptr;

    switch (res.vt) {
    case VT_DISPATCH:
        if (res.pdispVal) {
            if (rettype.isEmpty() || rettype == "IDispatch*" || rettype == "QVariant") {
                object = new QAxObject(res.pdispVal, qObject());
            } else if (QMetaType::fromName(rettype).id() != QMetaType::UnknownType) {
                QVariant qvar = VARIANTToQVariant(res, rettype, 0);
                object = *static_cast<QAxObject**>(qvar.data());
//                qVariantGet(qvar, object, rettype);
                res.pdispVal->AddRef();
            }
            if (object)
                static_cast<QAxBase*>(object)->d->tryCache = true;
        }
        break;
    case VT_UNKNOWN:
        if (res.punkVal) {
            if (rettype.isEmpty() || rettype == "IUnknown*") {
                object = new QAxObject(res.punkVal, qObject());
            } else if (QMetaType::fromName(rettype).id() != QMetaType::UnknownType) {
                QVariant qvar = VARIANTToQVariant(res, rettype, 0);
                object = *static_cast<QAxObject**>(qvar.data());
//                qVariantGet(qvar, object, rettype);
                res.punkVal->AddRef();
            }
            if (object)
                static_cast<QAxBase*>(object)->d->tryCache = true;
        }
        break;
    case VT_EMPTY:
#ifdef QT_CHECK_STATE
        {
            auto mo = qObject()->metaObject();
            const char *coclass = mo->classInfo(mo->indexOfClassInfo("CoClass")).value();
            qWarning("QAxBase::querySubObject: %s: Error calling function or property in %s (%s)"
                , name, control().toLatin1().data(), coclass ? coclass: "unknown");
        }
#endif
        break;
    default:
#ifdef QT_CHECK_STATE
        {
            auto mo = qObject()->metaObject();
            const char *coclass = mo->classInfo(mo->indexOfClassInfo("CoClass")).value();
            qWarning("QAxBase::querySubObject: %s: Method or property is not of interface type in %s (%s)"
                , name, control().toLatin1().data(), coclass ? coclass: "unknown");
        }
#endif
        break;
    }

    clearVARIANT(&res);
    return object;
}

class QtPropertyBag : public QComObject<IPropertyBag>
{
    Q_DISABLE_COPY_MOVE(QtPropertyBag)
public:
    QtPropertyBag() = default;
    virtual ~QtPropertyBag() = default;

    HRESULT __stdcall Read(LPCOLESTR name, VARIANT *var, IErrorLog *) override
    {
        if (!var)
            return E_POINTER;

        QString property = QString::fromWCharArray(name);
        QVariant qvar = map.value(property);
        QVariantToVARIANT(qvar, *var);
        return S_OK;
    }
    HRESULT __stdcall Write(LPCOLESTR name, VARIANT *var) override
    {
        if (!var)
            return E_POINTER;
        QString property = QString::fromWCharArray(name);
        QVariant qvar = VARIANTToQVariant(*var, nullptr);
        map[property] = qvar;

        return S_OK;
    }

    QAxBase::PropertyBag map;
};

/*!
    Returns a name:value map of all the properties exposed by the COM
    object.

    This is more efficient than getting multiple properties
    individually if the COM object supports property bags.

    \warning It is not guaranteed that the property bag implementation
    of the COM object returns all properties, or that the properties
    returned are the same as those available through the IDispatch
    interface.
*/
QAxBase::PropertyBag QAxBase::propertyBag() const
{
    PropertyBag result;

    if (!d->ptr && !d->initialized) {
        const_cast<QAxBase *>(this)->initialize(&d->ptr);
        d->initialized = true;
    }

    if (isNull())
        return result;
    ComPtr<IPersistPropertyBag> persist;
    d->ptr->QueryInterface(IID_IPersistPropertyBag, &persist);
    if (persist) {
        ComPtr<QtPropertyBag> pbag = makeComObject<QtPropertyBag>();
        persist->Save(pbag.Get(), false, true);
        result = pbag->map;
        return result;
    }
    const QMetaObject *mo = qObject()->metaObject();
    for (int p = mo->propertyOffset(); p < mo->propertyCount(); ++p) {
        const QMetaProperty property = mo->property(p);
        QVariant var = qObject()->property(property.name());
        result.insert(QLatin1String(property.name()), var);
    }
    return result;
}

/*!
    Sets the properties of the COM object to the corresponding values
    in \a bag.

    \warning
    You should only set property bags that have been returned by the
    propertyBag function, as it cannot be guaranteed that the property
    bag implementation of the COM object supports the same properties
    that are available through the IDispatch interface.

    \sa propertyBag()
*/
void QAxBase::setPropertyBag(const PropertyBag &bag)
{
    if (!d->ptr && !d->initialized) {
        initialize(&d->ptr);
        d->initialized = true;
    }

    if (isNull())
        return;
    ComPtr<IPersistPropertyBag> persist;
    d->ptr->QueryInterface(IID_IPersistPropertyBag, &persist);
    if (persist) {
        ComPtr<QtPropertyBag> pbag = makeComObject<QtPropertyBag>();
        pbag->map = bag;
        persist->Load(pbag.Get(), nullptr);
    } else {
        const QMetaObject *mo = qObject()->metaObject();
        for (int p = mo->propertyOffset(); p < mo->propertyCount(); ++p) {
            const QMetaProperty property = mo->property(p);
            QVariant var = bag.value(QLatin1String(property.name()));
            qObject()->setProperty(property.name(), var);
        }
    }
}

/*!
    Returns true if the property \a prop is writable; otherwise
    returns false. By default, all properties are writable.

    \warning
    Depending on the control implementation this setting might be
    ignored for some properties.

    \sa setPropertyWritable(), QAxBaseWidget::propertyChanged(), QAxBaseObject::propertyChanged()
*/
bool QAxBase::propertyWritable(const char *prop) const
{
    return d->propWritable.value(prop, true);
}

/*!
    Sets the property \a prop to writable if \a ok is true, otherwise
    sets \a prop to be read-only. By default, all properties are
    writable.

    \warning
    Depending on the control implementation this setting might be
    ignored for some properties.

    \sa propertyWritable(), QAxBaseWidget::propertyChanged(), QAxBaseObject::propertyChanged()
*/
void QAxBase::setPropertyWritable(const char *prop, bool ok)
{
    d->propWritable[prop] = ok;
}

/*!
    Returns true if there is no COM object loaded by this wrapper;
    otherwise return false.

    \sa control()
*/
bool QAxBase::isNull() const
{
    return !d->ptr;
}

/*!
    Returns a QVariant that wraps the COM object. The variant can
    then be used as a parameter in e.g. dynamicCall().
*/
QVariant QAxBase::asVariant() const
{
    if (!d->ptr && !d->initialized) {
        const_cast<QAxBase *>(this)->initialize(&d->ptr);
        d->initialized = true;
    }

    QVariant qvar;
    QByteArray cn(d->className());
    if (cn == "QAxObject" || cn == "QAxWidget" || cn == "QAxBase") {
        if (d->dispatch())
            qvar.setValue(d->dispatch());
        else if (d->ptr)
            qvar.setValue(d->ptr.Get());
    } else {
        cn.remove(0, cn.lastIndexOf(':') + 1);
        cn += '*';
        QObject *object = qObject();
        QMetaType metaType = QMetaType::fromName(cn);
        if (metaType.id() == QMetaType::UnknownType)
            metaType = QMetaType(qRegisterMetaType<QObject *>(cn));
        qvar = QVariant(metaType, &object);
    }

    return qvar;
}

// internal function that creates a QAxObject from an iface
// used by type-conversion code (types.cpp)
void *qax_createObjectWrapper(int metaType, IUnknown *iface)
{
    if (!iface)
        return nullptr;

    void *object = QMetaType(metaType).create(nullptr);
    QAxBasePrivate *d = reinterpret_cast<const QAxObject *>(object)->d;

    d->ptr.Attach(iface);
    d->initialized = true;

    // no release, since no addref

    return object;
}

/*!
    \internal
*/
QObject *QAxBase::qObject() const
{
    return d->qObject();
}

/*!
    \internal
*/
const char *QAxBase::className() const
{
    return d->className();
}

QT_END_NAMESPACE
