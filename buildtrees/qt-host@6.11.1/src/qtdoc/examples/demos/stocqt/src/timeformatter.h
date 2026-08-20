// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TIMEFORMATTER_H
#define TIMEFORMATTER_H

#include <QtCore/qdatetime.h>
#include <QtGraphs/qvalue3daxisformatter.h>
#include <QtQml/qqmlregistration.h>

class TimeFormatter : public QValue3DAxisFormatter
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString selectionFormat READ selectionFormat WRITE setSelectionFormat NOTIFY
                   selectionFormatChanged FINAL)

    Q_PROPERTY(EpochFormat epochFormat READ epochFormat WRITE setEpochFormat NOTIFY epochFormatChanged FINAL)
    Q_PROPERTY(
        long long epochOffset READ epochOffset WRITE setEpochOffset NOTIFY epochOffsetChanged FINAL)
public:
    explicit TimeFormatter(QObject *parent = 0);
    virtual ~TimeFormatter();

    enum class EpochFormat { Ms, S, Day };
    Q_ENUM(EpochFormat)

    virtual QValue3DAxisFormatter *createNewInstance() const;
    virtual void populateCopy(QValue3DAxisFormatter &copy);
    virtual QString stringForValue(qreal value, const QString &format);

    QString selectionFormat() const;
    EpochFormat epochFormat() const;
    long long epochOffset() const;

public Q_SLOTS:
    void setSelectionFormat(const QString &format);
    void setEpochFormat(const EpochFormat format);
    void setEpochOffset(const long long origin);

Q_SIGNALS:
    void originDateChanged(const QDate &date);
    void selectionFormatChanged(const QString &format);
    void epochFormatChanged(const EpochFormat format);
    void epochOffsetChanged(const long long origin);

private:
    Q_DISABLE_COPY(TimeFormatter)

    QDateTime valueToDateTime(qint64 value) const;
    QString m_selectionFormat;
    EpochFormat m_epochFormat;
    long long m_epochOffset = 0;
};

#endif // TIMEFORMATTER_H
