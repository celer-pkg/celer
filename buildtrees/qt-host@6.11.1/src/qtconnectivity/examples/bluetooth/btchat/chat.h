// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QDialog>

#include <QBluetoothHostInfo>

QT_BEGIN_NAMESPACE
namespace Ui {
    class Chat;
}
QT_END_NAMESPACE

class ChatServer;
class ChatClient;

//! [declaration]
class Chat : public QDialog
{
    Q_OBJECT

public:
    explicit Chat(QWidget *parent = nullptr);
    ~Chat();

signals:
    void sendMessage(const QString &message);

private slots:
    void connectClicked();
    void sendClicked();

    void showMessage(const QString &sender, const QString &message);

    void clientConnected(const QString &name);
    void clientDisconnected(const QString &name);
    void clientDisconnected();
    void connected(const QString &name);
    void reactOnSocketError(const QString &error);

    void newAdapterSelected();

    void initBluetooth();

    void updateIcons(Qt::ColorScheme scheme);

private:
    int adapterFromUserSelection() const;
    int currentAdapterIndex = 0;
    Ui::Chat *ui;

    ChatServer *server = nullptr;
    QList<ChatClient *> clients;
    QList<QBluetoothHostInfo> localAdapters;

    QString localName;
};
//! [declaration]
