// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGRENDERSKIN_H
#define QSSGRENDERSKIN_H

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

#include <QtQuick3DRuntimeRender/private/qssgrendertexturedata_p.h>

QT_BEGIN_NAMESPACE

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderSkin : public QSSGRenderTextureData
{
    explicit QSSGRenderSkin();
    ~QSSGRenderSkin() override;
    Q_DISABLE_COPY(QSSGRenderSkin)

    QByteArray &boneData();
    quint32 boneCount = 0;
};
QT_END_NAMESPACE

#endif
