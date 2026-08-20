// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "registermodel.h"
#include "ui_mainwindow.h"

#include <QtWidgets>
#include <QtSerialBus>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusbar->setSizeGripEnabled(false);

    ui->addressPort->setToolTip(tr("Enter <ip address>:<port> pair."));

    setupConnections();
    setupClientContainer();
    setupServerContainer();
}

MainWindow::~MainWindow()
{
    m_client.disconnectDevice();
    m_server.disconnectDevice();

    delete ui;
}

void MainWindow::setupConnections()
{
    // actions
    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::onConnectButtonClicked);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::onConnectButtonClicked);
    connect(ui->actionExit, &QAction::triggered, this, &QMainWindow::close);

    // buttons
    connect(ui->readButton, &QPushButton::clicked, this, &MainWindow::onReadButtonClicked);
    connect(ui->writeButton, &QPushButton::clicked, this, &MainWindow::onWriteButtonClicked);
    connect(ui->connectButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);

    // client
    connect(&m_client, &QModbusServer::stateChanged, this, &MainWindow::onStateChanged);
    connect(&m_client, &QModbusServer::errorOccurred, this, [this](QModbusDevice::Error) {
        statusBar()->showMessage(m_client.errorString(), 5000);
    });

    // server
    connect(&m_server, &QModbusServer::dataWritten, this, &MainWindow::updateWidgets);
    connect(&m_server, &QModbusServer::errorOccurred, this, [this](QModbusDevice::Error) {
        statusBar()->showMessage(m_server.errorString(), 5000);
    });
}

void MainWindow::onConnectButtonClicked()
{
    bool intendToConnect = (m_server.state() == QModbusDevice::UnconnectedState
        && m_client.state() == QModbusDevice::UnconnectedState);

    if (intendToConnect) {
        const QUrl url = QUrl::fromUserInput(ui->addressPort->text());
        m_server.setConnectionParameter(QModbusDevice::NetworkPortParameter, url.port());
        m_server.setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());
        m_server.setServerAddress(ui->serverAddress->text().toInt());

        if (m_server.connectDevice()) {
            m_client.setConnectionParameter(QModbusDevice::NetworkPortParameter, url.port());
            m_client.setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());

            if (m_client.connectDevice()) {
                ui->actionConnect->setEnabled(false);
                ui->actionDisconnect->setEnabled(true);
            } else {
                statusBar()->showMessage(tr("Client connect failed: ") + m_client.errorString(), 5000);
            }
        } else {
            statusBar()->showMessage(tr("Server connect failed: ") + m_server.errorString(), 5000);
        }
    } else {
        m_client.disconnectDevice();
        m_server.disconnectDevice();
        ui->actionConnect->setEnabled(true);
        ui->actionDisconnect->setEnabled(false);
    }
}

void MainWindow::onStateChanged(int state)
{
    ui->serverAddress->setEnabled(state == QModbusDevice::UnconnectedState);
    ui->addressPort->setEnabled(state == QModbusDevice::UnconnectedState);
    ui->actionConnect->setEnabled(state == QModbusDevice::UnconnectedState);
    ui->actionDisconnect->setEnabled(state == QModbusDevice::ConnectedState);

    if (state == QModbusDevice::UnconnectedState)
        ui->connectButton->setText(tr("Connect"));
    else if (state == QModbusDevice::ConnectedState)
        ui->connectButton->setText(tr("Disconnect"));
}

// -- client

void MainWindow::onReadReady()
{
    auto reply = qobject_cast<QModbusReply*>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        for (qsizetype i = 0, total = unit.valueCount(); i < total; ++i) {
            m_model->setData(m_model->index(unit.startAddress() + i, 1),
                QString::number(unit.value(i), 16), Qt::EditRole);
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

void MainWindow::onReadButtonClicked()
{
    QModbusRequest readRequest {
        QModbusPdu::FunctionCode(ModbusClient::CustomRead),
        quint16(ui->startAddress->value()),
        qMin(ui->numberOfRegisters->currentText().toUShort(),
            quint16(10 - ui->startAddress->value())) // do not go beyond 10 entries
    };

    if (auto* reply = m_client.sendRawRequest(readRequest, ui->serverAddress->value())) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &MainWindow::onReadReady);
    } else {
        statusBar()->showMessage(tr("Read error: ") + m_client.errorString(), 5000);
    }
}

void MainWindow::onWriteButtonClicked()
{
//! [generate_modbus_request]
    QModbusDataUnit unit {
        QModbusDataUnit::HoldingRegisters,
        ui->startAddress->value(),
        qMin(ui->numberOfRegisters->currentText().toUShort(),
            quint16(10 - ui->startAddress->value())) // do not go beyond 10 entries
    };

    for (qsizetype i = 0, total = unit.valueCount(); i < total; ++i)
        unit.setValue(i, m_model->m_registers[i + unit.startAddress()]);

    const quint8 byteCount = quint8(unit.valueCount() * 2);
    QModbusRequest writeRequest {
        QModbusPdu::FunctionCode(ModbusClient::CustomWrite),
        quint16(unit.startAddress()),
        quint16(unit.valueCount()), byteCount, unit.values()
    };
//! [generate_modbus_request]

//! [send_custom_command]
    if (auto *reply = m_client.sendRawRequest(writeRequest, ui->serverAddress->value())) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                if (reply->error() == QModbusDevice::ProtocolError) {
                    statusBar()->showMessage(tr("Write response error: %1 (Modbus exception: 0x%2)")
                        .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
                        5000);
                } else if (reply->error() != QModbusDevice::NoError) {
                    statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
                        arg(reply->errorString()).arg(reply->error(), -1, 16), 5000);
                }

                reply->deleteLater();
                }
            );
        }
    } else {
        statusBar()->showMessage(tr("Write error: ") + m_client.errorString(), 5000);
    }
//! [send_custom_command]
}

void MainWindow::setupClientContainer()
{
    m_model = new RegisterModel(this);
    m_model->setStartAddress(ui->startAddress->value());
    m_model->setNumberOfValues(ui->numberOfRegisters->currentText());

    ui->registersTable->setModel(m_model);
    connect(m_model, &RegisterModel::updateViewport, ui->registersTable->viewport(),
        QOverload<>::of(&QWidget::update));

    auto model = new QStandardItemModel(10, 1, this);
    for (int i = 0; i < 10; ++i)
        model->setItem(i, new QStandardItem(QStringLiteral("%1").arg(i + 1)));
    ui->numberOfRegisters->setModel(model);
    ui->numberOfRegisters->setCurrentText("10");
    connect(ui->numberOfRegisters, &QComboBox::currentTextChanged,
        m_model, &RegisterModel::setNumberOfValues);

    connect(ui->startAddress, &QSpinBox::valueChanged, m_model, &RegisterModel::setStartAddress);
    connect(ui->startAddress, &QSpinBox::valueChanged, this, [this, model](int i) {
        int lastPossibleIndex = 0;
        const int currentIndex = ui->numberOfRegisters->currentIndex();
        for (int ii = 0; ii < 10; ++ii) {
            if (ii < (10 - i)) {
                lastPossibleIndex = ii;
                model->item(ii)->setEnabled(true);
            } else {
                model->item(ii)->setEnabled(false);
            }
        }
        if (currentIndex > lastPossibleIndex)
            ui->numberOfRegisters->setCurrentIndex(lastPossibleIndex);
        }
    );
}

// -- server

void MainWindow::setRegister(const QString &value)
{
    const QString objectName = QObject::sender()->objectName();
    if (m_registers.contains(objectName)) {
        const quint16 id = quint16(QObject::sender()->property("ID").toUInt());

        bool ok = false;
        ok = m_server.setData(QModbusDataUnit::HoldingRegisters, id, value.toUShort(&ok, 16));
        if (!ok)
            statusBar()->showMessage(tr("Could not set register: ") + m_server.errorString(), 5000);
    }
}

void MainWindow::updateWidgets(QModbusDataUnit::RegisterType table, int address, int size)
{
    if (table != QModbusDataUnit::HoldingRegisters)
        return;

    QString text;
    quint16 value;
    for (int i = 0; i < size; ++i) {
        m_server.data(table, quint16(address + i), &value);
        m_registers.value(QStringLiteral("register_%1").arg(address + i))->setText(text.setNum(value, 16));
    }
}

void MainWindow::setupServerContainer()
{
    QRegularExpression regexp(QStringLiteral("register_(?<ID>\\d+)"));
    const QList<QLineEdit *> qle = ui->server->findChildren<QLineEdit *>(regexp);
    for (QLineEdit *lineEdit : qle) {
        const quint16 id = regexp.match(lineEdit->objectName()).captured(QStringLiteral("ID")).toInt();

        lineEdit->setProperty("ID", id);
        m_registers.insert(lineEdit->objectName(), lineEdit);
        lineEdit->setValidator(new QRegularExpressionValidator(
            QRegularExpression(QStringLiteral("[0-9a-f]{0,4}"), QRegularExpression::CaseInsensitiveOption),
            this)
        );

        bool ok;
        m_server.setData(QModbusDataUnit::HoldingRegisters, id, lineEdit->text().toUShort(&ok, 16));
        connect(lineEdit, &QLineEdit::textChanged, this, &MainWindow::setRegister);
    }
}
