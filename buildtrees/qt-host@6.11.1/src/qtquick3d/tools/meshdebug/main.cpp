// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QFile>
#include <QtCore/QDebug>

#include <QtQuick3DUtils/private/qssgmesh_p.h>

using Qt::hex;

using namespace QSSGMesh;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Setup command line arguments
    QCommandLineParser cmdLineParser;
    cmdLineParser.setApplicationDescription(
            "Allows to debug the high level structure of .mesh files for use with Qt Quick 3D");
    cmdLineParser.addHelpOption();
    cmdLineParser.process(app);
    QStringList meshFileNames = cmdLineParser.positionalArguments();

    // if there is nothing to do return early
    if (meshFileNames.isEmpty())
        return -1;

    // Process Mesh files
    for (const auto &meshFileName : meshFileNames) {
        QFile meshFile(meshFileName);
        if (!meshFile.open(QIODevice::ReadOnly)) {
            qWarning() << "could not open" << meshFileName;
            continue;
        }

        MeshInternal::MultiMeshInfo multiHeader = MeshInternal::readFileHeader(&meshFile);
        if (multiHeader.isValid()) {
            // Print Multiheader information
            qDebug() << " -- Multiheader --";
            qDebug() << "fileId:" << multiHeader.fileId;
            qDebug() << "version:" << multiHeader.fileVersion;
            qDebug() << "mesh entries:" << multiHeader.meshEntries.size();

            for (auto it = multiHeader.meshEntries.cbegin(), end = multiHeader.meshEntries.cend(); it != end; ++it) {
                const quint32 meshId = it.key();
                const quint64 meshOffset = it.value();
                qDebug() << "\t -- mesh entry --";
                qDebug() << "\tid:" << meshId;
                qDebug() << "\toffset:" << Qt::hex << meshOffset;

                MeshInternal::MeshDataHeader header;
                Mesh mesh;
                if (!MeshInternal::readMeshData(&meshFile, meshOffset, &mesh, &header)) {
                    qWarning("Failed to load mesh body");
                    continue;
                }

                // Header
                qDebug() << "\t -- MeshDataHeader --";
                qDebug() << "\tfileId:" << header.fileId;
                qDebug() << "\tfileVersion:" << header.fileVersion;
                qDebug() << "\tflags:" << header.flags;
                qDebug() << "\tsize in bytes:" << header.sizeInBytes;

                // Draw Mode
                qDebug() << "\t\tdraw mode:" << static_cast<int>(mesh.drawMode());

                // Winding
                qDebug() << "\t\twinding:" << QSSGBaseTypeHelpers::toString(QSSGRenderWinding(mesh.winding()));

                // Vertex Buffer
                const Mesh::VertexBuffer vb = mesh.vertexBuffer();
                qDebug() << "\t\t -- Vertex Buffer --";
                qDebug() << "\t\tentry count:" << vb.entries.size();
                for (quint32 idx = 0, end = vb.entries.size(); idx < end; ++idx) {
                    qDebug() << "\t\t\t -- Vertex Buffer Entry" << idx << "--";
                    const Mesh::VertexBufferEntry &entry(vb.entries[idx]);
                    qDebug() << "\t\t\tname:" << entry.name;
                    qDebug() << "\t\t\ttype:" << QSSGBaseTypeHelpers::toString(QSSGRenderComponentType(entry.componentType));
                    qDebug() << "\t\t\tcomponentCount:" << entry.componentCount;
                    qDebug() << "\t\t\tstart offset:" << entry.offset;
                }
                qDebug() << "\t\tstride:" << vb.stride;
                qDebug() << "\t\tdata size in bytes:" << vb.data.size();

                // Target Buffer
                if (header.hasSeparateTargetBuffer()) {
                    const Mesh::TargetBuffer tb = mesh.targetBuffer();
                    qDebug() << "\t\t -- Morph Target Buffers --";
                    qDebug() << "\t\tentry count:" << tb.entries.count();
                    for (quint32 idx = 0, end = tb.entries.count(); idx < end; ++idx) {
                        qDebug() << "\t\t\t -- Target Buffer Entry" << idx << "--";
                        const Mesh::VertexBufferEntry &entry(vb.entries[idx]);
                        qDebug() << "\t\t\tname:" << entry.name;
                        qDebug() << "\t\t\ttype:" << QSSGBaseTypeHelpers::toString(QSSGRenderComponentType(entry.componentType));
                        qDebug() << "\t\t\tcomponentCount:" << entry.componentCount;
                        qDebug() << "\t\t\tstart offset:" << entry.offset;
                    }
                    qDebug() << "\t\tnumber of targets:" << tb.numTargets;
                    qDebug() << "\t\tdata size in bytes:" << tb.data.size();
                }

                // Index Buffer
                const Mesh::IndexBuffer ib = mesh.indexBuffer();
                qDebug() << "\t\t -- Index Buffer --";
                qDebug() << "\t\tcomponentType:" << QSSGBaseTypeHelpers::toString(QSSGRenderComponentType(ib.componentType));
                qDebug() << "\t\tdata size in bytes:" << ib.data.size();

                // Subsets
                const QVector<Mesh::Subset> subsets = mesh.subsets();
                qDebug() << "\t\t -- Subsets --";
                qDebug() << "\t\tsubset count:" << subsets.size();
                for (quint32 idx = 0, end = subsets.size(); idx < end; ++idx) {
                    qDebug() << "\t\t -- Subset" << idx << "--";
                    const Mesh::Subset &subset(subsets[idx]);
                    qDebug() << "\t\tindex count:" << subset.count;
                    qDebug() << "\t\tstart offset in indices:" << subset.offset;
                    qDebug() << "\t\tbounds: (" <<
                                subset.bounds.min.x() << "," <<
                                subset.bounds.min.y() << "," <<
                                subset.bounds.min.z() << ") (" <<
                                subset.bounds.max.x() << "," <<
                                subset.bounds.max.y() << "," <<
                                subset.bounds.max.z() << ")";
                    qDebug() << "\t\tname:" << subset.name;
                    if (header.hasLightmapSizeHint())
                        qDebug() << "\t\tlightmap size hint:" << subset.lightmapSizeHint;
                    if (header.hasLodDataHint()) {
                        qDebug() << "\t\tlods: ";
                        for (auto lod : subset.lods) {
                            qDebug() << "\t\t\tcount: " << lod.count << "offset: " << lod.offset << "distance: " << lod.distance;
                        }
                    }
                }
            }

        }

        meshFile.close();
        qDebug() << "closed meshFile";
    }

    return 0;
}
