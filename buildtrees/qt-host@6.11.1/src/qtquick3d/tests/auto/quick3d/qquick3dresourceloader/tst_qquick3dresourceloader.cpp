// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>

#include <QtQuick3D/private/qquick3dresourceloader_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrenderresourceloader_p.h>
#include <QtQuick3D/private/qquick3dobject_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>

class tst_QQuick3DResourceLoader : public QObject
{
    Q_OBJECT

    // Work-around to get access to updateSpatialNode
    class ResourceLoader : public QQuick3DResourceLoader
    {
    public:
        using QQuick3DResourceLoader::updateSpatialNode;
    };

private Q_SLOTS:
    void testProperties();

};

void tst_QQuick3DResourceLoader::testProperties()
{
    ResourceLoader resourceLoader;
    auto node = static_cast<QSSGRenderResourceLoader *>(resourceLoader.updateSpatialNode(nullptr));
    QVERIFY(node);

    // We can really only test the meshSources property as the other two properties are
    // QML Lists instead of value based lists.

    // Mesh Sources
    QSignalSpy spy(&resourceLoader, SIGNAL(meshSourcesChanged()));
    resourceLoader.setMeshSources( {QUrl("#Cube"), QUrl("#Sphere")} );
    QCOMPARE(spy.size(), 1);
    node = static_cast<QSSGRenderResourceLoader *>(resourceLoader.updateSpatialNode(nullptr));
    QCOMPARE(resourceLoader.meshSources().size(), 2);
    QCOMPARE(node->meshes.size(), 2);

}

QTEST_APPLESS_MAIN(tst_QQuick3DResourceLoader)
#include "tst_qquick3dresourceloader.moc"
