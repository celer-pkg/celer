// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QMLDOMFORMATSCANNER_P_H
#define QMLDOMFORMATSCANNER_P_H

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

#include <QtQml/private/qqmljssourcelocation_p.h>

#include <QtCore/QMap>

QT_BEGIN_NAMESPACE

namespace QmlFormat {

using DisabledRegions = QMap<quint32, QQmlJS::SourceLocation>;
enum class Directive { On, Off };

[[nodiscard]] std::optional<Directive> directiveFromComment(QStringView commentLine);
[[nodiscard]] DisabledRegions identifyDisabledRegions(QStringView code, const QList<QQmlJS::SourceLocation> &comments);

} // end namespace QmlFormat

QT_END_NAMESPACE
#endif // QMLDOMFORMATSCANNER_P_H
