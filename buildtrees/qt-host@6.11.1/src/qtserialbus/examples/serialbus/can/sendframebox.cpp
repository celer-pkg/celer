// Copyright (C) 2017 Andre Hartmann <aha_1980@gmx.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "sendframebox.h"
#include "ui_sendframebox.h"

using namespace Qt::StringLiterals;

static const QRegularExpression &threeHexDigitsPattern()
{
    static const QRegularExpression result(u"[[:xdigit:]]{3}"_s);
    Q_ASSERT(result.isValid());
    return result;
}

static const QRegularExpression &oneDigitAndSpacePattern()
{
    static const QRegularExpression result(u"((\\s+)|^)([[:xdigit:]]{1})(\\s+)"_s);
    Q_ASSERT(result.isValid());
    return result;
}

const QRegularExpression &hexNumberPattern()
{
    static const QRegularExpression result(u"^[[:xdigit:]]*$"_s);
    Q_ASSERT(result.isValid());
    return result;
}

const QRegularExpression &twoSpacesPattern()
{
    static const QRegularExpression result(u"([\\s]{2})"_s);
    Q_ASSERT(result.isValid());
    return result;
}

enum {
    MaxStandardId = 0x7FF,
    MaxExtendedId = 0x10000000
};

enum {
    MaxPayload = 8,
    MaxPayloadFd = 64
};

static bool isEvenHex(QString input)
{
    input.remove(u' ');
    return (input.size() % 2) == 0;
}

// Formats a string of hex characters with a space between every byte
// Example: "012345" -> "01 23 45"
static QString formatHexData(const QString &input)
{
    QString out = input;

    while (true) {
        if (auto match = threeHexDigitsPattern().match(out); match.hasMatch()) {
            out.insert(match.capturedEnd() - 1, u' ');
        } else if (match = oneDigitAndSpacePattern().match(out); match.hasMatch()) {
            const auto pos = match.capturedEnd() - 1;
            if (out.at(pos) == u' ')
                out.remove(pos, 1);
        } else {
            break;
        }
    }

    return out.simplified().toUpper();
}

HexIntegerValidator::HexIntegerValidator(QObject *parent) :
    QValidator(parent),
    m_maximum(MaxStandardId)
{
}

QValidator::State HexIntegerValidator::validate(QString &input, int &) const
{
    if (input.isEmpty())
        return Intermediate;
    bool ok;
    uint value = input.toUInt(&ok, 16);
    return ok && value <= m_maximum ? Acceptable : Invalid;
}

void HexIntegerValidator::setMaximum(uint maximum)
{
    m_maximum = maximum;
}

HexStringValidator::HexStringValidator(QObject *parent) :
    QValidator(parent),
    m_maxLength(MaxPayload)
{
}

QValidator::State HexStringValidator::validate(QString &input, int &pos) const
{
    const int maxSize = 2 * m_maxLength;
    QString data = input;

    data.remove(u' ');

    if (data.isEmpty())
        return Intermediate;

    // limit maximum size
    if (data.size() > maxSize)
        return Invalid;

    // check if all input is valid
    if (!hexNumberPattern().match(data).hasMatch())
        return Invalid;

    // don't allow user to enter more than one space
    if (const auto match = twoSpacesPattern().match(input); match.hasMatch()) {
        input.replace(match.capturedStart(), 2, ' ');
        pos = match.capturedEnd() - 1;
    }

    // insert a space after every two hex nibbles
    while (true) {
        const QRegularExpressionMatch match = threeHexDigitsPattern().match(input);
        if (!match.hasMatch())
            break;
        const auto start = match.capturedStart();
        const auto end = match.capturedEnd();
        if (pos == start + 1) {
            // add one hex nibble before two - Abc
            input.insert(pos, u' ');
        } else if (pos == start + 2) {
            // add hex nibble in the middle - aBc
            input.insert(end - 1, u' ');
            pos = end;
        } else {
            // add one hex nibble after two - abC
            input.insert(end - 1, u' ');
            pos = end + 1;
        }
    }

    return Acceptable;
}

void HexStringValidator::setMaxLength(int maxLength)
{
    m_maxLength = maxLength;
}

SendFrameBox::SendFrameBox(QWidget *parent) :
    QGroupBox(parent),
    m_ui(new Ui::SendFrameBox)
{
    m_ui->setupUi(this);

    m_hexIntegerValidator = new HexIntegerValidator(this);
    m_ui->frameIdEdit->setValidator(m_hexIntegerValidator);
    m_hexStringValidator = new HexStringValidator(this);
    m_ui->payloadEdit->setValidator(m_hexStringValidator);

    connect(m_ui->dataFrame, &QRadioButton::toggled, this, [this](bool set) {
        if (set)
            m_ui->flexibleDataRateBox->setEnabled(true);
    });

    connect(m_ui->remoteFrame, &QRadioButton::toggled, this, [this](bool set) {
        if (set) {
            m_ui->flexibleDataRateBox->setEnabled(false);
            m_ui->flexibleDataRateBox->setChecked(false);
        }
    });

    connect(m_ui->errorFrame, &QRadioButton::toggled, this, [this](bool set) {
        if (set) {
            m_ui->flexibleDataRateBox->setEnabled(false);
            m_ui->flexibleDataRateBox->setChecked(false);
        }
    });

    connect(m_ui->extendedFormatBox, &QCheckBox::toggled, this, [this](bool set) {
        m_hexIntegerValidator->setMaximum(set ? MaxExtendedId : MaxStandardId);
    });

    connect(m_ui->flexibleDataRateBox, &QCheckBox::toggled, this, [this](bool set) {
        m_hexStringValidator->setMaxLength(set ? MaxPayloadFd : MaxPayload);
        m_ui->bitrateSwitchBox->setEnabled(set);
        if (!set)
            m_ui->bitrateSwitchBox->setChecked(false);
    });

    auto frameIdOrPayloadChanged = [this]() {
        const bool hasFrameId = !m_ui->frameIdEdit->text().isEmpty();
        m_ui->sendButton->setEnabled(hasFrameId);
        m_ui->sendButton->setToolTip(hasFrameId
                                     ? QString() : tr("Cannot send because no Frame ID was given."));
        if (hasFrameId) {
            const bool isEven = isEvenHex(m_ui->payloadEdit->text());
            m_ui->sendButton->setEnabled(isEven);
            m_ui->sendButton->setToolTip(isEven
                                         ? QString() : tr("Cannot send because Payload hex string is invalid."));
        }
    };
    connect(m_ui->frameIdEdit, &QLineEdit::textChanged, this, frameIdOrPayloadChanged);
    connect(m_ui->payloadEdit, &QLineEdit::textChanged, this, frameIdOrPayloadChanged);
    frameIdOrPayloadChanged();

    connect(m_ui->sendButton, &QPushButton::clicked, this, [this]() {
    //! [prepare_can_frame]
        const uint frameId = m_ui->frameIdEdit->text().toUInt(nullptr, 16);
        QString data = m_ui->payloadEdit->text();
        m_ui->payloadEdit->setText(formatHexData(data));
        const QByteArray payload = QByteArray::fromHex(data.remove(u' ').toLatin1());

        QCanBusFrame frame = QCanBusFrame(frameId, payload);
        frame.setExtendedFrameFormat(m_ui->extendedFormatBox->isChecked());
        frame.setFlexibleDataRateFormat(m_ui->flexibleDataRateBox->isChecked());
        frame.setBitrateSwitch(m_ui->bitrateSwitchBox->isChecked());

        if (m_ui->errorFrame->isChecked())
            frame.setFrameType(QCanBusFrame::ErrorFrame);
        else if (m_ui->remoteFrame->isChecked())
            frame.setFrameType(QCanBusFrame::RemoteRequestFrame);
    //! [prepare_can_frame]

        emit sendFrame(frame);
    });
}

SendFrameBox::~SendFrameBox()
{
    delete m_ui;
}
