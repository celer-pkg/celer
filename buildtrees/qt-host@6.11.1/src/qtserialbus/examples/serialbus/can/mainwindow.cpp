// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "canbusdeviceinfodialog.h"
#include "connectdialog.h"
#include "receivedframesmodel.h"

#include <QCanBus>
#include <QCanBusFrame>
#include <QCloseEvent>
#include <QDateTime>
#include <QDesktopServices>
#include <QLabel>
#include <QTimer>

using namespace Qt::StringLiterals;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_busStatusTimer(new QTimer(this))
{
    m_ui->setupUi(this);

    m_connectDialog = new ConnectDialog;

    m_status = new QLabel;
    m_ui->statusBar->addPermanentWidget(m_status);

    m_written = new QLabel;
    m_ui->statusBar->addWidget(m_written);

    m_received = new QLabel;
    m_ui->statusBar->addWidget(m_received);

    m_model = new ReceivedFramesModel(this);
    m_model->setQueueLimit(1000);
    m_ui->receivedFramesView->setModel(m_model);

    initActionsConnections();
    QTimer::singleShot(50, m_connectDialog, &ConnectDialog::show);

    connect(m_busStatusTimer, &QTimer::timeout, this, &MainWindow::busStatus);
    m_appendTimer = new QTimer(this);
    connect(m_appendTimer, &QTimer::timeout, this, &MainWindow::onAppendFramesTimeout);
    m_appendTimer->start(350);
}

MainWindow::~MainWindow()
{
    delete m_connectDialog;
    delete m_ui;
}

void MainWindow::initActionsConnections()
{
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionDeviceInformation->setEnabled(false);
    m_ui->sendFrameBox->setEnabled(false);

    connect(m_ui->sendFrameBox, &SendFrameBox::sendFrame, this, &MainWindow::sendFrame);
    connect(m_ui->actionConnect, &QAction::triggered, this, [this]() {
        if (m_canDevice)
            m_canDevice.release()->deleteLater();
        m_connectDialog->show();
    });
    connect(m_connectDialog, &QDialog::accepted, this, &MainWindow::connectDevice);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::disconnectDevice);
    connect(m_ui->actionResetController, &QAction::triggered, this, [this]() {
        m_canDevice->resetController();
    });
    connect(m_ui->actionQuit, &QAction::triggered, this, &QWidget::close);
    connect(m_ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(m_ui->actionClearLog, &QAction::triggered, m_model, &ReceivedFramesModel::clear);
    connect(m_ui->actionPluginDocumentation, &QAction::triggered, this, []() {
        QDesktopServices::openUrl(QUrl("http://doc.qt.io/qt-6/qtcanbus-backends.html#can-bus-plugins"));
    });
    connect(m_ui->actionDeviceInformation, &QAction::triggered, this, [this]() {
        auto info = m_canDevice->deviceInfo();
        CanBusDeviceInfoDialog dialog(info, this);
        dialog.exec();
    });
}

void MainWindow::processErrors(QCanBusDevice::CanBusError error) const
{
    switch (error) {
    case QCanBusDevice::ReadError:
    case QCanBusDevice::WriteError:
    case QCanBusDevice::ConnectionError:
    case QCanBusDevice::ConfigurationError:
    case QCanBusDevice::UnknownError:
        m_status->setText(m_canDevice->errorString());
        break;
    default:
        break;
    }
}

void MainWindow::connectDevice()
{
    const ConnectDialog::Settings p = m_connectDialog->settings();

    if (p.useModelRingBuffer)
        m_model->setQueueLimit(p.modelRingBufferSize);
    else
        m_model->setQueueLimit(0);

//! [create_can_device_0]
    QString errorString;
    m_canDevice.reset(QCanBus::instance()->createDevice(p.pluginName, p.deviceInterfaceName,
                                                        &errorString));
//! [create_can_device_0]
    if (!m_canDevice) {
        m_status->setText(tr("Error creating device '%1', reason: '%2'")
                          .arg(p.pluginName, errorString));
        return;
    }

    m_numberFramesWritten = 0;

//! [create_can_device_1]
    connect(m_canDevice.get(), &QCanBusDevice::errorOccurred,
            this, &MainWindow::processErrors);
    connect(m_canDevice.get(), &QCanBusDevice::framesReceived,
            this, &MainWindow::processReceivedFrames);
    connect(m_canDevice.get(), &QCanBusDevice::framesWritten,
            this, &MainWindow::processFramesWritten);
//! [create_can_device_1]

    if (p.useConfigurationEnabled) {
        for (const ConnectDialog::ConfigurationItem &item : p.configurations)
            m_canDevice->setConfigurationParameter(item.first, item.second);
    }

    if (!m_canDevice->connectDevice()) {
        m_status->setText(tr("Connection error: %1").arg(m_canDevice->errorString()));

        m_canDevice.reset();
    } else {
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->actionDeviceInformation->setEnabled(true);

        m_ui->sendFrameBox->setEnabled(true);

        const QVariant bitRate = m_canDevice->configurationParameter(QCanBusDevice::BitRateKey);
        if (bitRate.isValid()) {
            const bool isCanFd =
                    m_canDevice->configurationParameter(QCanBusDevice::CanFdKey).toBool();
            const QVariant dataBitRate =
                    m_canDevice->configurationParameter(QCanBusDevice::DataBitRateKey);
            if (isCanFd && dataBitRate.isValid()) {
                m_status->setText(tr("Plugin: %1, connected to %2 at %3 / %4 kBit/s")
                                  .arg(p.pluginName, p.deviceInterfaceName)
                                  .arg(bitRate.toInt() / 1000).arg(dataBitRate.toInt() / 1000));
            } else {
                m_status->setText(tr("Plugin: %1, connected to %2 at %3 kBit/s")
                                  .arg(p.pluginName, p.deviceInterfaceName)
                                  .arg(bitRate.toInt() / 1000));
            }
        } else {
            m_status->setText(tr("Plugin: %1, connected to %2")
                    .arg(p.pluginName, p.deviceInterfaceName));
        }

        if (m_canDevice->hasBusStatus())
            m_busStatusTimer->start(2000);
        else
            m_ui->busStatus->setText(tr("No CAN bus status available."));
    }
}

void MainWindow::busStatus()
{
    if (!m_canDevice || !m_canDevice->hasBusStatus()) {
        m_ui->busStatus->setText(tr("No CAN bus status available."));
        m_busStatusTimer->stop();
        return;
    }

    switch (m_canDevice->busStatus()) {
    case QCanBusDevice::CanBusStatus::Good:
        m_ui->busStatus->setText("CAN bus status: Good.");
        break;
    case QCanBusDevice::CanBusStatus::Warning:
        m_ui->busStatus->setText("CAN bus status: Warning.");
        break;
    case QCanBusDevice::CanBusStatus::Error:
        m_ui->busStatus->setText("CAN bus status: Error.");
        break;
    case QCanBusDevice::CanBusStatus::BusOff:
        m_ui->busStatus->setText("CAN bus status: Bus Off.");
        break;
    default:
        m_ui->busStatus->setText("CAN bus status: Unknown.");
        break;
    }
}

void MainWindow::disconnectDevice()
{
    if (!m_canDevice)
        return;

    m_busStatusTimer->stop();

    m_canDevice->disconnectDevice();

    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionDeviceInformation->setEnabled(false);

    m_ui->sendFrameBox->setEnabled(false);

    m_status->setText(tr("Disconnected"));
}

void MainWindow::processFramesWritten(qint64 count)
{
    m_numberFramesWritten += count;
    m_written->setText(tr("%1 frames written").arg(m_numberFramesWritten));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_connectDialog->close();
    event->accept();
}

static QString frameFlags(const QCanBusFrame &frame)
{
    QString result = u" --- "_s;

    if (frame.hasBitrateSwitch())
        result[1] = u'B';
    if (frame.hasErrorStateIndicator())
        result[2] = u'E';
    if (frame.hasLocalEcho())
        result[3] = u'L';

    return result;
}

void MainWindow::processReceivedFrames()
{
    if (!m_canDevice)
        return;

//! [receive_can_frame]
    while (m_canDevice->framesAvailable()) {
        m_numberFramesReceived++;
        const QCanBusFrame frame = m_canDevice->readFrame();

        QString data;
        if (frame.frameType() == QCanBusFrame::ErrorFrame)
            data = m_canDevice->interpretErrorFrame(frame);
        else
            data = QString::fromLatin1(frame.payload().toHex(' ').toUpper());

        const QString time = QString::fromLatin1("%1.%2  ")
                .arg(frame.timeStamp().seconds(), 10, 10, ' '_L1)
                .arg(frame.timeStamp().microSeconds() / 100, 4, 10, '0'_L1);

        const QString flags = frameFlags(frame);

        const QString id = QString::number(frame.frameId(), 16).toUpper();
        const QString dlc = QString::number(frame.payload().size());

        m_model->appendFrame(QStringList({QString::number(m_numberFramesReceived),
                                          time, flags, id, dlc, data}));
    }
//! [receive_can_frame]
}

void MainWindow::sendFrame(const QCanBusFrame &frame) const
{
    if (!m_canDevice)
        return;

//! [send_can_frame]
    m_canDevice->writeFrame(frame);
//! [send_can_frame]
}

void MainWindow::onAppendFramesTimeout()
{
    if (!m_canDevice)
        return;

    if (m_model->needUpdate()) {
        m_model->update();
        if (m_connectDialog->settings().useAutoscroll)
            m_ui->receivedFramesView->scrollToBottom();
        m_received->setText(tr("%1 frames received").arg(m_numberFramesReceived));
    }
}
