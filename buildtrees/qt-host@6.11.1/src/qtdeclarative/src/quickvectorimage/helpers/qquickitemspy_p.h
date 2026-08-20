// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKITEMSPY_P_H
#define QQUICKITEMSPY_P_H

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

#include <QtCore/qpointer.h>
#include <QtQuick/qquickitem.h>
#include <QtQuickVectorImageHelpers/qtquickvectorimagehelpersexports.h>

QT_BEGIN_NAMESPACE

class Q_QUICKVECTORIMAGEHELPERS_EXPORT QQuickItemSpy : public QQuickItem
{
    Q_OBJECT

    QML_NAMED_ELEMENT(ItemSpy)
    Q_PROPERTY(QSizeF requiredTextureSize READ requiredTextureSize NOTIFY requiredTextureSizeChanged FINAL)

public:
    QQuickItemSpy(QQuickItem *parent = nullptr);
    ~QQuickItemSpy() override;

    QSizeF requiredTextureSize() const;

    void itemChange(ItemChange change, const ItemChangeData &value) override;

Q_SIGNALS:
    void requiredTextureSizeChanged();
};

QT_END_NAMESPACE

#endif // QQUICKITEMSPY_P_H

