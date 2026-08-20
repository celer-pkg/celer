// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QNetworkAccessManager>
#include <QString>

class ImageDownloader : public QObject
{
    Q_OBJECT

public:
    ImageDownloader(QObject *parent = nullptr);

    int downloadCount() const;

public slots:
    void downloadImages();

signals:
    void finished();
    void downloadStart(int num);
    void downloadProgress(double progress);

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();

private:
    void downloadNextAsset();
    QStringList getDownloadList() const;

    QStringList m_downloadPaths;
    QString m_savePath;
    int m_downloadedCount = 0;
    QNetworkAccessManager m_manager;
    QString m_baseLocal;

    const static inline QString BASE_REMOTE = "https://download.qt.io/learning/examples/FX_Material_Showroom_Assets/";

    const static inline QStringList assetsList {
       "images/ambientdot.tif",
       "images/AOBake.png",
       "images/Asphalt010_2K_NormalGL.png",
       "images/Asphalt010_2K_Roughness.png",
       "images/cloud.png",
       "images/debris.png",
       "images/dotsprite.tif",
       "images/dust.png",
       "images/Fabric004_2K_NormalGL.png",
       "images/Fabric031_2K_Color.png",
       "images/Fabric031_2K_Detail.png",
       "images/Fabric031_2K_Displacement.png",
       "images/Fabric031_2K_NormalGL.png",
       "images/Fabric031_2K_Roughness.png",
       "images/FabricmatIcon.png",
       "images/fire.png",
       "images/LeathermatIcon.png",
       "images/lineSprite.tif",
       "images/Metal009_2K_NormalGL.png",
       "images/Metal009_2K_Roughness.png",
       "images/Metal029_2K_Displacement.jpg",
       "images/noisenormal.png",
       "images/raindrops_multi.tga",
       "images/rainsplash.png",
       "images/smoke.png",
       "images/snowflake_DF_multi.tga",
       "images/spark.png",
       "images/steam.png",
       "images/HDR/dreamstime_xl_119184006.png",
       "images/HDR/dreamstime_xl_182890747.png",
       "images/Icons/BrushedSteelmatIcon.png",
       "images/Icons/CarbonFibermatIcon.png",
       "images/Icons/CheckboxImagesfxIcon-1.png",
       "images/Icons/CheckboxImagesfxIcon-10.png",
       "images/Icons/CheckboxImagesfxIcon-2.png",
       "images/Icons/CheckboxImagesfxIcon-3.png",
       "images/Icons/CheckboxImagesfxIcon-4.png",
       "images/Icons/CheckboxImagesfxIcon-4.tif",
       "images/Icons/CheckboxImagesfxIcon-5.png",
       "images/Icons/CheckboxImagesfxIcon-6.png",
       "images/Icons/CheckboxImagesfxIcon-7.png",
       "images/Icons/CheckboxImagesfxIcon-8.png",
       "images/Icons/CheckboxImagesfxIcon-9.png",
       "images/Icons/CheckboxImagesfxIcon.png",
       "images/Icons/CheckboxImagesmodelIcon-1.png",
       "images/Icons/CheckboxImagesmodelIcon-2.png",
       "images/Icons/CheckboxImagesmodelIcon-3.png",
       "images/Icons/CheckboxImagesmodelIcon.png",
       "images/Icons/CoppermatIcon.png",
       "images/Icons/FabricmatIcon.png",
       "images/Icons/GlassmatIcon.png",
       "images/Icons/Icon_Colorful.png",
       "images/Icons/Icon_Dark.png",
       "images/Icons/Icon_Light.png",
       "images/Icons/LeathermatIcon.png",
       "images/Icons/PlasticTexturedmatIcon.png",
       "images/Icons/SilvermatIcon.png",
       "images/Icons/StonematIcon.png",
       "images/Icons/WoodmatIcon.png",
       "images/leathertextures/Leather037_2K-PNG/Leather037_2K_Color.png",
       "images/leathertextures/Leather037_2K-PNG/Leather037_2K_NormalGL.png",
       "images/leathertextures/Leather037_2K-PNG/Leather037_2K_Roughness.png",
       "images/stonetextures/Rock023_2K_AmbientOcclusion.png",
       "images/stonetextures/Rock023_2K_Color.png",
       "images/stonetextures/Rock023_2K_NormalGL.png",
       "images/stonetextures/Rock023_2K_Roughness.png",
       "images/woodtextures/Wood048_2K-PNG/Wood048_2K_Color.png",
       "images/woodtextures/Wood048_2K-PNG/Wood048_2K_NormalGL.png",
       "images/woodtextures/Wood048_2K-PNG/Wood048_2K_Roughness.png",
       "Meshes/bunnyUV.mesh",
       "Meshes/floor.mesh",
       "Meshes/materialBall.mesh",
       "Meshes/stanford_Dragon.mesh"
    };
};
