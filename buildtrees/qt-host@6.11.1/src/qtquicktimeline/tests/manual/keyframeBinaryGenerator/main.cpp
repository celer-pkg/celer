// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QCoreApplication>
#include <QtCore/QCborStreamWriter>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QMetaType>
#include <QtGui/QColor>
#include <QtGui/QVector3D>
#include "keyframedatautils_p.h"

struct Keyframe
{
    double frame;
    QVariant value;
    int easing = 0;
};

struct KeyframeData
{
    QString filename;
    QMetaType::Type propertyType;
    QList<Keyframe> keyframes;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QList<KeyframeData> data;

    // Animate real ('opacity' property)
    KeyframeData keyframeData1;
    keyframeData1.filename = "animate_real.cbor";
    keyframeData1.propertyType = QMetaType::QReal;
    Keyframe key1_1;
    key1_1.frame = 0;
    key1_1.value = 0.0;
    key1_1.easing = 0;
    keyframeData1.keyframes.append(key1_1);
    Keyframe key1_2;
    key1_2.frame = 1000;
    key1_2.value = 1.0;
    key1_2.easing = 0;
    keyframeData1.keyframes.append(key1_2);
    Keyframe key1_3;
    key1_3.frame = 2000;
    key1_3.value = 0.0;
    key1_3.easing = 0;
    keyframeData1.keyframes.append(key1_3);
    data.append(keyframeData1);

    // Animate bool ('visible' property)
    KeyframeData keyframeData2;
    keyframeData2.filename = "animate_bool.cbor";
    keyframeData2.propertyType = QMetaType::Bool;
    Keyframe key2_1;
    key2_1.frame = 0;
    key2_1.value = true;
    key2_1.easing = 0;
    keyframeData2.keyframes.append(key2_1);
    Keyframe key2_2;
    key2_2.frame = 1000;
    key2_2.value = false;
    key2_2.easing = 0;
    keyframeData2.keyframes.append(key2_2);
    Keyframe key2_3;
    key2_3.frame = 2000;
    key2_3.value = true;
    key2_3.easing = 0;
    keyframeData2.keyframes.append(key2_3);
    data.append(keyframeData2);

    // Animate color
    KeyframeData keyframeData3;
    keyframeData3.filename = "animate_color.cbor";
    keyframeData3.propertyType = QMetaType::QColor;
    Keyframe key3_1;
    key3_1.frame = 0;
    key3_1.value = QColor("green");
    key3_1.easing = 0;
    keyframeData3.keyframes.append(key3_1);
    Keyframe key3_2;
    key3_2.frame = 1000;
    key3_2.value = QColor(255, 255, 0);
    key3_2.easing = 0;
    keyframeData3.keyframes.append(key3_2);
    Keyframe key3_3;
    key3_3.frame = 2000;
    key3_3.value = QColor(255, 255, 255);
    key3_3.easing = 0;
    keyframeData3.keyframes.append(key3_3);
    data.append(keyframeData3);

    // Animate vector3d
    KeyframeData keyframeData4;
    keyframeData4.filename = "animate_vector3d.cbor";
    keyframeData4.propertyType = QMetaType::QVector3D;
    Keyframe key4_1;
    key4_1.frame = 0;
    key4_1.value = QVector3D(500.0, 500.0, 500.0);
    key4_1.easing = 0;
    keyframeData4.keyframes.append(key4_1);
    Keyframe key4_2;
    key4_2.frame = 1000;
    key4_2.value = QVector3D(100.0, 200.0, 300.0);
    key4_2.easing = 0;
    keyframeData4.keyframes.append(key4_2);
    Keyframe key4_3;
    key4_3.frame = 2000;
    key4_3.value = QVector3D(-1000.0, 0.0, 0.0);
    key4_3.easing = 0;
    keyframeData4.keyframes.append(key4_3);
    data.append(keyframeData4);

    for (auto keyframeData : data) {
        // Output into current running path
        QString path = QDir::currentPath();
        QString outputFile = path + "\\" + keyframeData.filename;
        QFile output(outputFile);

        qDebug() << "Generating:" << outputFile;

        if (!output.open(QIODevice::WriteOnly)) {
            qWarning() << "Unable to open output file:" << outputFile;
            exit(0);
        }

        QCborStreamWriter writer(&output);

        writeKeyframesHeader(writer, keyframeData.propertyType);

        // Start keyframes array
        writer.startArray();
        for (auto keyFrame : keyframeData.keyframes) {
            writer.append(keyFrame.frame);
            writer.append(keyFrame.easing);
            writeValue(writer, keyFrame.value);
        }
        // End Keyframes array
        writer.endArray();

        // End root array
        writer.endArray();

        output.close();
    }

    return app.exec();
}
