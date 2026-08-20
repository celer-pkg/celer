// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPROFILERADAPTER_H
#define QQUICK3DPROFILERADAPTER_H

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

#include <QtQml/private/qqmlabstractprofileradapter_p.h>
#include <QtQuick3DUtils/private/qquick3dprofiler_p.h>

QT_BEGIN_NAMESPACE

class QQuick3DProfilerAdapter : public QQmlAbstractProfilerAdapter {
    Q_OBJECT
public:
    QQuick3DProfilerAdapter(QObject *parent = 0);
    ~QQuick3DProfilerAdapter();
    qint64 sendMessages(qint64 until, QList<QByteArray> &messages) override;
    void receiveData(const QVector<QQuick3DProfilerData> &new_data, const QHash<int, QByteArray> &eventData);

private:
    int next;
    QVector<QQuick3DProfilerData> m_data;
    QHash<int, QByteArray> m_eventData;
};

QT_END_NAMESPACE

#endif // QQUICK3DPROFILERADAPTER_H
