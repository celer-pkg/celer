// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#include "qaxobject.h"
#include "qaxobject_p.h"
#include "qaxbase_p.h"

#include <quuid.h>
#include <qmetaobject.h>
#include <qstringlist.h>

#include <windows.h>

#include <QtCore/private/qcomptr_p.h>

QT_BEGIN_NAMESPACE

QAxObjectInterface::~QAxObjectInterface() = default;

QAxBaseObject::QAxBaseObject(QObjectPrivate &d, QObject *parent)
    : QObject(d, parent)
{
}

/*!
   \class QAxBaseObject
   \brief QAxBaseObject provides static properties and signals for QAxObject.
   \inmodule QAxContainer
   \since 6.0
*/

/*!
    \property QAxBaseObject::classContext
    \brief the context the ActiveX control will run in (default CLSCTX_SERVER).

    The property affects the "dwClsContext" argument when calling
    CoCreateInstance. This can be used to control in-proc vs. out-of-proc
    startup for controls supporting both alternatives. Also, it can be used to
    modify/reduce control permissions when used with CLSCTX_ENABLE_CLOAKING
    and an impersonation token.

    Note that it must be set before setControl() to have any effect.
    \sa QAxBaseWidget::control
*/

/*!
    \property QAxBaseObject::control
    \brief the name of the COM object wrapped by this QAxBaseObject object.

    Setting this property initializes the COM object. Any COM object
    previously set is shut down.

    The most efficient way to set this property is by using the
    registered component's UUID, e.g.
    \sa QAxBaseWidget::control, QAxBaseWidget::classContext
*/

/*!
    \fn void QAxBaseObject::signal(const QString &name, int argc, void *argv)

    This generic signal gets emitted when the COM object issues the
    event \a name. \a argc is the number of parameters provided by the
    event (DISPPARAMS.cArgs), and \a argv is the pointer to the
    parameter values (DISPPARAMS.rgvarg). Note that the order of parameter
    values is turned around, ie. the last element of the array is the first
    parameter in the function.

    \sa QAxBaseWidget::signal()
*/

/*!
    \fn void QAxBaseObject::propertyChanged(const QString &name)

    If the COM object supports property notification, this signal gets
    emitted when the property called \a name is changed.

    \sa QAxBaseWidget::propertyChanged()
*/

/*!
    \fn void QAxBaseObject::exception(int code, const QString &source, const QString &desc, const QString &help)

    This signal is emitted when the COM object throws an exception while called using the OLE automation
    interface IDispatch. \a code, \a source, \a desc and \a help provide information about the exception as
    provided by the COM server and can be used to provide useful feedback to the end user. \a help includes
    the help file, and the help context ID in brackets, e.g. "filename [id]".

    \sa QAxBaseWidget::exception()
*/

void QAxObjectPrivate::emitException(int code, const QString &source, const QString &desc,
                                     const QString &help)
{
    Q_Q(QAxObject);
    emit q->exception(code, source, desc, help);
}

void  QAxObjectPrivate::emitPropertyChanged(const QString &name)
{
    Q_Q(QAxObject);
    emit q->propertyChanged(name);
}

void  QAxObjectPrivate::emitSignal(const QString &name, int argc, void *argv)
{
    Q_Q(QAxObject);
    emit q->signal(name, argc, argv);
}

/*!
    \class QAxObject
    \brief The QAxObject class provides a QObject that wraps a COM object.

    \inmodule QAxContainer

    A QAxObject can be instantiated as an empty object, with the name
    of the COM object it should wrap, or with a pointer to the
    IUnknown that represents an existing COM object. If the COM object
    implements the \c IDispatch interface, the properties, methods and
    events of that object become available as Qt properties, slots and
    signals. The base class, QAxBase, provides an API to access the
    COM object directly through the IUnknown pointer.

    QAxObject is a QObject and can be used as such, e.g. it can be
    organized in an object hierarchy, receive events and connect to
    signals and slots.

    QAxObject also inherits most of its ActiveX-related functionality
    from QAxBase, notably dynamicCall() and querySubObject().

    \warning
    You can subclass QAxObject, but you cannot use the Q_OBJECT macro
    in the subclass (the generated moc-file will not compile), so you
    cannot add further signals, slots or properties. This limitation is
    due to the metaobject information generated in runtime.
    To work around this problem, aggregate the QAxObject as a member of
    the QObject subclass.

    \sa QAxBase, QAxWidget, QAxScript, {ActiveQt Framework}
*/

/*!
    Creates an empty COM object and propagates \a parent to the
    QObject constructor. To initialize the object, call setControl().

    \sa {control}{setControl()}
*/
QAxObject::QAxObject(QObject *parent)
: QAxBaseObject(*new QAxObjectPrivate, parent)
{
    Q_D(QAxObject);
    axBaseInit(d);
}

/*!
    Creates a QAxObject that wraps the COM object \a c. \a parent is
    propagated to the QObject constructor.

    \sa {control}{setControl()}
*/
QAxObject::QAxObject(const QString &c, QObject *parent)
: QAxBaseObject(*new QAxObjectPrivate, parent)
{
    Q_D(QAxObject);
    axBaseInit(d);
    setControl(c);
}

/*!
    Creates a QAxObject that wraps the COM object referenced by \a
    iface. \a parent is propagated to the QObject constructor.
*/
QAxObject::QAxObject(IUnknown *iface, QObject *parent)
: QAxBaseObject(*new QAxObjectPrivate, parent)
{
    Q_D(QAxObject);
    axBaseInit(d, iface);
}

/*!
    Releases the COM object and destroys the QAxObject,
    cleaning up all allocated resources.
*/
QAxObject::~QAxObject()
{
    Q_D(QAxObject);
    d->clear();
}

unsigned long QAxObject::classContext() const
{
    return QAxBase::classContext();
}

void QAxObject::setClassContext(ulong classContext)
{
    QAxBase::setClassContext(classContext);
}

QString QAxObject::control() const
{
    return QAxBase::control();
}

bool QAxObject::setControl(const QString &c)
{
    return QAxBase::setControl(c);
}

void QAxObject::clear()
{
    resetControl();
}

void QAxObject::resetControl()
{
    Q_D(QAxObject);
    d->clear();
}

void QAxObjectPrivate::clear()
{
    Q_Q(QAxObject);
    q->QAxBase::clear();
}

/*!
    \internal
*/
void QAxObject::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    QAxBasePrivate::qtStaticMetaCall(static_cast<QAxObject *>(_o), _c, _id, _a);
}

const QMetaObject *QAxObjectPrivate::fallbackMetaObject() const
{
    return &QAxObject::staticMetaObject;
}

/*!
    \internal
*/
const QMetaObject *QAxObject::metaObject() const
{
    return QAxBase::axBaseMetaObject();
}

const QMetaObject *QAxObjectPrivate::parentMetaObject() const
{
    return &QAxBaseObject::staticMetaObject;
}

/*!
    \internal
*/
void *QAxObject::qt_metacast(const char *cname)
{
    if (!qstrcmp(cname, "QAxObject")) return static_cast<void *>(this);
    if (!qstrcmp(cname, "QAxBase")) return static_cast<QAxBase *>(this);
    return QAxBaseObject::qt_metacast(cname);
}

QObject* QAxObjectPrivate::qObject() const
{
    Q_Q(const QAxObject);
    return static_cast<QObject *>(const_cast<QAxObject *>(q));
}
const char *QAxObjectPrivate::className() const
{
    return "QAxObject";
}

/*!
    \internal
*/
int QAxObject::qt_metacall(QMetaObject::Call call, int id, void **v)
{
    Q_D(QAxObject);
    id = QAxBaseObject::qt_metacall(call, id, v);
    if (id < 0)
        return id;
    return d->qtMetaCall(call, id, v);
}

/*!
    \fn QObject *QAxObject::qObject() const
    \internal
*/

/*!
    \reimp
*/
void QAxObject::connectNotify(const QMetaMethod &)
{
    QAxBase::connectNotify();
}

/*!
    \since 4.1

    Requests the COM object to perform the action \a verb. The
    possible verbs are returned by verbs().

    The function returns true if the object could perform the action, otherwise returns false.
*/
bool QAxObject::doVerb(const QString &verb)
{
    if (!verbs().contains(verb))
        return false;
    ComPtr<IOleObject> ole;
    queryInterface(IID_IOleObject, &ole);
    if (!ole)
        return false;

    LONG index = indexOfVerb(verb);

    HRESULT hres = ole->DoVerb(index, nullptr, nullptr, 0, nullptr, nullptr);

    return hres == S_OK;
}

QT_END_NAMESPACE
