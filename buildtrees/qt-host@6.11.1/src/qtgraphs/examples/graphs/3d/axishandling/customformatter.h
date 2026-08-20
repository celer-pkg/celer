// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CUSTOMFORMATTER_H
#define CUSTOMFORMATTER_H

#include <QtCore/qdatetime.h>
#include <QtGraphs/qvalue3daxisformatter.h>
#include <QtQml/qqmlregistration.h>

//! [2]
class CustomFormatter : public QValue3DAxisFormatter
{
    //! [2]
    Q_OBJECT
    QML_ELEMENT

    //! [1]
    Q_PROPERTY(QDate originDate READ originDate WRITE setOriginDate NOTIFY originDateChanged)
    //! [1]
    //! [3]
    Q_PROPERTY(QString selectionFormat READ selectionFormat WRITE setSelectionFormat NOTIFY
                   selectionFormatChanged)
    //! [3]
public:
    explicit CustomFormatter(QObject *parent = 0);
    ~CustomFormatter() override;

    //! [0]
    QValue3DAxisFormatter *createNewInstance() const override;
    void populateCopy(QValue3DAxisFormatter &copy) override;
    void recalculate() override;
    QString stringForValue(qreal value, const QString &format) override;
    //! [0]

    QDate originDate() const;
    QString selectionFormat() const;

public Q_SLOTS:
    void setOriginDate(const QDate &date);
    void setSelectionFormat(const QString &format);

Q_SIGNALS:
    void originDateChanged(const QDate &date);
    void selectionFormatChanged(const QString &format);

private:
    Q_DISABLE_COPY(CustomFormatter)

    QDateTime valueToDateTime(qreal value) const;

    QDate m_originDate;
    QString m_selectionFormat;
};

#endif
