// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottieimage_p.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QString>

#include <QtLottie/private/qlottieconstants_p.h>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottieImage::QLottieImage(const QLottieImage &other)
    : QLottieBase(other)
{
    m_url = other.m_url;
    m_size = other.m_size;
    m_image = other.m_image;
}

QLottieImage::QLottieImage(QLottieBase *parent)
{
    setParent(parent);
}

QLottieBase *QLottieImage::clone() const
{
    return new QLottieImage(*this);
}

int QLottieImage::construct(const QJsonObject &definition)
{
    QLottieBase::parse(definition);
    if (m_hidden)
        return 0;

    qCDebug(lcLottieQtLottieParser) << "QLottieImage::parse():" << m_name;

    QJsonObject asset = definition.value("asset"_L1).toObject();
    if (!checkRequiredKeys(asset, "Image"_L1, { "p"_L1, "w"_L1, "h"_L1 }, m_name))
        return -1;

    QString assetString = asset.value("p"_L1).toString();

    if (assetString.startsWith("data:image"_L1)) {
        QStringList assetsDataStringList = assetString.split(","_L1);
        if (assetsDataStringList.size() > 1) {
            QByteArray assetData = QByteArray::fromBase64(assetsDataStringList[1].toLatin1());
            m_image.loadFromData(assetData);
        }
        if (m_image.isNull()) {
            qCWarning(lcLottieQtLottieParser) << "Unable to load embedded image asset"
                                              << asset.value("id"_L1).toString();
        }
    } else {
        QFileInfo info(asset.value("fileSource"_L1).toString());
        QString urlPath = info.path() + QLatin1Char('/') + asset.value("u"_L1).toString()
                + QLatin1Char('/') + assetString;
        m_url = QUrl(urlPath);
        m_url.setScheme("file"_L1);
        QString path = m_url.toLocalFile();
        m_image.load(path);
        if (m_image.isNull())
            qCWarning(lcLottieQtLottieParser) << "Unable to load file" << path;
    }

    const qreal width = asset.value("w"_L1).toDouble();
    const qreal height = asset.value("h"_L1).toDouble();
    m_size = QSizeF(width, height);

    return 0;
}

void QLottieImage::render(QLottieRenderer &renderer) const
{
    renderer.render(*this);
}

QT_END_NAMESPACE
