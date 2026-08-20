// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "meshloader_p.h"

#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtGui/QVector2D>

QT_BEGIN_NAMESPACE

QString slashTag = QStringLiteral("/");

bool MeshLoader::loadOBJ(const QString &path, QList<QVector3D> &out_vertices,
                         QList<QVector2D> &out_uvs, QList<QVector3D> &out_normals)
{
    QList<unsigned int> vertexIndices, uvIndices, normalIndices;
    QList<QVector3D> temp_vertices;
    QList<QVector2D> temp_uvs;
    QList<QVector3D> temp_normals;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Cannot open the file");
        return false;
    }

    QTextStream textIn(&file);
    while (!textIn.atEnd()) {
        QString line = textIn.readLine();
        QStringList lineContents = line.split(QStringLiteral(" "));
        if (!lineContents.at(0).compare(QStringLiteral("v"))) {
            QVector3D vertex;
            vertex.setX(lineContents.at(1).toFloat());
            vertex.setY(lineContents.at(2).toFloat());
            vertex.setZ(lineContents.at(3).toFloat());
            temp_vertices.append(vertex);
        }
        else if (!lineContents.at(0).compare(QStringLiteral("vt"))) {
            QVector2D uv;
            uv.setX(lineContents.at(1).toFloat());
            uv.setY(lineContents.at(2).toFloat()); // invert this if using DDS textures
            temp_uvs.append(uv);
        }
        else if (!lineContents.at(0).compare(QStringLiteral("vn"))) {
            QVector3D normal;
            normal.setX(lineContents.at(1).toFloat());
            normal.setY(lineContents.at(2).toFloat());
            normal.setZ(lineContents.at(3).toFloat());
            temp_normals.append(normal);
        }
        else if (!lineContents.at(0).compare(QStringLiteral("f"))) {
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            QStringList set1 = lineContents.at(1).split(slashTag);
            QStringList set2 = lineContents.at(2).split(slashTag);
            QStringList set3 = lineContents.at(3).split(slashTag);
            if (set1.size() < 3 || set2.size() < 3 || set3.size() < 3) {
                qWarning("The file being loaded is missing UVs and/or normals");
                return false;
            }
            vertexIndex[0] = set1.at(0).toUInt();
            vertexIndex[1] = set2.at(0).toUInt();
            vertexIndex[2] = set3.at(0).toUInt();
            uvIndex[0] = set1.at(1).toUInt();
            uvIndex[1] = set2.at(1).toUInt();
            uvIndex[2] = set3.at(1).toUInt();
            normalIndex[0] = set1.at(2).toUInt();
            normalIndex[1] = set2.at(2).toUInt();
            normalIndex[2] = set3.at(2).toUInt();
            vertexIndices.append(vertexIndex[0]);
            vertexIndices.append(vertexIndex[1]);
            vertexIndices.append(vertexIndex[2]);
            uvIndices.append(uvIndex[0]);
            uvIndices.append(uvIndex[1]);
            uvIndices.append(uvIndex[2]);
            normalIndices.append(normalIndex[0]);
            normalIndices.append(normalIndex[1]);
            normalIndices.append(normalIndex[2]);
        }
    }

    // For each vertex of each triangle
    for (int i = 0; i < vertexIndices.size(); i++) {
        // Get the indices of its attributes
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int uvIndex = uvIndices[i];
        unsigned int normalIndex = normalIndices[i];

        // Get the attributes thanks to the index
        QVector3D vertex = temp_vertices[vertexIndex - 1];
        QVector2D uv = temp_uvs[uvIndex - 1];
        QVector3D normal = temp_normals[normalIndex - 1];

        // Put the attributes in buffers
        out_vertices.append(vertex);
        out_uvs.append(uv);
        out_normals.append(normal);
    }

    return true;
}

QT_END_NAMESPACE
