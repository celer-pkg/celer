// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtLocation/QLocation>

QT_BEGIN_NAMESPACE

namespace QLocation {

/*!
    \namespace QLocation
    \inmodule QtLocation
    \keyword QLocation Namespace

    \brief The QLocation namespace contains miscellaneous identifiers used throughout the
           QtLocation module.
*/

/*!
    \enum QLocation::Visibility

    Defines the visibility of a QPlace or QPlaceCategory.

    \value UnspecifiedVisibility    No explicit visibility has been defined.
    \value DeviceVisibility         Places and categories with DeviceVisibility are only stored on
                                    the local device.
    \value PrivateVisibility        Places and categories with PrivateVisibility are only visible
                                    to the current user.  The data may be stored either locally or
                                    on a remote service or both.
    \value PublicVisibility         Places and categories with PublicVisibility are visible to
                                    everyone.

    A particular manager may support one or more visibility scopes.  For example a manager from one provider may only provide places
    that are public to everyone, whilst another may provide both public and private places.

    \note The meaning of unspecified visibility depends on the context it is used.

    When \e saving a place or category, the
    default visibility is unspecified meaning that the manager chooses an appropriate visibility scope for the item.

    When \e searching for places, unspecified means that places of any scope is returned.
*/

/*!
    \enum QLocation::ReferenceSurface
    \since 6.6

    Defines the reference surface on which various map items (e.g. polygons, polylines) are defined.

    \value Map                      Items are defined on a map. This means, e.g. for a polyline that
                                    nodes are connected with straight lines on the map.
    \value Globe                    Items are defined on the globe. This means, e.g. for a polyine
                                    that nodes are connected with circle sections that represent the
                                    shortest connection between points on a sphere. This connection is
                                    also known as great circle path.
*/
}

QT_END_NAMESPACE
