// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeocodingmanagerengine.h"
#include "qgeocodingmanagerengine_p.h"

#include "qgeoaddress.h"
#include "qgeocoordinate.h"

#include <QtPositioning/QGeoShape>

QT_BEGIN_NAMESPACE

/*!
    \class QGeoCodingManagerEngine
    \inmodule QtLocation
    \ingroup QtLocation-impl
    \since 5.6

    \brief The QGeoCodingManagerEngine class provides an interface and
    convenience methods to implementers of QGeoServiceProvider plugins who want
    to provide support for geocoding operations.

    \note There are no source or binary compatibility guarantees for the
    backend classes. The API is only guaranteed to work with the Qt version it
    was developed against. API changes will however only be made in minor
    releases. (6.6, 6.7, and so on.)

    In the default implementation, supportsGeocoding() and
    supportsReverseGeocoding() returns false while geocode() and
    reverseGeocode() cause QGeoCodeReply::UnsupportedOptionError to occur.

    If the service provider supports geocoding the subclass should provide an
    implementation of geocode() and call setSupportsGeocoding(true) at
    some point in time before geocode() is called.

    Similarly, if the service provider supports reverse geocoding the subclass
    should provide an implementation reverseGeocode() and call
    setSupportsReverseGeocoding(true) at some point in time before
    reverseGeocode() is called.

    A subclass of QGeoCodingManagerEngine will often make use of a subclass
    fo QGeoCodeReply internally, in order to add any engine-specific
    data (such as a QNetworkReply object for network-based services) to the
    QGeoCodeReply instances used by the engine.

    \sa QGeoCodingManager
*/

/*!
    Constructs a new engine with the specified \a parent, using \a parameters
    to pass any implementation specific data to the engine.
*/
QGeoCodingManagerEngine::QGeoCodingManagerEngine(const QVariantMap &parameters, QObject *parent)
    : QObject(parent),
      d_ptr(new QGeoCodingManagerEnginePrivate())
{
    Q_UNUSED(parameters);
}

/*!
    Destroys this engine.
*/
QGeoCodingManagerEngine::~QGeoCodingManagerEngine()
{
    delete d_ptr;
}

/*!
    Sets the name which this engine implementation uses to distinguish itself
    from the implementations provided by other plugins to \a managerName.

    The combination of managerName() and managerVersion() should be unique
    amongst plugin implementations.
*/
void QGeoCodingManagerEngine::setManagerName(const QString &managerName)
{
    d_ptr->managerName = managerName;
}

/*!
    Returns the name which this engine implementation uses to distinguish
    itself from the implementations provided by other plugins.

    The combination of managerName() and managerVersion() should be unique
    amongst plugin implementations.
*/
QString QGeoCodingManagerEngine::managerName() const
{
    return d_ptr->managerName;
}

/*!
    Sets the version of this engine implementation to \a managerVersion.

    The combination of managerName() and managerVersion() should be unique
    amongst plugin implementations.
*/
void QGeoCodingManagerEngine::setManagerVersion(int managerVersion)
{
    d_ptr->managerVersion = managerVersion;
}

/*!
    Returns the version of this engine implementation.

    The combination of managerName() and managerVersion() should be unique
    amongst plugin implementations.
*/
int QGeoCodingManagerEngine::managerVersion() const
{
    return d_ptr->managerVersion;
}

/*!
    Begins the geocoding of \a address. Geocoding is the process of finding a
    coordinate that corresponds to a given address.

    A QGeoCodeReply object will be returned, which can be used to manage the
    geocoding operation and to return the results of the operation.

    This engine and the returned QGeoCodeReply object will emit signals
    indicating if the operation completes or if errors occur.

    If supportsGeocoding() returns false an
    QGeoCodeReply::UnsupportedOptionError will occur.

    Once the operation has completed, QGeoCodeReply::locations() can be used to
    retrieve the results, which will consist of a list of QGeoLocation objects.
    These objects represent a combination of coordinate and address data.

    The address data returned in the results may be different from \a address.
    This will usually occur if the geocoding service backend uses a different
    canonical form of addresses or if \a address was only partially filled out.

    If \a bounds is non-null and a valid QGeoShape it will be used to
    limit the results to those that are contained by \a bounds. This is
    particularly useful if \a address is only partially filled out, as the
    service will attempt to geocode all matches for the specified data.

    The user is responsible for deleting the returned reply object, although
    this can be done in the slot connected to QGeoCodingManagerEngine::finished(),
    QGeoCodingManagerEngine::errorOccurred(), QGeoCodeReply::finished() or
    QGeoCodeReply::errorOccurred() with deleteLater().
*/
QGeoCodeReply *QGeoCodingManagerEngine::geocode(const QGeoAddress &address,
        const QGeoShape &bounds)
{
    Q_UNUSED(address);
    Q_UNUSED(bounds);
    return new QGeoCodeReply(QGeoCodeReply::UnsupportedOptionError,
                               QLatin1String("Geocoding is not supported by this service provider."), this);
}

/*!
    Begins the reverse geocoding of \a coordinate. Reverse geocoding is the
    process of finding an address that corresponds to a given coordinate.

    A QGeoCodeReply object will be returned, which can be used to manage the
    reverse geocoding operation and to return the results of the operation.

    This engine and the returned QGeoCodeReply object will emit signals
    indicating if the operation completes or if errors occur.

    If supportsReverseGeocoding() returns false an
    QGeoCodeReply::UnsupportedOptionError will occur.

    At that point QGeoCodeReply::locations() can be used to retrieve the
    results, which will consist of a list of QGeoLocation objects. These objects
    represent a combination of coordinate and address data.

    The coordinate data returned in the results may be different from \a
    coordinate. This will usually occur if the reverse geocoding service
    backend shifts the coordinates to be closer to the matching addresses, or
    if the backend returns results at multiple levels of detail.

    If multiple results are returned by the reverse geocoding service backend
    they will be provided in order of specificity. This normally occurs if the
    backend is configured to reverse geocode across multiple levels of detail.
    As an example, some services will return address and coordinate pairs for
    the street address, the city, the state and the country.

    If \a bounds is non-null and a valid QGeoShape it will be used to
    limit the results to those that are contained by \a bounds.

    The user is responsible for deleting the returned reply object, although
    this can be done in the slot connected to QGeoCodingManagerEngine::finished(),
    QGeoCodingManagerEngine::errorOccurred(), QGeoCodeReply::finished() or
    QGeoCodeReply::errorOccurred() with deleteLater().
*/
QGeoCodeReply *QGeoCodingManagerEngine::reverseGeocode(const QGeoCoordinate &coordinate,
                                                       const QGeoShape &bounds)
{
    Q_UNUSED(coordinate);
    Q_UNUSED(bounds);
    return new QGeoCodeReply(QGeoCodeReply::UnsupportedOptionError,
                               QLatin1String("Reverse geocoding is not supported by this service provider."), this);
}

/*!
    Begins geocoding for a location matching \a address.

    A QGeoCodeReply object will be returned, which can be used to manage the
    geocoding operation and to return the results of the operation.

    This engine and the returned QGeoCodeReply object will emit signals
    indicating if the operation completes or if errors occur.

    Once the operation has completed, QGeoCodeReply::locations() can be used to
    retrieve the results, which will consist of a list of QGeoLocation objects.
    These objects represent a combination of coordinate and address data.

    If \a limit is -1 the entire result set will be returned, otherwise at most
    \a limit results will be returned.

    The \a offset parameter is used to ask the geocoding service to not return the
    first \a offset results.

    The \a limit and \a offset results are used together to implement paging.

    If \a bounds is non-null and a valid QGeoShape it will be used to
    limit the results to those that are contained by \a bounds.

    The user is responsible for deleting the returned reply object, although
    this can be done in the slot connected to QGeoCodingManagerEngine::finished(),
    QGeoCodingManagerEngine::errorOccurred(), QGeoCodeReply::finished() or
    QGeoCodeReply::errorOccurred() with deleteLater().
*/
QGeoCodeReply *QGeoCodingManagerEngine::geocode(const QString &address,
                                                int limit,
                                                int offset,
                                                const QGeoShape &bounds)
{
    Q_UNUSED(address);
    Q_UNUSED(limit);
    Q_UNUSED(offset);
    Q_UNUSED(bounds);

    return new QGeoCodeReply(QGeoCodeReply::UnsupportedOptionError,
                             QLatin1String("Searching is not supported by this service provider."), this);
}

/*!
    Sets the locale to be used by this manager to \a locale.

    If this geocoding manager supports returning the results
    in different languages, they will be returned in the language of \a locale.

    The locale used defaults to the system locale if this is not set.
*/
void QGeoCodingManagerEngine::setLocale(const QLocale &locale)
{
    d_ptr->locale = locale;
}

/*!
    Returns the locale used to hint to this geocoding manager about what
    language to use for the results.
*/
QLocale QGeoCodingManagerEngine::locale() const
{
    return d_ptr->locale;
}

/*!
\fn void QGeoCodingManagerEngine::finished(QGeoCodeReply *reply)

    This signal is emitted when \a reply has finished processing.

    If reply::error() equals QGeoCodeReply::NoError then the processing
    finished successfully.

    This signal and QGeoCodeReply::finished() will be emitted at the same
    time.

    \note Do not delete the \a reply object in the slot connected to this
    signal. Use deleteLater() instead.
*/

/*!
\fn void QGeoCodingManagerEngine::errorOccurred(QGeoCodeReply *reply, QGeoCodeReply::Error error, const QString &errorString)

    This signal is emitted when an error has been detected in the processing of
    \a reply. The QGeoCodingManagerEngine::finished() signal will probably follow.

    The error will be described by the error code \a error. If \a errorString is
    not empty it will contain a textual description of the error.

    This signal and QGeoCodeReply::errorOccurred() will be emitted at the same time.

    \note Do not delete the \a reply object in the slot connected to this
    signal. Use deleteLater() instead.
*/

QT_END_NAMESPACE
