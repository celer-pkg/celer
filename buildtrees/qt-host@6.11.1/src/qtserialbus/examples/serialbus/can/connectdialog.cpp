// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "connectdialog.h"
#include "ui_connectdialog.h"

#include <QCanBus>
#include <QSettings>

ConnectDialog::ConnectDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ConnectDialog),
    m_settings(new QSettings("QtProject", "CAN example"))
{
    m_ui->setupUi(this);

    m_ui->errorFilterEdit->setValidator(new QIntValidator(0, 0x1FFFFFFFU, this));

    m_ui->loopbackBox->addItem(tr("unspecified"), QVariant());
    m_ui->loopbackBox->addItem(tr("false"), QVariant(false));
    m_ui->loopbackBox->addItem(tr("true"), QVariant(true));

    m_ui->receiveOwnBox->addItem(tr("unspecified"), QVariant());
    m_ui->receiveOwnBox->addItem(tr("false"), QVariant(false));
    m_ui->receiveOwnBox->addItem(tr("true"), QVariant(true));

    m_ui->canFdBox->addItem(tr("false"), QVariant(false));
    m_ui->canFdBox->addItem(tr("true"), QVariant(true));

    m_ui->dataBitrateBox->setFlexibleDateRateEnabled(true);

    connect(m_ui->okButton, &QPushButton::clicked, this, &ConnectDialog::ok);
    connect(m_ui->cancelButton, &QPushButton::clicked, this, &ConnectDialog::cancel);
    connect(m_ui->useConfigurationBox, &QCheckBox::toggled,
            m_ui->configurationBox, &QGroupBox::setEnabled);
    connect(m_ui->pluginListBox, &QComboBox::currentTextChanged,
            this, &ConnectDialog::pluginChanged);
    connect(m_ui->interfaceListBox, &QComboBox::currentTextChanged,
            this, &ConnectDialog::interfaceChanged);
    connect(m_ui->ringBufferBox, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState state){
            m_ui->ringBufferLimitBox->setEnabled(state == Qt::CheckState::Checked);
    });

    m_ui->rawFilterEdit->hide();
    m_ui->rawFilterLabel->hide();

    m_ui->pluginListBox->addItems(QCanBus::instance()->plugins());

    restoreSettings();
}

ConnectDialog::~ConnectDialog()
{
    delete m_settings;
    delete m_ui;
}

ConnectDialog::Settings ConnectDialog::settings() const
{
    return m_currentSettings;
}


void ConnectDialog::saveSettings()
{
    m_settings->beginGroup("LastSettings");

    m_settings->setValue("PluginName", m_currentSettings.pluginName);
    m_settings->setValue("DeviceInterfaceName", m_currentSettings.deviceInterfaceName);
    m_settings->setValue("UseAutoscroll", m_currentSettings.useAutoscroll);
    m_settings->setValue("UseRingBuffer", m_currentSettings.useModelRingBuffer);
    m_settings->setValue("RingBufferSize", m_currentSettings.modelRingBufferSize);
    m_settings->setValue("UseCustomConfiguration", m_currentSettings.useConfigurationEnabled);

    if (m_currentSettings.useConfigurationEnabled) {
        m_settings->setValue("Loopback", configurationValue(QCanBusDevice::LoopbackKey));
        m_settings->setValue("ReceiveOwn", configurationValue(QCanBusDevice::ReceiveOwnKey));
        m_settings->setValue("ErrorFilter", configurationValue(QCanBusDevice::ErrorFilterKey));
        m_settings->setValue("BitRate", configurationValue(QCanBusDevice::BitRateKey));
        m_settings->setValue("CanFd", configurationValue(QCanBusDevice::CanFdKey));
        m_settings->setValue("DataBitRate", configurationValue(QCanBusDevice::DataBitRateKey));
    }

    m_settings->endGroup();
}

void ConnectDialog::restoreSettings()
{
    m_settings->beginGroup("LastSettings");

    m_currentSettings.pluginName = m_settings->value("PluginName").toString();
    m_currentSettings.deviceInterfaceName = m_settings->value("DeviceInterfaceName").toString();
    m_currentSettings.useAutoscroll = m_settings->value("UseAutoscroll").toBool();
    m_currentSettings.useModelRingBuffer = m_settings->value("UseRingBuffer").toBool();
    m_currentSettings.modelRingBufferSize = m_settings->value("RingBufferSize").toInt();
    m_currentSettings.useConfigurationEnabled = m_settings->value("UseCustomConfiguration").toBool();

    revertSettings();

    auto setting = [this](const QString &key) {
        return m_settings->value(key).toString();
    };

    if (m_currentSettings.useConfigurationEnabled) {
        m_ui->loopbackBox->setCurrentText(setting("Loopback"));

        m_ui->receiveOwnBox->setCurrentText(setting("ReceiveOwn"));

        m_ui->errorFilterEdit->setText(setting("ErrorFilter"));

        m_ui->bitrateBox->setCurrentText(setting("BitRate"));

        m_ui->canFdBox->setCurrentText(setting("CanFd"));

        m_ui->dataBitrateBox->setCurrentText(setting("DataBitRate"));
    }

    m_settings->endGroup();

    updateSettings();
}

void ConnectDialog::pluginChanged(const QString &plugin)
{
    m_ui->interfaceListBox->clear();
    m_interfaces = QCanBus::instance()->availableDevices(plugin);
    for (const QCanBusDeviceInfo &info : std::as_const(m_interfaces))
        m_ui->interfaceListBox->addItem(info.name());
}

void ConnectDialog::interfaceChanged(const QString &interface)
{
    const auto deviceInfo = std::find_if(m_interfaces.constBegin(), m_interfaces.constEnd(),
                                   [interface](const QCanBusDeviceInfo &info) {
        return interface == info.name();
    });

    if (deviceInfo == m_interfaces.constEnd())
        m_ui->deviceInfoBox->clear();
    else
        m_ui->deviceInfoBox->setDeviceInfo(*deviceInfo);
}

void ConnectDialog::ok()
{
    updateSettings();
    saveSettings();
    accept();
}

void ConnectDialog::cancel()
{
    revertSettings();
    reject();
}

QString ConnectDialog::configurationValue(QCanBusDevice::ConfigurationKey key)
{
    QVariant result;

    for (const ConfigurationItem &item : std::as_const(m_currentSettings.configurations)) {
        if (item.first == key) {
            result = item.second;
            break;
        }
    }

    if (result.isNull() && (
                key == QCanBusDevice::LoopbackKey ||
                key == QCanBusDevice::ReceiveOwnKey)) {
        return tr("unspecified");
    }

    return result.toString();
}

void ConnectDialog::revertSettings()
{
    m_ui->pluginListBox->setCurrentText(m_currentSettings.pluginName);
    m_ui->interfaceListBox->setCurrentText(m_currentSettings.deviceInterfaceName);
    m_ui->useConfigurationBox->setChecked(m_currentSettings.useConfigurationEnabled);

    m_ui->ringBufferBox->setChecked(m_currentSettings.useModelRingBuffer);
    m_ui->ringBufferLimitBox->setValue(m_currentSettings.modelRingBufferSize);
    m_ui->autoscrollBox->setChecked(m_currentSettings.useAutoscroll);

    QString value = configurationValue(QCanBusDevice::LoopbackKey);
    m_ui->loopbackBox->setCurrentText(value);

    value = configurationValue(QCanBusDevice::ReceiveOwnKey);
    m_ui->receiveOwnBox->setCurrentText(value);

    value = configurationValue(QCanBusDevice::ErrorFilterKey);
    m_ui->errorFilterEdit->setText(value);

    value = configurationValue(QCanBusDevice::BitRateKey);
    m_ui->bitrateBox->setCurrentText(value);

    value = configurationValue(QCanBusDevice::CanFdKey);
    m_ui->canFdBox->setCurrentText(value);

    value = configurationValue(QCanBusDevice::DataBitRateKey);
    m_ui->dataBitrateBox->setCurrentText(value);
}

void ConnectDialog::updateSettings()
{
    m_currentSettings.pluginName = m_ui->pluginListBox->currentText();
    m_currentSettings.deviceInterfaceName = m_ui->interfaceListBox->currentText();
    m_currentSettings.useConfigurationEnabled = m_ui->useConfigurationBox->isChecked();

    m_currentSettings.useModelRingBuffer = m_ui->ringBufferBox->isChecked();
    m_currentSettings.modelRingBufferSize = m_ui->ringBufferLimitBox->value();
    m_currentSettings.useAutoscroll = m_ui->autoscrollBox->isChecked();

    if (m_currentSettings.useConfigurationEnabled) {
        m_currentSettings.configurations.clear();
        // process LoopBack
        if (m_ui->loopbackBox->currentIndex() != 0) {
            ConfigurationItem item;
            item.first = QCanBusDevice::LoopbackKey;
            item.second = m_ui->loopbackBox->currentData();
            m_currentSettings.configurations.append(item);
        }

        // process ReceiveOwnKey
        if (m_ui->receiveOwnBox->currentIndex() != 0) {
            ConfigurationItem item;
            item.first = QCanBusDevice::ReceiveOwnKey;
            item.second = m_ui->receiveOwnBox->currentData();
            m_currentSettings.configurations.append(item);
        }

        // process error filter
        if (!m_ui->errorFilterEdit->text().isEmpty()) {
            QString value = m_ui->errorFilterEdit->text();
            bool ok = false;
            value.toInt(&ok); // check if value contains a valid integer
            if (ok) {
                ConfigurationItem item;
                item.first = QCanBusDevice::ErrorFilterKey;
                item.second = value;
                m_currentSettings.configurations.append(item);
            }
        }

        // process raw filter list
        if (!m_ui->rawFilterEdit->text().isEmpty()) {
            //TODO current ui not sufficient to reflect this param
        }

        // process bitrate
        const int bitrate = m_ui->bitrateBox->bitRate();
        if (bitrate > 0) {
            const ConfigurationItem item(QCanBusDevice::BitRateKey, QVariant(bitrate));
            m_currentSettings.configurations.append(item);
        }

        // process CAN FD setting
        ConfigurationItem fdItem;
        fdItem.first = QCanBusDevice::CanFdKey;
        fdItem.second = m_ui->canFdBox->currentData();
        m_currentSettings.configurations.append(fdItem);

        // process data bitrate
        const int dataBitrate = m_ui->dataBitrateBox->bitRate();
        if (dataBitrate > 0 && m_ui->canFdBox->currentData().toBool()) {
            const ConfigurationItem item(QCanBusDevice::DataBitRateKey, QVariant(dataBitrate));
            m_currentSettings.configurations.append(item);
        }
    }
}
