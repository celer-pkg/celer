// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottieroot_p.h"
#include "qlottielayer_p.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QString>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottieRoot::QLottieRoot(const QLottieRoot &other)
    : QLottieBase(other)
{
}

QLottieBase *QLottieRoot::clone() const
{
    return new QLottieRoot(*this);
}

QSize QLottieRoot::layerSize() const
{
    return m_size;
}

int QLottieRoot::parseSource(const QByteArray &jsonSource, const QUrl &fileSource)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonSource);
    QJsonObject rootObj = doc.object();
    m_definition = rootObj;

    if (rootObj.empty())
        return -1;

    QMap<QString, QJsonObject> assets;
    QJsonArray jsonLayers = rootObj.value("layers"_L1).toArray();
    QJsonArray jsonAssets = rootObj.value("assets"_L1).toArray();
    QString name = rootObj.value("nm"_L1).toString();

    if (!checkRequiredKeys(rootObj, ""_L1, { "fr"_L1, "ip"_L1, "op"_L1 }, name))
        return -1;

    m_frameRate = rootObj.value("fr"_L1).toVariant().toInt();
    if (m_frameRate <= 0) {
        qCWarning(lcLottieQtLottieParser) << "\"fr\" value of" << name << "should be greater than 0";
        return -1;
    }

    m_startFrame = rootObj.value("ip"_L1).toVariant().toInt();
    m_endFrame = rootObj.value("op"_L1).toVariant().toInt();

    m_size = QSize(rootObj.value("w"_L1).toInt(-1), rootObj.value("h"_L1).toInt(-1));

    QJsonArray::const_iterator jsonAssetsIt = jsonAssets.constBegin();
    while (jsonAssetsIt != jsonAssets.constEnd()) {
        QJsonObject jsonAsset = (*jsonAssetsIt).toObject();

        jsonAsset.insert("fileSource"_L1, QJsonValue::fromVariant(fileSource));
        if (!checkRequiredKeys(jsonAsset, "Asset"_L1, { "id"_L1 }))
            return -1;

        QString id = jsonAsset.value("id"_L1).toString();
        assets.insert(id, jsonAsset);
        jsonAssetsIt++;
    }

    int ret = QLottieLayer::constructLayers(jsonLayers, this, assets);

    return ret;
}

void QLottieRoot::setStructureDumping(bool enabled)
{
    m_structureDumping = enabled ? 1 : 0;
}

QT_END_NAMESPACE
