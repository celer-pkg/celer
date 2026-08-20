// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIESHAPE_P_H
#define QLOTTIESHAPE_P_H

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

#include <QPainterPath>

#include <QtLottie/private/qlottiebase_p.h>
#include <QtLottie/private/qlottieproperty_p.h>

QT_BEGIN_NAMESPACE

class QLottieFill;
class QLottieStroke;
class QLottieTrimPath;

class Q_LOTTIE_EXPORT QLottieShape : public QLottieBase
{
public:
    QLottieShape();
    ~QLottieShape();
    explicit QLottieShape(const QLottieShape &other);

    QLottieBase *clone() const override;

    static QLottieShape *construct(QJsonObject definition, QLottieBase *parent = nullptr);

    virtual const QPainterPath &path() const;
    virtual bool acceptsTrim() const;
    virtual void applyTrim(const QLottieTrimPath& trimmer);
    const QLottieTrimPath *currentTrim() const;

    int direction() const;
    bool hasReversedDirection() const { return m_direction == 3; }

protected:
    enum class OwnsAppliedTrim: bool { No = false, Yes = true};
    struct AppliedTrimPtr
    {
        Q_DISABLE_COPY_MOVE(AppliedTrimPtr)
    public:
        AppliedTrimPtr();
        ~AppliedTrimPtr();

        void reset(QLottieTrimPath *appliedTrim, OwnsAppliedTrim owns);
        QLottieTrimPath *data() const;
        bool owns() const;
    private:
        QTaggedPointer<QLottieTrimPath, OwnsAppliedTrim> m_appliedTrim;
    };

    QPainterPath m_path;
    AppliedTrimPtr m_appliedTrim;
    int m_direction = 0;
};

QT_END_NAMESPACE

#endif // QLOTTIESHAPE_P_H
