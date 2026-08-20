// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QDATETIMEAXIS_P_H
#define QDATETIMEAXIS_P_H

#include <QtGraphs/QDateTimeAxis>
#include <private/qabstractaxis_p.h>

QT_BEGIN_NAMESPACE

class QDateTimeAxisPrivate : public QAbstractAxisPrivate
{
public:
    QDateTimeAxisPrivate();
    ~QDateTimeAxisPrivate() override;

protected:
    qreal m_min = QDateTime::fromMSecsSinceEpoch(0, QTimeZone::UTC).toMSecsSinceEpoch();
    qreal m_max = QDateTime::fromMSecsSinceEpoch(0, QTimeZone::UTC).addYears(10).toMSecsSinceEpoch();
    QTimeZone m_timeZone = QTimeZone::UTC;
    qreal m_tickInterval = 0.0;
    qsizetype m_subTickCount = 0;
    QString m_format = QStringLiteral("dd-MMMM-yy");
    qreal m_zoom = 1.0;
    qreal m_pan = 0.0;
    qreal m_visualMin;
    qreal m_visualMax;

public:
    void setMin(const QVariant &min) override;
    void setMax(const QVariant &max) override;
    void setRange(const QVariant &min, const QVariant &max) override;
    void setRange(qreal min, qreal max) override;
    qreal min() override { return m_min; }
    qreal max() override { return m_max; }
    void calculateVisualRange();

private:
    Q_DECLARE_PUBLIC(QDateTimeAxis)
};

QT_END_NAMESPACE

#endif // QDATETIMEAXIS_P_H
