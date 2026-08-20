// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef NDEFMESSAGEMODEL_H
#define NDEFMESSAGEMODEL_H

#include <QAbstractListModel>

#include <QNdefMessage>
#include <QQmlEngine>

class NdefMessageModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QNdefMessage message READ message WRITE setMessage NOTIFY messageChanged)

public:
    explicit NdefMessageModel(QObject *parent = nullptr);

    enum Role {
        RecordTypeRole,
        RecordTextRole,
    };

    enum RecordType {
        OtherRecord,
        TextRecord,
        UriRecord,
    };
    Q_ENUM(RecordType)

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;

    QNdefMessage message() const;
    void setMessage(const QNdefMessage &newMessage);

    Q_INVOKABLE void clearMessage();
    Q_INVOKABLE void addTextRecord(const QString &text);
    Q_INVOKABLE void addUriRecord(const QString &uri);
    Q_INVOKABLE void setTextData(int row, const QString &text);

Q_SIGNALS:
    void messageChanged();

private:
    QNdefMessage m_message;
};

#endif // NDEFMESSAGEMODEL_H
