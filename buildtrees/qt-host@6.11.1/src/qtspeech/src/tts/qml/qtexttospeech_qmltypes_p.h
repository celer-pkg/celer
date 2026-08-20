// Copyright (C) 2022 The Qt Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTTEXTTOSPEECHTYPES_H
#define QTTEXTTOSPEECHTYPES_H

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

#include <QtQml/qqml.h>
#include <QtTextToSpeech/QtTextToSpeech>

QT_BEGIN_NAMESPACE

struct QVoiceForeign
{
    Q_GADGET
    QML_FOREIGN(QVoice)
    QML_VALUE_TYPE(voice)
};

// To prevent the same QVoice type from being exported twice into qmltypes
// (for value type and for the enums)
struct QVoiceDerived : public QVoice
{
    Q_GADGET
};

namespace QVoiceForeignNamespace
{
    Q_NAMESPACE
    QML_FOREIGN_NAMESPACE(QVoiceDerived)
    QML_NAMED_ELEMENT(Voice)
}

QT_END_NAMESPACE

#endif // QTTEXTTOSPEECHTYPES_H
