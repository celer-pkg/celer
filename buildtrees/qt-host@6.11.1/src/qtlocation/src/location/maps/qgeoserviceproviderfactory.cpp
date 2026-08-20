// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeoserviceproviderfactory.h"

QT_BEGIN_NAMESPACE

/*!
    \class QGeoServiceProviderFactory
    \inmodule QtLocation
    \ingroup QtLocation-impl
    \since 5.6
    \deprecated

    \brief The QGeoServiceProviderFactory class is a factory class used as the
    plugin interface for services related to geographical information.

    \note There are no source or binary compatibility guarantees for the
    backend classes. The API is only guaranteed to work with the Qt version it
    was developed against. API changes will however only be made in minor
    releases. (6.6, 6.7, and so on.)

    Implementers must provide a unique combination of providerName() and
    providerVersion() per plugin.

    The other functions should be overridden if the plugin supports the
    associated set of functionality.
*/

/*!
    \fn QGeoServiceProviderFactory::~QGeoServiceProviderFactory()

    Destroys this QGeoServiceProviderFactory instance.
*/

/*!
    Returns a new QGeoCodingManagerEngine instance, initialized with \a
    parameters, which implements the location geocoding functionality.

    If \a error is not \nullptr it should be set to QGeoServiceProvider::NoError on
    success or an appropriate QGeoServiceProvider::Error on failure.

    If \a errorString is not \nullptr it should be set to a string describing any
    error which occurred.

    The default implementation returns \nullptr, which causes a
    QGeoServiceProvider::NotSupportedError in QGeoServiceProvider.
*/
QGeoCodingManagerEngine *
QGeoServiceProviderFactory::createGeocodingManagerEngine(const QVariantMap &parameters,
                                                         QGeoServiceProvider::Error *error,
                                                         QString *errorString) const
{
    Q_UNUSED(parameters);
    Q_UNUSED(error);
    Q_UNUSED(errorString);

    return nullptr;
}

/*!
    Returns a new QGeoMappingManagerEngine instance, initialized with \a
    parameters, which implements mapping functionality.

    If \a error is not \nullptr it should be set to QGeoServiceProvider::NoError on
    success or an appropriate QGeoServiceProvider::Error on failure.

    If \a errorString is not \nullptr it should be set to a string describing any
    error which occurred.

    The default implementation returns \nullptr, which causes a
    QGeoServiceProvider::NotSupportedError in QGeoServiceProvider.

    \internal
*/
QGeoMappingManagerEngine *
QGeoServiceProviderFactory::createMappingManagerEngine(const QVariantMap &parameters,
                                                       QGeoServiceProvider::Error *error,
                                                       QString *errorString) const
{
    Q_UNUSED(parameters);
    Q_UNUSED(error);
    Q_UNUSED(errorString);

    return nullptr;
}

/*!
    Returns a new QGeoRoutingManagerEngine instance, initialized with \a
    parameters, which implements routing functionality.

    If \a error is not \nullptr it should be set to QGeoServiceProvider::NoError on
    success or an appropriate QGeoServiceProvider::Error on failure.

    If \a errorString is not \nullptr it should be set to a string describing any
    error which occurred.

    The default implementation returns \nullptr, which causes a
    QGeoServiceProvider::NotSupportedError in QGeoServiceProvider.
*/
QGeoRoutingManagerEngine *
QGeoServiceProviderFactory::createRoutingManagerEngine(const QVariantMap &parameters,
                                                       QGeoServiceProvider::Error *error,
                                                       QString *errorString) const

{
    Q_UNUSED(parameters);
    Q_UNUSED(error);
    Q_UNUSED(errorString);

    return nullptr;
}

/*!
    Returns a new QPlaceManagerEngine instance, initialized with \a
    parameters, which implements the place searching functionality.

    If \a error is not \nullptr it should be set to QGeoServiceProvider::NoError on
    success or an appropriate QGeoServiceProvider::Error on failure.

    If \a errorString is not \nullptr it should be set to a string describing any
    error which occurred.

    The default implementation returns \nullptr, which causes a
    QGeoServiceProvider::NotSupportedError in QGeoServiceProvider.
*/
QPlaceManagerEngine *
QGeoServiceProviderFactory::createPlaceManagerEngine(const QVariantMap &parameters,
                                                     QGeoServiceProvider::Error *error,
                                                     QString *errorString) const
{
    Q_UNUSED(parameters);
    Q_UNUSED(error);
    Q_UNUSED(errorString);

    return nullptr;
}

/*!
    Notify the plugin when the qml \a engine is ready. In this moment the plugins can use it
    to register Image Providers.

    The default implementation does nothing.
    \since 5.12
*/
void QGeoServiceProviderFactory::setQmlEngine(QQmlEngine *engine)
{
    Q_UNUSED(engine);
}

QT_END_NAMESPACE
