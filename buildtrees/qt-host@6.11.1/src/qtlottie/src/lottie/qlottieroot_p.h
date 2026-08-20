// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIEROOT_P_H
#define QLOTTIEROOT_P_H

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

#include <QtLottie/private/qlottiebase_p.h>

QT_BEGIN_NAMESPACE

class Q_LOTTIE_EXPORT QLottieRoot : public QLottieBase
{
public:
    QLottieRoot() = default;
    explicit QLottieRoot(const QLottieRoot &other);

    QLottieBase *clone() const override;
    QSize layerSize() const override;

    int parseSource(const QByteArray &jsonSource, const QUrl &fileSource);

    void setStructureDumping(bool enabled);

    int startFrame() const {
        return m_startFrame;
    }

    int endFrame() const {
        return m_endFrame;
    }

    int frameRate() const {
        return m_frameRate;
    }

private:
    QSize m_size;
    int m_frameRate = 30;
    int m_startFrame = 0;
    int m_endFrame = 0;
};

QT_END_NAMESPACE

#endif // QLOTTIEROOT_P_H
