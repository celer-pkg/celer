// Copyright (C) 2021 Andre Hartmann <aha_1980@gmx.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CANBUSDEVICEINFOBOX_H
#define CANBUSDEVICEINFOBOX_H

#include <QGroupBox>

QT_BEGIN_NAMESPACE

class QCanBusDeviceInfo;

namespace Ui {
class CanBusDeviceInfoBox;
}

QT_END_NAMESPACE

class CanBusDeviceInfoBox : public QGroupBox
{
    Q_OBJECT

public:
    explicit CanBusDeviceInfoBox(QWidget *parent = nullptr);
    ~CanBusDeviceInfoBox();
    void clear();
    void setDeviceInfo(const QCanBusDeviceInfo &info);

private:
    Ui::CanBusDeviceInfoBox *m_ui = nullptr;
};

#endif // CANBUSDEVICEINFOBOX_H
