// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QCanBusDevice>
#include <QVariant>

#include <memory>

int main(int /*argc*/, char ** /*argv*/)
{
    std::unique_ptr<QCanBusDevice> device = std::make_unique<QCanBusDevice>();

    //! [Filter Examples]
    QCanBusDevice::Filter filter;
    QList<QCanBusDevice::Filter> filterList;

    // filter all CAN bus packages with id 0x444 (base) or 0xXXXXX444 (extended)
    filter.frameId = 0x444u;
    filter.frameIdMask = 0x7FFu;
    filter.format = QCanBusDevice::Filter::MatchBaseAndExtendedFormat;
    filter.type = QCanBusFrame::InvalidFrame;
    filterList.append(filter);

    // filter all DataFrames with extended CAN bus package format
    filter.frameId = 0x0;
    filter.frameIdMask = 0x0;
    filter.format = QCanBusDevice::Filter::MatchExtendedFormat;
    filter.type = QCanBusFrame::DataFrame;
    filterList.append(filter);

    // apply filter
    device->setConfigurationParameter(QCanBusDevice::RawFilterKey, QVariant::fromValue(filterList));
    //! [Filter Examples]

    //! [SocketCan Filter Example]
    QList<QCanBusDevice::Filter> list;
    QCanBusDevice::Filter f;

    // only accept odd numbered frame id of type remote request
    // frame can utilize extended or base format
    f.frameId = 0x1;
    f.frameIdMask = 0x1;
    f.format = QCanBusDevice::Filter::MatchBaseAndExtendedFormat;
    f.type = QCanBusFrame::RemoteRequestFrame;
    list.append(f);

    device->setConfigurationParameter(QCanBusDevice::RawFilterKey, QVariant::fromValue(list));
    device->setConfigurationParameter(QCanBusDevice::ErrorFilterKey,
                                      QVariant::fromValue(QCanBusFrame::FrameErrors(QCanBusFrame::AnyError)));
    //! [SocketCan Filter Example]

    Q_UNUSED(filter);
    return 0;
}

