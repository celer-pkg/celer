// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CLIENTCONSOLE_H
#define CLIENTCONSOLE_H

#include <QWidget>

namespace qt::examples::sensors {
class Coordinates;
class Temperature;
class WarningNotification;
} // namespace qt::examples::sensors

QT_BEGIN_NAMESPACE
namespace Ui {
class ClientConsole;
}
QT_END_NAMESPACE

class ClientConsole : public QWidget
{
    Q_OBJECT

public:
    ClientConsole(QWidget *parent = nullptr);
    ~ClientConsole();

    void onCoordinatesUpdated(const qt::examples::sensors::Coordinates &coord);
    void onTemperatureUpdated(const qt::examples::sensors::Temperature &temp);
    void onWarning(const qt::examples::sensors::WarningNotification &warn);

private:
    Ui::ClientConsole *ui;
};

#endif // CLIENTCONSOLE_H
