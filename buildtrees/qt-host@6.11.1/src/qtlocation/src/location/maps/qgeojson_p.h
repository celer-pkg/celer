// Copyright (C) 2018 The Qt Company Ltd.
// Copyright (C) 2018 Julian Sherollari <jdotsh@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOJSON_H
#define QGEOJSON_H

#include <QtLocation/private/qlocationglobal_p.h>
#include <QtCore/QVariantList>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class QJsonDocument;
class QString;

namespace QGeoJson {
    // This method imports a GeoJSON file to a QVariantList
    Q_LOCATION_EXPORT QVariantList importGeoJson(const QJsonDocument &doc);

    // This method exports a GeoJSON file from a QVariantList
    Q_LOCATION_EXPORT QJsonDocument exportGeoJson(const QVariantList &list);

    // This method exports the content of the imported QVariantList in a
    // readable format
    Q_LOCATION_EXPORT QString toString(const QVariantList &importedGeoJson);
}

QT_END_NAMESPACE

#endif // QGEOJSON_H
