// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qplaceicon.h"
#include "qplaceicon_p.h"
#include "qplacemanager.h"
#include "qplacemanagerengine.h"

#include <QtCore/QVariant>
#include <QtQml/QJSValue>

QT_USE_NAMESPACE

QT_DEFINE_QSDP_SPECIALIZATION_DTOR(QPlaceIconPrivate)

bool QPlaceIconPrivate::operator == (const QPlaceIconPrivate &other) const
{
    return manager == other.manager
            && parameters == other.parameters;
}

/*!
    \class QPlaceIcon
    \inmodule QtLocation
    \ingroup QtLocation-places
    \ingroup QtLocation-places-data
    \since 5.6

    \brief The QPlaceIcon class represents an icon.

    The typical usage of an icon is to use the url() function to specify
    a preferred icon size.

    \snippet places/requesthandler.h icon

    The icons are typically backend dependent, if a manager backend does not support a given size, the URL of the icon that most
    closely matches those parameters is returned.

    The icon class also has a key-value set of parameters.  The precise key one
    needs to use depends on the \l {Qt Location#Plugin References and Parameters}{plugin}
    being used.  These parameters influence which icon URL is returned by
    the manager and may also be used to specify icon URL locations when
    saving icons.

    If there is only ever one image for an icon, then QPlaceIcon::SingleUrl can be used as a parameter
    key with a QUrl as the associated value.  If this key is set, then the url() function will always return the specified URL
    and not defer to any manager.
*/

/*!
    \qmlvaluetype icon
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-places
    \ingroup qml-QtLocation5-places-data
    \since QtLocation 5.5

    \brief The icon type represents the icon of a place.

    The typical usage of an icon is to use the url() function to specify
    a preferred icon size.

    The icons are typically backend dependent, if a manager backend does not support a given size, the URL of the icon that most
    closely matches those parameters is returned.

    The icon class also has a key-value set of parameters.  The precise key one
    needs to use depends on the \l {Qt Location#Plugin References and Parameters}{plugin}
    being used.  These parameters influence which icon URL is returned by
    the manager and may also be used to specify icon URL locations when
    saving icons.

    If there is only ever one image for an icon, then QPlaceIcon::SingleUrl can be used as a parameter
    key with a QUrl as the associated value.  If this key is set, then the url() function will always return the specified URL
    and not defer to any manager.
*/

/*!
    \variable QPlaceIcon::SingleUrl
    \brief Parameter key for an icon that only has a single image URL.

    The parameter value to be used with this key is a QUrl.  An icon with this parameter set will
    always return the specified URL regardless of the requested size when url() is called.
*/
const QString QPlaceIcon::SingleUrl(QLatin1String("singleUrl"));

/*!
    Constructs an icon.
*/
QPlaceIcon::QPlaceIcon()
    : d(new QPlaceIconPrivate)
{
}

/*!
    Constructs a copy of \a other.
*/
QPlaceIcon::QPlaceIcon(const QPlaceIcon &other) noexcept = default;

/*!
    Destroys the icon.
*/
QPlaceIcon::~QPlaceIcon() = default;

/*!
    Assigns \a other to this icon and returns a reference to this icon.
*/
QPlaceIcon &QPlaceIcon::operator=(const QPlaceIcon &other) noexcept
{
    if (this == &other)
        return *this;

    d = other.d;
    return *this;
}

/*!
    \fn bool QPlaceIcon::operator==(const QPlaceIcon &lhs, const QPlaceIcon &rhs) noexcept

    Returns true if \a lhs is equal to \a rhs, otherwise returns false.
*/

/*!
    \fn bool QPlaceIcon::operator!=(const QPlaceIcon &lhs, const QPlaceIcon &rhs) noexcept

    Returns true if \a lhs is not equal to \a rhs, otherwise returns false.
*/

bool QPlaceIcon::isEqual(const QPlaceIcon &other) const noexcept
{
    return *d == *(other.d);
}

/* ### Need to evaluate whether we need this at all (or perhaps only for tests)
    \qmlproperty Plugin Icon::plugin

    The property holds the plugin that is responsible for managing this icon.
*/

/*!
    Returns an icon URL according to the given \a size.

    If no manager has been assigned to the icon, and the parameters do not contain the QPlaceIcon::SingleUrl key, a default constructed QUrl
    is returned.
*/
QUrl QPlaceIcon::url(const QSize &size) const
{
    if (d->parameters.contains(QPlaceIcon::SingleUrl)) {
        QVariant value = d->parameters.value(QPlaceIcon::SingleUrl);
        if (value.typeId() == QMetaType::QUrl)
            return value.toUrl();
        else if (value.typeId() == QMetaType::QString)
            return QUrl::fromUserInput(value.toString());

        return QUrl();
    }

    if (!d->manager)
        return QUrl();

    return d->manager->d->constructIconUrl(*this, size);
}

/*!
    \property QPlaceIcon::parameters
    \brief the set of parameters for the icon that are manager or plugin specific.
*/

/*!
    Returns a set of parameters for the icon that are manager/plugin specific.
    These parameters are used by the manager to return the appropriate
    URL when url() is called and to specify locations to save to
    when saving icons.

    Consult the \l {Qt Location#Plugin References and Parameters}{plugin documentation}
    for what parameters are supported and how they should be used.
*/
QVariantMap QPlaceIcon::parameters() const
{
    return d->parameters;
}

/*!
    Sets the parameters of the icon to \a parameters.
*/
void QPlaceIcon::setParameters(const QVariantMap &parameters)
{
    d->parameters = parameters;
}

/*!
    \property QPlaceIcon::manager
    \brief the manager of this icon.
*/

/*!
    Returns the manager that this icon is associated with.
*/
QPlaceManager *QPlaceIcon::manager() const
{
    return d->manager;
}

/*!
    Sets the \a manager that this icon is associated with.  The icon does not take
    ownership of the pointer.
*/
void QPlaceIcon::setManager(QPlaceManager *manager)
{
    d->manager = manager;
}

/*!
    Returns a boolean indicating whether the all the fields of the icon are empty or not.
*/
bool QPlaceIcon::isEmpty() const
{
    return (d->manager == 0
            && d->parameters.isEmpty());
}

#include "moc_qplaceicon.cpp"
