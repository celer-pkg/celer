// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "writeregistermodel.h"

#include <QModbusTcpClient>
#if QT_CONFIG(modbus_serialport)
#    include <QModbusRtuSerialClient>
#    include <QSerialPortInfo>
#endif
#include <QStandardItemModel>
#include <QStatusBar>
#include <QUrl>

using namespace Qt::StringLiterals;

enum ModbusConnection {
    Serial,
    Tcp
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_settingsDialog = new SettingsDialog(this);

    initActions();

    writeModel = new WriteRegisterModel(this);
    writeModel->setStartAddress(ui->writeAddress->value());
    writeModel->setNumberOfValues(ui->writeSize->currentText());

    ui->writeValueTable->setModel(writeModel);
    ui->writeValueTable->hideColumn(2);
    connect(writeModel, &WriteRegisterModel::updateViewport,
            ui->writeValueTable->viewport(), QOverload<>::of(&QWidget::update));

    ui->writeTable->addItem(tr("Coils"), QModbusDataUnit::Coils);
    ui->writeTable->addItem(tr("Discrete Inputs"), QModbusDataUnit::DiscreteInputs);
    ui->writeTable->addItem(tr("Input Registers"), QModbusDataUnit::InputRegisters);
    ui->writeTable->addItem(tr("Holding Registers"), QModbusDataUnit::HoldingRegisters);

    ui->portEdit->setToolTip(tr("For serial connection enter COM port name\n"
                                "(eg. COM1, ttyS0, etc).\n"
                                "For TCP connection enter\n<ip address>:<port> pair."));

#if QT_CONFIG(modbus_serialport)
    ui->connectType->setCurrentIndex(0);
    onConnectTypeChanged(0);
#else
    // lock out the serial port option
    ui->connectType->setCurrentIndex(1);
    onConnectTypeChanged(1);
    ui->connectType->setEnabled(false);
#endif

    auto *model = new QStandardItemModel(0, 1, this);
    for (int i = 1; i <= 10; ++i)
        model->appendRow(new QStandardItem(QString::number(i)));
    ui->writeSize->setModel(model);
    ui->writeSize->setCurrentText(u"10"_s);
    connect(ui->writeSize, &QComboBox::currentTextChanged,
            writeModel, &WriteRegisterModel::setNumberOfValues);

    connect(ui->writeAddress, &QSpinBox::valueChanged, writeModel, &WriteRegisterModel::setStartAddress);
    connect(ui->writeAddress, &QSpinBox::valueChanged, this, [this, model](int i) {
        int lastPossibleIndex = 0;
        const int currentIndex = ui->writeSize->currentIndex();
        for (int ii = 0; ii < 10; ++ii) {
            if (ii < (10 - i)) {
                lastPossibleIndex = ii;
                model->item(ii)->setEnabled(true);
            } else {
                model->item(ii)->setEnabled(false);
            }
        }
        if (currentIndex > lastPossibleIndex)
            ui->writeSize->setCurrentIndex(lastPossibleIndex);
    });
}

MainWindow::~MainWindow()
{
    if (modbusDevice)
        modbusDevice->disconnectDevice();
    delete modbusDevice;

    delete ui;
}

void MainWindow::initActions()
{
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionExit->setEnabled(true);
    ui->actionOptions->setEnabled(true);

    connect(ui->connectButton, &QPushButton::clicked,
            this, &MainWindow::onConnectButtonClicked);
    connect(ui->actionConnect, &QAction::triggered,
            this, &MainWindow::onConnectButtonClicked);
    connect(ui->actionDisconnect, &QAction::triggered,
            this, &MainWindow::onConnectButtonClicked);
    connect(ui->readButton, &QPushButton::clicked,
            this, &MainWindow::onReadButtonClicked);
    connect(ui->writeButton, &QPushButton::clicked,
            this, &MainWindow::onWriteButtonClicked);
    connect(ui->readWriteButton, &QPushButton::clicked,
            this, &MainWindow::onReadWriteButtonClicked);
    connect(ui->connectType, &QComboBox::currentIndexChanged,
            this, &MainWindow::onConnectTypeChanged);
    connect(ui->writeTable, &QComboBox::currentIndexChanged,
            this, &MainWindow::onWriteTableChanged);

    connect(ui->actionExit, &QAction::triggered, this, &QMainWindow::close);
    connect(ui->actionOptions, &QAction::triggered, m_settingsDialog, &QDialog::show);
}

void MainWindow::onConnectTypeChanged(int index)
{
    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        delete modbusDevice;
        modbusDevice = nullptr;
    }

//! [create_client_0]
    auto type = static_cast<ModbusConnection>(index);
    if (type == Serial) {
#if QT_CONFIG(modbus_serialport)
        modbusDevice = new QModbusRtuSerialClient(this);
        // Try to fill in the first available serial port name if the line edit
        // is empty, or contains a url (assume that ':' is only a part of url).
        const auto ports = QSerialPortInfo::availablePorts();
        const auto currentText = ui->portEdit->text();
        if (!ports.isEmpty() && (currentText.isEmpty() || currentText.contains(u':')))
            ui->portEdit->setText(ports.front().portName());
#endif
    } else if (type == Tcp) {
        modbusDevice = new QModbusTcpClient(this);
        const QUrl currentUrl = QUrl::fromUserInput(ui->portEdit->text());
        // Check if we already have <ip address>:<port>
        if (currentUrl.port() <= 0)
            ui->portEdit->setText(QLatin1String("127.0.0.1:50200"));
    }
//! [create_client_0]

    connect(modbusDevice, &QModbusClient::errorOccurred, this, [this](QModbusDevice::Error) {
        statusBar()->showMessage(modbusDevice->errorString(), 5000);
    });

    if (!modbusDevice) {
        ui->connectButton->setDisabled(true);
        statusBar()->showMessage(tr("Could not create Modbus client."), 5000);
    } else {
        connect(modbusDevice, &QModbusClient::stateChanged,
                this, &MainWindow::onModbusStateChanged);
    }
}

void MainWindow::onConnectButtonClicked()
{
    if (!modbusDevice)
        return;

    statusBar()->clearMessage();
    if (modbusDevice->state() != QModbusDevice::ConnectedState) {
//! [create_client_1]
        const auto settings = m_settingsDialog->settings();
        if (static_cast<ModbusConnection>(ui->connectType->currentIndex()) == Serial) {
            modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
                ui->portEdit->text());
#if QT_CONFIG(modbus_serialport)
            modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter,
                settings.parity);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
                settings.baud);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
                settings.dataBits);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
                settings.stopBits);
#endif
        } else {
            const QUrl url = QUrl::fromUserInput(ui->portEdit->text());
            modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, url.port());
            modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());
        }
        modbusDevice->setTimeout(settings.responseTime);
        modbusDevice->setNumberOfRetries(settings.numberOfRetries);
//! [create_client_1]
        if (!modbusDevice->connectDevice()) {
            statusBar()->showMessage(tr("Connect failed: %1").arg(modbusDevice->errorString()), 5000);
        } else {
            ui->actionConnect->setEnabled(false);
            ui->actionDisconnect->setEnabled(true);
        }
    } else {
        modbusDevice->disconnectDevice();
        ui->actionConnect->setEnabled(true);
        ui->actionDisconnect->setEnabled(false);
    }
}

void MainWindow::onModbusStateChanged(int state)
{
    bool connected = (state != QModbusDevice::UnconnectedState);
    ui->actionConnect->setEnabled(!connected);
    ui->actionDisconnect->setEnabled(connected);

    if (state == QModbusDevice::UnconnectedState)
        ui->connectButton->setText(tr("Connect"));
    else if (state == QModbusDevice::ConnectedState)
        ui->connectButton->setText(tr("Disconnect"));

    ui->connectType->setEnabled(!connected);
    ui->portEdit->setEnabled(!connected);
    ui->serverEdit->setEnabled(!connected);
}

void MainWindow::onReadButtonClicked()
{
    if (!modbusDevice)
        return;
    ui->readValue->clear();
    statusBar()->clearMessage();

//! [read_data_1]
    if (auto *reply = modbusDevice->sendReadRequest(readRequest(), ui->serverEdit->value())) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &MainWindow::onReadReady);
        else
            delete reply; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: %1").arg(modbusDevice->errorString()), 5000);
    }
//! [read_data_1]
}

//! [read_data_2]
void MainWindow::onReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        for (qsizetype i = 0, total = unit.valueCount(); i < total; ++i) {
            const QString entry = tr("Address: %1, Value: %2").arg(unit.startAddress() + i)
                                     .arg(QString::number(unit.value(i),
                                          unit.registerType() <= QModbusDataUnit::Coils ? 10 : 16));
            ui->readValue->addItem(entry);
        }
    } else if (reply->error() == QModbusDevice::ProtocolError) {
        statusBar()->showMessage(tr("Read response error: %1 (Modbus exception: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->rawResult().exceptionCode(), -1, 16), 5000);
    } else {
        statusBar()->showMessage(tr("Read response error: %1 (code: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->error(), -1, 16), 5000);
    }

    reply->deleteLater();
}
//! [read_data_2]

void MainWindow::onWriteButtonClicked()
{
    if (!modbusDevice)
        return;
    statusBar()->clearMessage();

//! [write_data_0]
    QModbusDataUnit writeUnit = writeRequest();
    QModbusDataUnit::RegisterType table = writeUnit.registerType();
    for (qsizetype i = 0, total = writeUnit.valueCount(); i < total; ++i) {
        const auto addr = i + writeUnit.startAddress();
        if (table == QModbusDataUnit::Coils)
            writeUnit.setValue(i, writeModel->m_coils[addr]);
        else
            writeUnit.setValue(i, writeModel->m_holdingRegisters[addr]);
    }

    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, ui->serverEdit->value())) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                const auto error = reply->error();
                if (error == QModbusDevice::ProtocolError) {
                    statusBar()->showMessage(tr("Write response error: %1 (Modbus exception: 0x%2)")
                        .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
                        5000);
                } else if (error != QModbusDevice::NoError) {
                    statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
                        arg(reply->errorString()).arg(error, -1, 16), 5000);
                }
                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        statusBar()->showMessage(tr("Write error: %1").arg(modbusDevice->errorString()), 5000);
    }
//! [write_data_0]
}

void MainWindow::onReadWriteButtonClicked()
{
    if (!modbusDevice)
        return;
    ui->readValue->clear();
    statusBar()->clearMessage();

    QModbusDataUnit writeUnit = writeRequest();
    QModbusDataUnit::RegisterType table = writeUnit.registerType();
    for (qsizetype i = 0, total = writeUnit.valueCount(); i < total; ++i) {
        const auto addr = i + writeUnit.startAddress();
        if (table == QModbusDataUnit::Coils)
            writeUnit.setValue(i, writeModel->m_coils[addr]);
        else
            writeUnit.setValue(i, writeModel->m_holdingRegisters[addr]);
    }

    if (auto *reply = modbusDevice->sendReadWriteRequest(readRequest(), writeUnit,
        ui->serverEdit->value())) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &MainWindow::onReadReady);
        else
            delete reply; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: %1").arg(modbusDevice->errorString()), 5000);
    }
}

void MainWindow::onWriteTableChanged(int index)
{
    const bool coilsOrHolding = index == 0 || index == 3;
    if (coilsOrHolding) {
        ui->writeValueTable->setColumnHidden(1, index != 0);
        ui->writeValueTable->setColumnHidden(2, index != 3);
        ui->writeValueTable->resizeColumnToContents(0);
    }

    ui->readWriteButton->setEnabled(index == 3);
    ui->writeButton->setEnabled(coilsOrHolding);
    ui->writeGroupBox->setEnabled(coilsOrHolding);
}

//! [read_data_0]
QModbusDataUnit MainWindow::readRequest() const
{
    const auto table = ui->writeTable->currentData().value<QModbusDataUnit::RegisterType>();

    int startAddress = ui->readAddress->value();
    Q_ASSERT(startAddress >= 0 && startAddress < 10);

    // do not go beyond 10 entries
    quint16 numberOfEntries = qMin(ui->readSize->currentText().toUShort(),
                                   quint16(10 - startAddress));
    return QModbusDataUnit(table, startAddress, numberOfEntries);
}
//! [read_data_0]

QModbusDataUnit MainWindow::writeRequest() const
{
    const auto table = ui->writeTable->currentData().value<QModbusDataUnit::RegisterType>();

    int startAddress = ui->writeAddress->value();
    Q_ASSERT(startAddress >= 0 && startAddress < 10);

    // do not go beyond 10 entries
    quint16 numberOfEntries = qMin(ui->writeSize->currentText().toUShort(),
                                   quint16(10 - startAddress));
    return QModbusDataUnit(table, startAddress, numberOfEntries);
}
