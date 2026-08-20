// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#ifndef UTILS_H
#define UTILS_H

#include "types.h"

#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtHttpServer/QtHttpServer>

#include <optional>

static std::optional<QByteArray> readFileToByteArray(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return std::nullopt;

    return file.readAll();
}

static std::optional<QJsonArray> byteArrayToJsonArray(const QByteArray &arr)
{
    QJsonParseError err;
    const auto json = QJsonDocument::fromJson(arr, &err);
    if (err.error || !json.isArray())
        return std::nullopt;
    return json.array();
}

static std::optional<QJsonObject> byteArrayToJsonObject(const QByteArray &arr)
{
    QJsonParseError err;
    const auto json = QJsonDocument::fromJson(arr, &err);
    if (err.error || !json.isObject())
        return std::nullopt;
    return json.object();
}

template<typename T>
static IdMap<T> tryLoadFromFile(const FromJsonFactory<T> &itemFactory, const QString &path)
{
    const auto maybeBytes = readFileToByteArray(path);
    if (maybeBytes) {
        const auto maybeArray = byteArrayToJsonArray(*maybeBytes);
        if (maybeArray) {
            return IdMap<T>(itemFactory, *maybeArray);
        } else {
            qDebug() << "Content of " << path << " is not json array.";
        }
    } else {
        qDebug() << "Reading file " << path << " failed.";
    }
    return IdMap<T>();
}

static QByteArray getValueFromHeader(const QHttpHeaders &headers,
                                     QByteArrayView headerName)
{
    return headers.value(headerName).toByteArray();
}

static std::optional<QString> getTokenFromRequest(const QHttpServerRequest &request)
{
    std::optional<QString> token;
    if (auto bytes = getValueFromHeader(request.headers(), "token"); !bytes.isEmpty()) {
        token = bytes;
    }
    return token;
}

#endif // UTILS_H
