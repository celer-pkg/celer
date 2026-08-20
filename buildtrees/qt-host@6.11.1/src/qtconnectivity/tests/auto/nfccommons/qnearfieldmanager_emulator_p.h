// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QNEARFIELDMANAGER_EMULATOR_H
#define QNEARFIELDMANAGER_EMULATOR_H

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

#include <QtNfc/private/qnearfieldmanager_p.h>
#include <QtNfc/private/qnearfieldtarget_p.h>
#include <QtNfc/qndeffilter.h>

#include <QtCore/QMetaMethod>
#include <QtCore/QObject>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class TagBase;
class QNearFieldManagerPrivateImpl : public QNearFieldManagerPrivate
{
    Q_OBJECT

public:
    QNearFieldManagerPrivateImpl();
    ~QNearFieldManagerPrivateImpl() override;

    bool isEnabled() const override;

    bool isSupported(QNearFieldTarget::AccessMethod accessMethod) const override;

    bool startTargetDetection(QNearFieldTarget::AccessMethod accessMethod) override;
    void stopTargetDetection(const QString &message) override;
    void setUserInformation(const QString &message) override;

    void reset();

signals:
    void userInformationChanged(const QString &message);

private slots:
    void tagActivated(TagBase *tag);
    void tagDeactivated(TagBase *tag);

private:
    QMap<TagBase *, QPointer<QNearFieldTargetPrivate> > targets;

};

QT_END_NAMESPACE

#endif // QNEARFIELDMANAGER_EMULATOR_H
