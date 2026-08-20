// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCONNMANINFORMATIONBACKEND_H
#define QCONNMANINFORMATIONBACKEND_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtNetwork/private/qnetworkinformation_p.h>
#include "qconnmanservice.h"

QT_BEGIN_NAMESPACE

class QConnManNetworkInformationBackend : public QNetworkInformationBackend
{
    Q_OBJECT
public:
    QConnManNetworkInformationBackend();
    ~QConnManNetworkInformationBackend() = default;

    QString name() const override;
    QNetworkInformation::Features featuresSupported() const override
    {
        if (!isValid())
            return {};
        return featuresSupportedStatic();
    }

    static QNetworkInformation::Features featuresSupportedStatic()
    {
        using Feature = QNetworkInformation::Feature;
        return QNetworkInformation::Features(Feature::Reachability | Feature::TransportMedium);
    }

    bool isValid() const { return iface.isValid(); }

    void onStateChanged(const QString &state);
    void onTypeChanged(const QString &type);

private:
    Q_DISABLE_COPY_MOVE(QConnManNetworkInformationBackend)

    QConnManInterface iface;
};

QT_END_NAMESPACE

#endif
