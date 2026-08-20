// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "mainwindow.h"
#include "modbustcpclient.h"

#include <QLoggingCategory>
#include <QModbusPdu>
#include <QModbusRtuSerialClient>
#include <QSerialPortInfo>

#ifndef QT_STATIC
 QT_BEGIN_NAMESPACE
 Q_LOGGING_CATEGORY(QT_MODBUS, "qt.modbus")
 Q_LOGGING_CATEGORY(QT_MODBUS_LOW, "qt.modbus.lowlevel")
 QT_END_NAMESPACE
#endif

QT_USE_NAMESPACE

MainWindow *s_instance = nullptr;

static void HandlerFunction(QtMsgType, const QMessageLogContext &, const QString &msg)
{
    if (auto instance = MainWindow::instance())
        instance->appendToLog(msg);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_debugHandler(HandlerFunction)
{
    setupUi(this);
    s_instance = this;

    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports)
        serialPortCombo->addItem(info.portName(), false);
    serialPortCombo->insertSeparator(serialPortCombo->count());
    serialPortCombo->addItem(tr("Add port..."), true);
    serialPortCombo->setInsertPolicy(QComboBox::InsertAtTop);

    connect(tcpRadio, &QRadioButton::toggled, this, [this](bool toggled) {
        stackedWidget->setCurrentIndex(toggled);
    });
    connect(actionExit, &QAction::triggered, this, &QMainWindow::close);

    QLoggingCategory::setFilterRules(QStringLiteral("qt.modbus* = true"));
}

MainWindow::~MainWindow()
{
    disconnectAndDelete();
    s_instance = nullptr;
}

MainWindow *MainWindow::instance()
{
    return s_instance;
}

void MainWindow::on_sendButton_clicked()
{
    const bool isSerial = serialRadio->isChecked();
    const bool isCustom = (isSerial ? fcSerialDrop : fcTcpDrop)->currentIndex() == 0;
    const QByteArray pduData = QByteArray::fromHex((isSerial ? pduSerialLine : pduTcpLine)->text()
        .toLatin1());

    QModbusReply *reply = nullptr;
    if (isCustom && pduData.isEmpty()) {
        qDebug() << "Error: Cannot send custom PDU without any data.";
        return;
    }

    const quint8 address = quint8((isSerial ? addressSpin : ui1Spin)->value());
    if (isCustom) {
        qDebug() << "Send: Sending custom PDU.";
        reply = m_device->sendRawRequest(QModbusRequest(QModbusRequest::FunctionCode(
            pduData[0]), pduData.mid(1)), address);
    } else {
        qDebug() << "Send: Sending PDU with predefined function code.";
        quint16 fc = (isSerial ? fcSerialDrop : fcTcpDrop)->currentText().left(4).toUShort(0, 16);
        reply = m_device->sendRawRequest(QModbusRequest(QModbusRequest::FunctionCode(fc), pduData),
            address);
    }

    if (reply) {
        sendButton->setDisabled(true);
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [reply, this]() {
                sendButton->setEnabled(true);
                qDebug() << "Receive: Asynchronous response PDU: " << reply->rawResult() << Qt::endl;
            });
        } else {
            sendButton->setEnabled(true);
            qDebug() << "Receive: Synchronous response pdu: " << reply->rawResult() << Qt::endl;
        }
    }
}

void MainWindow::on_connectButton_clicked()
{
    if (tcpRadio->isChecked()) {
        auto device = new ModbusTcpClient;
        connect(ti1Spin, &QSpinBox::valueChanged,
                device, &ModbusTcpClient::valueChanged);
        connect(ti2Spin, &QSpinBox::valueChanged,
                device, &ModbusTcpClient::valueChanged);

        connect(pi1Spin, &QSpinBox::valueChanged,
                device, &ModbusTcpClient::valueChanged);
        connect(pi2Spin, &QSpinBox::valueChanged,
                device, &ModbusTcpClient::valueChanged);

        connect(l1Spin, &QSpinBox::valueChanged,
                device, &ModbusTcpClient::valueChanged);
        connect(l2Spin, &QSpinBox::valueChanged,
                device, &ModbusTcpClient::valueChanged);

        connect(ui1Spin, &QSpinBox::valueChanged,
                device, &ModbusTcpClient::valueChanged);

        m_device = device;
        device->valueChanged(0);    // trigger update
        m_device->setConnectionParameter(QModbusDevice::NetworkAddressParameter,
            tcpAddressEdit->text());
        m_device->setConnectionParameter(QModbusDevice::NetworkPortParameter,
            tcpPortEdit->text());
    } else {
        m_device = new QModbusRtuSerialClient;
        m_device->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
            serialPortCombo->currentText());

        int parity = parityCombo->currentIndex();
        if (parity > 0)
            parity++;
        m_device->setConnectionParameter(QModbusDevice::SerialParityParameter, parity);
        m_device->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
            dataBitsCombo->currentText().toInt());
        m_device->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
            stopBitsCombo->currentText().toInt());
        m_device->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
            baudRateCombo->currentText().toInt());
    }
    m_device->setTimeout(timeoutSpin->value());
    m_device->setNumberOfRetries(retriesSpin->value());

    connect(m_device, &QModbusDevice::errorOccurred, this, [this](QModbusDevice::Error) {
        qDebug().noquote() << QStringLiteral("Error: %1").arg(m_device->errorString());
        emit disconnectButton->clicked();
    }, Qt::QueuedConnection);

    connect(m_device, &QModbusDevice::stateChanged, this, [](QModbusDevice::State state) {
        switch (state) {
        case QModbusDevice::UnconnectedState:
            qDebug().noquote() << QStringLiteral("State: Entered unconnected state.");
            break;
        case QModbusDevice::ConnectingState:
            qDebug().noquote() << QStringLiteral("State: Entered connecting state.");
            break;
        case QModbusDevice::ConnectedState:
            qDebug().noquote() << QStringLiteral("State: Entered connected state.");
            break;
        case QModbusDevice::ClosingState:
            qDebug().noquote() << QStringLiteral("State: Entered closing state.");
            break;
        }
    });
    m_device->connectDevice();
}

void MainWindow::on_disconnectButton_clicked()
{
    disconnectAndDelete();
}

void MainWindow::on_serialPortCombo_currentIndexChanged(int index)
{
    const bool custom = serialPortCombo->itemData(index, Qt::UserRole).toBool();
    serialPortCombo->setEditable(custom);
    if (custom) {
        serialPortCombo->clearEditText();
        serialPortCombo->lineEdit()->setPlaceholderText(QStringLiteral("Type here..."));
    }
}

void MainWindow::disconnectAndDelete()
{
    if (!m_device)
        return;
    m_device->disconnectDevice();
    m_device->disconnect();
    delete m_device;
    m_device = nullptr;
}
