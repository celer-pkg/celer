// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>
#include <QtQuick3D/qquick3dobject.h>

#include <QtQuick3D/private/qquick3dcamera_p.h>
#include <QtQuick3D/private/qquick3dperspectivecamera_p.h>
#include <QtQuick3D/private/qquick3dorthographiccamera_p.h>
#include <QtQuick3D/private/qquick3dfrustumcamera_p.h>
#include <QtQuick3D/private/qquick3dcustomcamera_p.h>

#include <QtQuick3D/private/qquick3ddefaultmaterial_p.h>
#include <QtQuick3D/private/qquick3dtexture_p.h>
#include <QtQuick3D/private/qquick3ddirectionallight_p.h>
#include <QtQuick3D/private/qquick3dpointlight_p.h>
#include <QtQuick3D/private/qquick3dspotlight_p.h>
#include <QtQuick3D/private/qquick3dmaterial_p.h>
#include <QtQuick3D/private/qquick3dmodel_p.h>
#include <QtQuick3D/private/qquick3dskeleton_p.h>
#include <QtQuick3D/private/qquick3djoint_p.h>
#include <QtQuick3D/private/qquick3dmorphtarget_p.h>
#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtQuick3D/private/qquick3dviewport_p.h>
#include <QtQuick3D/private/qquick3dsceneenvironment_p.h>
#include <QtQuick3D/private/qquick3dpickresult_p.h>
#include <QtQuick3D/private/qquick3drepeater_p.h>
#include <QtQuick3D/private/qquick3dloader_p.h>
#include <QtQuick3D/private/qquick3dprincipledmaterial_p.h>
#include <QtQuick3D/private/qquick3drenderstats_p.h>
#include <QtQuick3D/private/qquick3dgeometry_p.h>
#include <QtQuick3D/private/qquick3dquaternionutils_p.h>
#include <QtQuick3D/private/qquick3dquaternionanimation_p.h>
#include <QtQuick3D/private/qquick3dtexturedata_p.h>
#include <QtQuick3D/private/qquick3dreflectionprobe_p.h>
#include <QtQuick3D/private/qquick3dbakedlightmap_p.h>
#include <QtQuick3D/private/qquick3dlightmapper_p.h>

#include <private/qqmlglobal_p.h>


static void initResources()
{
#ifdef QT_STATIC
    Q_INIT_RESOURCE(qmake_QtQuick3D);
    Q_INIT_RESOURCE(res);
#endif
}

QT_BEGIN_NAMESPACE

Q_GHS_KEEP_REFERENCE(qml_register_types_QtQuick3D);

static QQmlPrivate::AutoParentResult qquick3dobject_autoParent(QObject *obj, QObject *parent)
{
    // When setting a parent (especially during dynamic object creation) in QML,
    // also try to set up the analogous item/window relationship.
    if (QQuick3DObject *parentItem = qmlobject_cast<QQuick3DObject *>(parent)) {
        QQuick3DObject *item = qmlobject_cast<QQuick3DObject *>(obj);
        if (item) {
            // An Item has another Item
            item->setParentItem(parentItem);
            return QQmlPrivate::Parented;
        }
        return QQmlPrivate::IncompatibleObject;
    } else if (qmlobject_cast<QQuick3DObject *>(obj)) {
        return QQmlPrivate::IncompatibleParent;
    }
    return QQmlPrivate::IncompatibleObject;
}

static void qt_quick3d_defineModule()
{
    QQmlPrivate::RegisterAutoParent autoparent = { 0, &qquick3dobject_autoParent };
    QQmlPrivate::qmlregister(QQmlPrivate::AutoParentRegistration, &autoparent);

    qRegisterMetaType<QQuick3DPickResult>();
    qRegisterMetaType<QQuick3DRenderStats *>();
    qRegisterMetaType<QQuick3DBounds3>();

}

class QQuick3DPlugin : public QQmlEngineExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    QQuick3DPlugin(QObject *parent = nullptr) : QQmlEngineExtensionPlugin(parent)
    {
        volatile auto registration = &qml_register_types_QtQuick3D;
        Q_UNUSED(registration);
        qt_quick3d_defineModule();
        initResources();
    }
};

QT_END_NAMESPACE

#include "plugin.moc"
