// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef BUILDMESSAGE_H
#define BUILDMESSAGE_H

#include <QtCore/qobject.h>
#include <QtCore/qdebug.h>
#include <QtQml/qqmlregistration.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercache_p.h>

QT_BEGIN_NAMESPACE

struct BuildMessage
{
    using Status = QtQuick3DEditorHelpers::ShaderBaker::Status;
    QString message;
    QString identifier;
    qint64 line = -1;
    qint64 column = -1;
    Status status = Status::Success;

    friend QDebug operator<<(QDebug stream, const BuildMessage &err)
    {
        stream << err.message;
        if (err.line != -1)
            stream << "at line: " << err.line;
        return stream;
    }
};

class ShaderBuildMessage
{
    Q_GADGET
    Q_PROPERTY(QString filename READ filename)
    Q_PROPERTY(QString message READ message)
    Q_PROPERTY(QString identifier READ identifier)
    Q_PROPERTY(qint64 line READ line)
    Q_PROPERTY(Status status READ status)
    Q_PROPERTY(Stage stage READ stage)
    QML_VALUE_TYPE(shaderStatus)
public:
    enum class Status
    {
        Success,
        Error
    };
    Q_ENUM(Status)
    enum class Stage
    {
        Vertex,
        Fragment
    };
    Q_ENUM(Stage)

    ShaderBuildMessage() = default;
    ShaderBuildMessage(const BuildMessage &data, const QString &filename, Stage stage);
    const QString &filename() const { return m_filename; }
    const QString &message() const { return m_message.message; }
    const QString &identifier() const { return m_message.identifier; }
    qint64 line() const { return m_message.line; }
    Status status() const { return static_cast<Status>(m_message.status); }
    Stage stage() const { return m_stage; }

private:
    BuildMessage m_message;
    QString m_filename;
    Stage m_stage;
};

// To prevent the same type from being exported twice into qmltypes
// (for value type and for the enums)
struct ShaderBuildMessageDerived : public ShaderBuildMessage
{
    Q_GADGET
};

namespace ShaderEnums {
Q_NAMESPACE
QML_FOREIGN_NAMESPACE(ShaderBuildMessageDerived)
QML_NAMED_ELEMENT(ShaderConstants)
}

QT_END_NAMESPACE

#endif // BUILDMESSAGE_H
