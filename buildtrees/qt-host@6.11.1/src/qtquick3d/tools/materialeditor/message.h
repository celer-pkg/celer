// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MESSAGE_H
#define MESSAGE_H

#include <QtCore/qglobal.h>
#include <QtCore/qlist.h>

#include <QtCore/qbytearray.h>

QT_BEGIN_NAMESPACE

class QIODevice;

using MessageId = QByteArrayView;

struct Message
{
    enum class Type { Invalid, Uniforms, Filenames };

    using MessagePtr = std::shared_ptr<Message>;

    virtual ~Message();
    // NOTE! These functions assume the device is open and read/writeable when called!
    virtual bool read(QDataStream &device) = 0;
    virtual bool write(QDataStream &device) const = 0;
    virtual Type type() const = 0;

    static Message::MessagePtr getMessage(QIODevice &device);
    static bool postMessage(QIODevice &device, const Message &message);

    friend QDataStream &operator>>(QDataStream &stream, Message &message) { message.read(stream); return stream; }
    friend QDataStream &operator<<(QDataStream &stream, const Message &message) { message.write(stream); return stream; }
};

struct UniformsMessage : Message
{
    static constexpr MessageId id { "UM" };

    using Uniform = std::pair<QByteArray /* type */, QByteArray /* name */>;
    using UniformList = QList<Uniform>;
    UniformList uniforms;

    UniformsMessage() = default;
    UniformsMessage(UniformList list) : uniforms(list) {}
    ~UniformsMessage() override;

    bool read(QDataStream &stream) final;
    bool write(QDataStream &stream) const final;
    Type type() const final { return Type::Uniforms; }
};

struct FilenamesMessage : Message
{
    static constexpr MessageId id { "FM" };

    ~FilenamesMessage() override;
    bool read(QDataStream &stream) final;
    bool write(QDataStream &stream) const final;
    Type type() const final { return Type::Filenames; }
    QByteArray vertFilename;
    QByteArray fragFilename;
};

QT_END_NAMESPACE

#endif // MESSAGE_H
