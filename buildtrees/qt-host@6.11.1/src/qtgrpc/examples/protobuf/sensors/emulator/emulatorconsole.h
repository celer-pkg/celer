// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef EMULATORCONSOLE_H
#define EMULATORCONSOLE_H

#include <QWidget>

namespace qt::examples::sensors {
class Coordinates;
class Temperature;
class WarningNotification;
} // namespace qt::examples::sensors

QT_BEGIN_NAMESPACE
namespace Ui {
class EmulatorConsole;
}
QT_END_NAMESPACE

class EmulatorConsole : public QWidget
{
    Q_OBJECT

public:
    explicit EmulatorConsole(QWidget *parent = nullptr);
    ~EmulatorConsole();

signals:
    void coordinatesUpdated(const qt::examples::sensors::Coordinates &);
    void temperatureUpdated(const qt::examples::sensors::Temperature &);
    void warning(const qt::examples::sensors::WarningNotification &);

private:
    Ui::EmulatorConsole *ui;
};
#endif // EMULATORCONSOLE_H
