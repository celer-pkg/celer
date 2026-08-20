// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOMANEUVERDERIVED_P_H
#define QGEOMANEUVERDERIVED_P_H

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

#include "qgeomaneuver.h"

#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

// Only for QML. To prevent the class from being exported into qmltypes twice:
// for a value type, and for the enums.
struct QGeoManeuverDerived : public QGeoManeuver
{
    Q_GADGET
};

namespace QGeoManeuverForeignNamespace
{
    Q_NAMESPACE
    QML_FOREIGN_NAMESPACE(QGeoManeuverDerived)
    QML_NAMED_ELEMENT(RouteManeuver)
}

QT_END_NAMESPACE

#endif // QGEOMANEUVERDERIVED_P_H
