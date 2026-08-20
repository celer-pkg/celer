// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#include "qaxaggregated.h"

#ifdef QT_WIDGETS_LIB
#  include <QtWidgets/qwidget.h>
#endif

QT_BEGIN_NAMESPACE

/*!
    \class QAxAggregated
    \brief The QAxAggregated class is an abstract base class for implementations of
    additional COM interfaces.

    \inmodule QAxServer

    Create a subclass of QAxAggregated and reimplement
    queryInterface() to support additional COM interfaces. Use
    multiple inheritance from those COM interfaces. Implement the
    IUnknown interface of those COM interfaces by delegating the
    calls to \c QueryInterface(), \c AddRef() and \c Release() to the
    interface provided by controllingUnknown().

    Use the widget() method if you need to make calls to the QWidget
    implementing the ActiveX control. You must not store that pointer
    in your subclass (unless you use QPointer), as the QWidget can
    be destroyed by the ActiveQt framework at any time.

    \sa QAxBindable, QAxFactory, {Active Qt}
*/

/*!
    \fn QAxAggregated::~QAxAggregated()

    The destructor is called internally by Qt.
*/

/*!
    \fn long QAxAggregated::queryInterface(const QUuid &iid, void **iface)

    Reimplement this pure virtual function to support additional COM
    interfaces. Set the value of \a iface to point to this object to
    support the interface \a iid. Note that you must cast the \c
    this pointer to the appropriate superclass.

    \snippet src_activeqt_control_qaxbindable.cpp 2

    Return the standard COM results \c S_OK (interface is supported)
    or \c E_NOINTERFACE (requested interface is not supported).

    \warning
    Even though you must implement the \c IUnknown interface if you
    implement any COM interface you must not support the \c IUnknown
    interface in your queryInterface() implementation.
*/

/*!
    \fn IUnknown *QAxAggregated::controllingUnknown() const

    Returns the \c IUnknown interface of the ActiveX control. Implement
    the \c IUnknown interface in your QAxAggregated subclass to
    delegate calls to \c QueryInterface(), \c AddRef(), and \c
    Release() to the interface provided by this function.

    \snippet src_activeqt_control_qaxbindable.cpp 3

    Instead of declaring and implementing these three functions
    manually, you can use the \c QAXAGG_IUNKNOWN macro in the class
    declaration of your subclass.
*/

/*!
    \fn QObject *QAxAggregated::object() const

    Returns a pointer to the QObject subclass implementing the COM object.
    This function might return 0.

    \warning
    You must not store the returned pointer, unless you use a
    QPointer, since the QObject can be destroyed by ActiveQt at any
    time.
*/

/*!
    Returns a pointer to the QWidget subclass implementing the ActiveX control.
    This function might return 0.

    \warning
    You must not store the returned pointer, unless you use a
    QPointer, since the QWidget can be destroyed by ActiveQt at any
    time.
*/
QWidget *QAxAggregated::widget() const
{
#ifdef QT_WIDGETS_LIB
    return qobject_cast<QWidget*>(the_object);
#else
    return 0;
#endif
}

QT_END_NAMESPACE
